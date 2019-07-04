#pragma once

#include <stdint.h>
#include <string.h>
#include <unordered_map>

#include "pandabase.h"
#include "netAddress.h"
#include "connection.h"

#include "datagram.h"
#include "datagramIterator.h"

#include "msgtypes.h"
#include "network.h"

class MessageDirector;
class PostRemoveHandle;
class ParticipantInterface;

class Participant : public NetworkHandler
{
PUBLISHED:
  Participant(MessageDirector *acceptor, ParticipantInterface *interface, PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);
  ~Participant();

  void received_datagram(DatagramIterator &iterator);
  void received_control_message(DatagramIterator &iterator);
  void disconnected();

public:
  ParticipantInterface *m_interface = nullptr;
  uint64_t m_channel = 0;
  uint64_t m_lo_channel = 0;
  uint64_t m_hi_channel = 0;
};

class MessageDirector : public NetworkAcceptor
{
PUBLISHED:
  MessageDirector(const char *address, uint16_t port, uint32_t backlog = 100000, size_t num_threads = 1);
  ~MessageDirector();

  virtual Participant* init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);
  virtual ParticipantInterface* init_interface();

public:
  ParticipantInterface *m_interface = nullptr;
};

class PostRemoveHandle
{
public:
  PostRemoveHandle(uint64_t m_sender, Datagram &datagram);
  ~PostRemoveHandle();

public:
  uint64_t m_sender = 0;
  Datagram &m_datagram;
};

class ParticipantInterface : public TypedObject
{
PUBLISHED:
  ParticipantInterface(MessageDirector *messagedirector);
  ~ParticipantInterface();

  bool has_participant(uint64_t channel);
  bool has_participant(Participant *participant);

  void add_participant(uint64_t channel, Participant *participant);

  void remove_participant(uint64_t channel);
  void remove_participant(Participant *participant);

  Participant* get_participant(uint64_t channel);

  bool has_post_remove(uint64_t channel, PostRemoveHandle *post_remove);
  void add_post_remove(uint64_t channel, PostRemoveHandle *post_remove);
  void remove_post_remove(uint64_t channel, PostRemoveHandle *post_remove);
  void clear_post_removes(Participant *participant, uint64_t channel);

  void route_datagram(uint64_t channel, uint64_t sender, uint16_t message_type, Datagram &raw_datagram);

public:
  MessageDirector *m_messagedirector = nullptr;
  unordered_map<uint64_t, Participant*> m_channels_map;
  unordered_map<uint64_t, vector<PostRemoveHandle*>> m_post_removes_map;

public:
  static TypeHandle get_class_type()
  {
    return _type_handle;
  }

  static void init_type()
  {
    TypedObject::init_type();
    register_type(_type_handle, "ParticipantInterface", TypedObject::get_class_type());
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
