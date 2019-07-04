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
  return m_acceptor->m_writer->send(datagram, m_connection);
}

void NetworkHandler::received_datagram(DatagramIterator &iterator)
{

}

void NetworkHandler::disconnected()
{

}

NetworkAcceptor::NetworkAcceptor(const char *address, uint16_t port, uint32_t backlog, size_t num_threads)
  : m_address(address), m_port(port), m_backlog(backlog)
{
  m_manager = new QueuedConnectionManager();
  m_listener = new QueuedConnectionListener(m_manager, num_threads);
  m_reader = new QueuedConnectionReader(m_manager, num_threads);
  m_writer = new ConnectionWriter(m_manager, num_threads);

  // setup the connection
  m_socket = m_manager->open_TCP_server_rendezvous(m_address, m_port, m_backlog);
  if (!m_socket)
  {
    throw runtime_error("Failed to open TCP socket!");
  }

  m_listener->add_connection(m_socket);

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

  delete m_manager;
  delete m_listener;
  delete m_reader;
  delete m_writer;
}

bool NetworkAcceptor::has_handler(NetworkHandler *handler)
{
  assert(handler != nullptr);
  for (NetworkHandler *x : m_handlers)
  {
    if (x == handler)
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
    throw runtime_error("Cannot add an already added handler object!");
  }

  m_handlers.push_back(handler);
  m_reader->add_connection(handler->m_connection);
}

void NetworkAcceptor::remove_handler(NetworkHandler *handler)
{
  assert(handler != nullptr);
  if (!has_handler(handler))
  {
    throw runtime_error("Cannot remove an unknown handler object!");
  }

  m_reader->remove_connection(handler->m_connection);
  m_handlers.erase(remove(m_handlers.begin(), m_handlers.end(), handler), m_handlers.end());
  delete handler;
}

NetworkHandler* NetworkAcceptor::get_handler_from_connection(Connection *connection)
{
  assert(connection != nullptr);
  for (NetworkHandler *handler : m_handlers)
  {
    if (handler->m_connection == connection)
    {
      return handler;
    }
  }

  return nullptr;
}

NetworkHandler* NetworkAcceptor::init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection)
{
  return new NetworkHandler(this, rendezvous, address, connection);
}

AsyncTask::DoneStatus NetworkAcceptor::listener_poll(GenericAsyncTask *task, void *data)
{
  NetworkAcceptor *self = (NetworkAcceptor*)data;
  if (self->m_listener->new_connection_available())
  {
    PT(Connection) rendezvous;
    NetAddress address;
    PT(Connection) connection;

    if (self->m_listener->get_new_connection(rendezvous, address, connection))
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
  if (self->m_reader->data_available())
  {
    NetDatagram datagram;
    if (self->m_reader->get_data(datagram))
    {
      if (datagram.get_length() > 0)
      {
        NetworkHandler *handler = self->get_handler_from_connection(datagram.get_connection());
        assert(handler != nullptr);

        DatagramIterator iterator(datagram);
        handler->received_datagram(iterator);
      }
    }
  }

  return AsyncTask::DS_cont;
}

AsyncTask::DoneStatus NetworkAcceptor::disconnect_poll(GenericAsyncTask *task, void *data)
{
  NetworkAcceptor *self = (NetworkAcceptor*)data;
  for (NetworkHandler *handler : self->m_handlers)
  {
    if (!self->m_reader->is_connection_ok(handler->m_connection))
    {
      handler->disconnected();
      self->remove_handler(handler);
    }
  }

  return AsyncTask::DS_cont;
}
