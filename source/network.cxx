#include "network.h"

TypeHandle NetworkHandler::_type_handle;
TypeHandle NetworkAcceptor::_type_handle;

NetworkHandler::NetworkHandler(NetworkAcceptor *acceptor, PT(Connection) rendezvous, NetAddress address, PT(Connection) connection)
  : m_acceptor(acceptor), m_rendezvous(rendezvous), m_address(address), m_connection(connection)
{

}

NetworkHandler::~NetworkHandler()
{

}

bool NetworkHandler::send_datagram(Datagram &datagram)
{
  return m_acceptor->m_writer.send(datagram, m_connection);
}

void NetworkHandler::receive_datagram(DatagramIterator &iterator)
{

}

void NetworkHandler::disconnected()
{

}

NetworkAcceptor::NetworkAcceptor(const char *address, uint16_t port, uint32_t backlog, size_t num_threads)
  : m_address(address), m_port(port), m_backlog(backlog), m_listener(&m_manager, num_threads),
    m_reader(&m_manager, num_threads), m_writer(&m_manager, num_threads)
{
  // setup our connection
  setup_connection();

  // setup up our polling tasks
  PT(AsyncTaskManager) m_task_mgr = AsyncTaskManager::get_global_ptr();
  m_listen_task = new GenericAsyncTask("_listen_task", &NetworkAcceptor::listener_poll, this);
  m_reader_task = new GenericAsyncTask("_reader_task", &NetworkAcceptor::reader_poll, this);
  m_disconnect_task = new GenericAsyncTask("_disconnect_task", &NetworkAcceptor::disconnect_poll, this);

  m_task_mgr->add(m_listen_task);
  m_task_mgr->add(m_reader_task);
  m_task_mgr->add(m_disconnect_task);
}

NetworkAcceptor::~NetworkAcceptor()
{
  m_task_mgr->remove(m_listen_task);
  m_task_mgr->remove(m_reader_task);
  m_task_mgr->remove(m_disconnect_task);
}

void NetworkAcceptor::setup_connection()
{
  m_connection = m_manager.open_TCP_server_rendezvous(m_address, m_port, m_backlog);
  if (!m_connection)
  {
    throw runtime_error("Failed to open TCP server rendezvous!");
  }

  m_listener.add_connection(m_connection);
}

bool NetworkAcceptor::has_handler(NetworkHandler *handler)
{
  unordered_map<Connection*, NetworkHandler*>::iterator it = m_handlers_map.begin();
  for (it; it != m_handlers_map.end(); ++it)
  {
    if (it->first == handler->m_connection && it->second == handler)
    {
      return true;
    }
  }

  return false;
}

void NetworkAcceptor::add_handler(NetworkHandler *handler)
{
  assert(handler != nullptr);
  if (has_handler(handler))
  {
    return;
  }

  m_handlers_map.insert(pair<PT(Connection), NetworkHandler*>(handler->m_connection, handler));
  m_reader.add_connection(handler->m_connection);
}

void NetworkAcceptor::remove_handler(NetworkHandler *handler)
{
  assert(handler != nullptr);
  if (!has_handler(handler))
  {
    return;
  }

  m_reader.remove_connection(handler->m_connection);
  unordered_map<Connection*, NetworkHandler*>::iterator it;
  it = m_handlers_map.find(handler->m_connection);
  m_handlers_map.erase(it, m_handlers_map.end());
  delete handler;
}

NetworkHandler* NetworkAcceptor::get_handler(PT(Connection) connection)
{
  unordered_map<Connection*, NetworkHandler*>::iterator it;
  it = m_handlers_map.find(connection);
  if (it != m_handlers_map.end())
  {
    return it->second;
  }

  return nullptr;
}

void NetworkAcceptor::disconnect_handler(NetworkHandler *handler)
{
  assert(handler != nullptr);
  handler->disconnected();
  remove_handler(handler);
}

NetworkHandler* NetworkAcceptor::init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection)
{
  return new NetworkHandler(this, rendezvous, address, connection);
}

AsyncTask::DoneStatus NetworkAcceptor::listener_poll(GenericAsyncTask *task, void *data)
{
  NetworkAcceptor *self = (NetworkAcceptor*)data;
  if (self->m_listener.new_connection_available())
  {
    PT(Connection) rendezvous;
    NetAddress address;
    PT(Connection) connection;

    if (self->m_listener.get_new_connection(rendezvous, address, connection))
    {
      NetworkHandler *handler = self->init_handler(rendezvous, address, connection);
      assert(handler != nullptr);

      self->add_handler(handler);
    }
  }

  return AsyncTask::DS_cont;
}

AsyncTask::DoneStatus NetworkAcceptor::reader_poll(GenericAsyncTask *task, void *data)
{
  NetworkAcceptor *self = (NetworkAcceptor*)data;
  if (self->m_reader.data_available())
  {
    NetDatagram datagram;
    if (self->m_reader.get_data(datagram))
    {
      PT(Connection) connection = datagram.get_connection();
      NetworkHandler *handler = self->get_handler(connection);
      assert(handler != nullptr);

      if (!datagram.get_length())
      {
        // don't wait for us to check to see if this connection is still alive,
        // go ahead and just disconnect the handler
        self->disconnect_handler(handler);
      }
      else
      {
        DatagramIterator iterator(datagram);
        handler->receive_datagram(iterator);
      }
    }
  }

  return AsyncTask::DS_cont;
}

AsyncTask::DoneStatus NetworkAcceptor::disconnect_poll(GenericAsyncTask *task, void *data)
{
  NetworkAcceptor *self = (NetworkAcceptor*)data;
  unordered_map<Connection*, NetworkHandler*>::iterator it = self->m_handlers_map.begin();
  for (it; it != self->m_handlers_map.end(); ++it)
  {
    NetworkHandler *handler = it->second;
    if (!self->m_reader.is_connection_ok(handler->m_connection))
    {
      self->disconnect_handler(handler);
    }
  }

  return AsyncTask::DS_cont;
}
