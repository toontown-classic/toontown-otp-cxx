// Copyright (c) 2019, Caleb Marshall.
//
// This file is part of Toontown OTP.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// You should have received a copy of the MIT License
// along with Toontown OTP. If not, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <stdint.h>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <unordered_map>

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

class NetworkConnector : public TypedObject
{
PUBLISHED:
  NetworkConnector(const char *address, uint16_t port, int timeout_ms=5000, size_t num_threads=0);
  virtual ~NetworkConnector();

  virtual void setup_connection();

  bool send_datagram(Datagram &datagram);
  virtual void receive_datagram(DatagramIterator &iterator);
  virtual void disconnected();
  void disconnect();

private:
  static AsyncTask::DoneStatus reader_poll(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus disconnect_poll(GenericAsyncTask *task, void *data);

private:
  string m_address;
  uint16_t m_port;
  int m_timeout_ms;

  QueuedConnectionManager m_manager;
  QueuedConnectionReader m_reader;
  ConnectionWriter m_writer;

  PT(Connection) m_connection;

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
    register_type(_type_handle, "NetworkConnector", TypedObject::get_class_type());
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

class NetworkHandler : public TypedObject
{
PUBLISHED:
  NetworkHandler(NetworkAcceptor *acceptor, PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);
  virtual ~NetworkHandler();

  bool send_datagram(Datagram &datagram);
  virtual void receive_datagram(DatagramIterator &iterator);
  virtual void disconnected();

private:
  NetworkAcceptor *m_acceptor = nullptr;

public:
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
  NetworkAcceptor(const char *address, uint16_t port, uint32_t backlog, size_t num_threads=0);
  virtual ~NetworkAcceptor();

  virtual void setup_connection();

  bool has_handler(NetworkHandler *handler);
  void add_handler(NetworkHandler *handler);
  void remove_handler(NetworkHandler *handler);
  NetworkHandler* get_handler(PT(Connection) connection);
  bool send_handler_datagram(NetworkHandler *handler, Datagram &datagram);
  void disconnect_handler(NetworkHandler *handler);

  virtual NetworkHandler* init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);

private:
  static AsyncTask::DoneStatus listener_poll(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus reader_poll(GenericAsyncTask *task, void *data);
  static AsyncTask::DoneStatus disconnect_poll(GenericAsyncTask *task, void *data);

private:
  string m_address;
  uint16_t m_port;
  uint32_t m_backlog;

  QueuedConnectionManager m_manager;
  QueuedConnectionListener m_listener;
  QueuedConnectionReader m_reader;
  ConnectionWriter m_writer;

  PT(Connection) m_connection;
  unordered_map<Connection*, NetworkHandler*> m_handlers_map;

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
