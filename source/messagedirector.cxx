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

#include "messagedirector.h"

Participant::Participant(MessageDirector *acceptor, ParticipantInterface *interface, PT(Connection) rendezvous, NetAddress address, PT(Connection) connection)
  : m_interface(interface), NetworkHandler(acceptor, rendezvous, address, connection)
{

}

Participant::~Participant()
{

}

void Participant::receive_datagram(DatagramIterator &iterator)
{
  uint64_t channels = iterator.get_uint8();
  uint64_t channel = iterator.get_uint64();
  if (channels == 1 && channel == CONTROL_MESSAGE)
  {
    uint16_t message_type = iterator.get_uint16();
    uint64_t sender = iterator.get_uint64();
    switch (message_type)
    {
      case CONTROL_SET_CHANNEL:
        {
          if (!m_channel)
          {
            m_channel = sender;
          }

          m_interface->add_participant(sender, this);
        }
        break;
      case CONTROL_REMOVE_CHANNEL:
        {
          m_interface->clear_post_removes(this, sender);
          m_interface->remove_participant(sender);
        }
        break;
      case CONTROL_SET_CON_NAME:
        break;
      case CONTROL_SET_CON_URL:
        break;
      case CONTROL_ADD_RANGE:
        break;
      case CONTROL_REMOVE_RANGE:
        break;
      case CONTROL_ADD_POST_REMOVE:
        {
          if (iterator.get_remaining_size() > 0)
          {
            Datagram *datagram = new Datagram();
            datagram->append_data(iterator.get_remaining_bytes());

            PostRemoveHandle *post_remove = new PostRemoveHandle(sender, datagram);
            m_interface->add_post_remove(sender, post_remove);
          }
        }
        break;
      case CONTROL_CLEAR_POST_REMOVE:
        {
          m_interface->clear_post_removes(this, sender);
        }
        break;
      default:
        return;
    }
  }
  else
  {
    uint64_t sender = iterator.get_uint64();
    uint16_t message_type = iterator.get_uint16();
    Datagram datagram(iterator.get_remaining_bytes());
    m_interface->route_datagram(channel, sender, message_type, datagram);
  }
}

void Participant::disconnected()
{
  m_interface->clear_post_removes(this, m_channel);
  m_interface->remove_participant(m_channel);
}

MessageDirector::MessageDirector(const char *address, uint16_t port, uint32_t backlog, size_t num_threads)
  : NetworkAcceptor(address, port, backlog, num_threads)
{
  m_interface = new ParticipantInterface(this);
}

MessageDirector::~MessageDirector()
{
  delete m_interface;
}

Participant* MessageDirector::init_handler(PT(Connection) rendezvous, NetAddress address, PT(Connection) connection)
{
  return new Participant(this, this->m_interface, rendezvous, address, connection);
}

PostRemoveHandle::PostRemoveHandle(uint64_t sender, Datagram *datagram)
  : m_sender(sender), m_datagram(datagram)
{

}

PostRemoveHandle::~PostRemoveHandle()
{
  m_datagram->clear();
  delete m_datagram;
}

TypeHandle ParticipantInterface::_type_handle;

ParticipantInterface::ParticipantInterface(MessageDirector *messagedirector)
  : m_messagedirector(messagedirector)
{

}

ParticipantInterface::~ParticipantInterface()
{

}

bool ParticipantInterface::has_participant(uint64_t channel)
{
  unordered_map<uint64_t, Participant*>::iterator it = m_channels_map.begin();
  it = m_channels_map.find(channel);
  return it != m_channels_map.end();
}

bool ParticipantInterface::has_participant(Participant *participant)
{
  unordered_map<uint64_t, Participant*>::iterator it = m_channels_map.begin();
  for (; it != m_channels_map.end(); ++it)
  {
    if (it->second == participant)
    {
      return true;
    }
  }

  return false;
}

void ParticipantInterface::add_participant(uint64_t channel, Participant *participant)
{
  assert(participant != nullptr);
  if (!channel)
  {
    return;
  }

  if (has_participant(channel))
  {
    return;
  }

  m_channels_map.insert(pair<uint64_t, Participant*>(channel, participant));
}

void ParticipantInterface::remove_participant(uint64_t channel)
{
  if (!channel)
  {
    return;
  }

  if (!has_participant(channel))
  {
    return;
  }

  unordered_map<uint64_t, Participant*>::iterator it;
  it = m_channels_map.find(channel);
  assert(it != m_channels_map.end());
  m_channels_map.erase(channel);
}

void ParticipantInterface::remove_participant(Participant *participant)
{
  assert(participant != nullptr);
  if (!has_participant(participant))
  {
    return;
  }

  remove_participant(participant->m_channel);
}

Participant* ParticipantInterface::get_participant(uint64_t channel)
{
  unordered_map<uint64_t, Participant*>::iterator it = m_channels_map.begin();
  it = m_channels_map.find(channel);
  if (it != m_channels_map.end())
  {
    return it->second;
  }

  return nullptr;
}

bool ParticipantInterface::has_post_remove(uint64_t channel, PostRemoveHandle *post_remove)
{
  unordered_map<uint64_t, vector<PostRemoveHandle*>>::iterator it;
  it = m_post_removes_map.find(channel);
  if (it != m_post_removes_map.end())
  {
    vector<PostRemoveHandle*> post_removes = it->second;
    if (find(post_removes.begin(), post_removes.end(), post_remove) != post_removes.end())
    {
      return true;
    }
  }

  return false;
}

void ParticipantInterface::add_post_remove(uint64_t channel, PostRemoveHandle *post_remove)
{
  if (has_post_remove(channel, post_remove))
  {
    return;
  }

  unordered_map<uint64_t, vector<PostRemoveHandle*>>::iterator it;
  it = m_post_removes_map.find(channel);
  if (it != m_post_removes_map.end())
  {
    it->second.push_back(post_remove);
  }
  else
  {
    vector<PostRemoveHandle*> post_removes;
    post_removes.push_back(post_remove);
    m_post_removes_map.insert(pair<uint64_t, vector<PostRemoveHandle*>>(channel, post_removes));
  }
}

void ParticipantInterface::remove_post_remove(uint64_t channel, PostRemoveHandle *post_remove)
{
  if (!has_post_remove(channel, post_remove))
  {
    return;
  }

  unordered_map<uint64_t, vector<PostRemoveHandle*>>::iterator it;
  it = m_post_removes_map.find(channel);
  assert(it != m_post_removes_map.end());

  // remove the post remove handle
  vector<PostRemoveHandle*> post_removes = it->second;
  post_removes.erase(remove(post_removes.begin(), post_removes.end(), post_remove), post_removes.end());
  delete post_remove;

  // remove the post removes entry if we have no more handles
  if (!post_removes.size())
  {
    m_post_removes_map.erase(channel);
  }
}

void ParticipantInterface::clear_post_removes(Participant *participant, uint64_t channel)
{
  unordered_map<uint64_t, vector<PostRemoveHandle*>>::iterator it;
  it = m_post_removes_map.find(channel);
  if (it != m_post_removes_map.end())
  {
    for (PostRemoveHandle *post_remove : it->second)
    {
      assert(post_remove != nullptr);
      Datagram dg;
      dg.append_data(post_remove->m_datagram->get_data(), post_remove->m_datagram->get_length());
      remove_post_remove(channel, post_remove);

      DatagramIterator iterator(dg);
      participant->receive_datagram(iterator);
    }
  }
}

void ParticipantInterface::route_datagram(uint64_t channel, uint64_t sender, uint16_t message_type, Datagram &raw_datagram)
{
  if (!channel)
  {
    return;
  }

  Participant *participant = get_participant(channel);
  if (!participant)
  {
    return;
  }

  Datagram route_dg;
  route_dg.add_uint8(1);
  route_dg.add_uint64(channel);
  route_dg.add_uint64(sender);
  route_dg.add_uint16(message_type);
  route_dg.append_data(raw_datagram.get_data(), raw_datagram.get_length());
  participant->send_datagram(route_dg);
}
