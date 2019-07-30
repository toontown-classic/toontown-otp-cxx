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
#include <vector>
#include <deque>
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

  void receive_datagram(DatagramIterator &iterator);
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
  MessageDirector(const char *address, uint16_t port, uint32_t backlog=100000, size_t num_threads=1);
  ~MessageDirector();

  Participant* init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection);

public:
  ParticipantInterface *m_interface = nullptr;
};

class PostRemoveHandle
{
public:
  PostRemoveHandle(uint64_t m_sender, Datagram *datagram);
  ~PostRemoveHandle();

public:
  uint64_t m_sender = 0;
  Datagram *m_datagram;
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
