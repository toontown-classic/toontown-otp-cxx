#pragma once

#include <stdint.h>
#include <string.h>

#include "pandabase.h"
#include "asyncTaskManager.h"
#include "genericAsyncTask.h"
#include "netAddress.h"
#include "connection.h"

#include "datagram.h"
#include "datagramIterator.h"

#include "queuedConnectionManager.h"
#include "queuedConnectionListener.h"
#include "queuedConnectionReader.h"
#include "connectionWriter.h"

using namespace std;

class NetworkAcceptor;

class NetworkHandler : public TypedObject
{
PUBLISHED:
  NetworkHandler(NetworkAcceptor *acceptor, PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);
  virtual ~NetworkHandler();

  virtual bool send_datagram(Datagram &datagram);
  virtual void received_datagram(DatagramIterator &iterator);
  virtual void disconnected();

public:
  NetworkAcceptor *m_acceptor = nullptr;
  PT(Connection) m_rendezvous;
  NetAddress m_address;
  PT(Connection) m_connection;

public:
  static TypeHandle get_class_type()
  {
    return _type_handle;
  }

  static void init_type()
  {
    TypedObject::init_type();
    register_type(_type_handle, "NetworkHandler", TypedObject::get_class_type());
  }

  virtual TypeHandle get_type() const
  {
    return get_class_type();
  }

  virtual TypeHandle force_init_type()
  {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

class NetworkAcceptor : public TypedObject
{
PUBLISHED:
  NetworkAcceptor(const char *address, uint16_t port, uint32_t backlog, size_t num_threads = 0);
  virtual ~NetworkAcceptor();

  bool has_handler(NetworkHandler *handler);
  void add_handler(NetworkHandler *handler);
  void remove_handler(NetworkHandler *handler);
  NetworkHandler* get_handler_from_connection(Connection *connection);

  virtual NetworkHandler* init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);

private:
  static AsyncTask::DoneStatus listener_poll(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus reader_poll(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus disconnect_poll(GenericAsyncTask *task, void *data);

public:
  string m_address;
  uint16_t m_port;
  uint32_t m_backlog;
  PT(Connection) m_socket;
  vector<NetworkHandler*> m_handlers;

  QueuedConnectionManager *m_manager = nullptr;
  QueuedConnectionListener *m_listener = nullptr;
  QueuedConnectionReader *m_reader = nullptr;
  ConnectionWriter *m_writer = nullptr;

  PT(AsyncTaskManager) m_task_mgr;
  PT(GenericAsyncTask) m_listen_task;
  PT(GenericAsyncTask) m_reader_task;
  PT(GenericAsyncTask) m_disconnect_task;

public:
  static TypeHandle get_class_type()
  {
    return _type_handle;
  }

  static void init_type()
  {
    TypedObject::init_type();
    register_type(_type_handle, "NetworkAcceptor", TypedObject::get_class_type());
  }

  virtual TypeHandle get_type() const
  {
    return get_class_type();
  }

  virtual TypeHandle force_init_type()
  {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};
