/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/ls-message.h"
#include "ns3/log.h"
#include "ns3/simulator.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LSMessage");
NS_OBJECT_ENSURE_REGISTERED (LSMessage);

LSMessage::LSMessage ()
{
}

LSMessage::~LSMessage ()
{
}

LSMessage::LSMessage (LSMessage::MessageType messageType, uint32_t sequenceNumber, uint8_t ttl, Ipv4Address originatorAddress)
{
  m_messageType = messageType;
  m_sequenceNumber = sequenceNumber;
  m_ttl = ttl;
  m_originatorAddress = originatorAddress;
}

TypeId 
LSMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("LSMessage")
    .SetParent<Header> ()
    .AddConstructor<LSMessage> ()
  ;
  return tid;
}

TypeId
LSMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


uint32_t
LSMessage::GetSerializedSize (void) const
{
  // size of messageType, sequence number, originator address, ttl
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t) + IPV4_ADDRESS_SIZE + sizeof (uint8_t);
  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.GetSerializedSize ();
        break;
      case PING_RSP:
        size += m_message.pingRsp.GetSerializedSize ();
        break;
      case ND_REQ:
	size += m_message.ndReq.GetSerializedSize ();
	break;
      case ND_RSP:
	size += m_message.ndRsp.GetSerializedSize ();
	break;
      case LSP:
	size += m_message.lsp.GetSerializedSize ();
	break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
LSMessage::Print (std::ostream &os) const
{
  os << "\n****LSMessage Dump****\n" ;
  os << "messageType: " << m_messageType << "\n";
  os << "sequenceNumber: " << m_sequenceNumber << "\n";
  os << "ttl: " << m_ttl << "\n";
  os << "originatorAddress: " << m_originatorAddress << "\n";
  os << "PAYLOAD:: \n";
  
  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Print (os);
        break;
      case PING_RSP:
        m_message.pingRsp.Print (os);
        break;
      case ND_REQ:
        m_message.pingRsp.Print (os);
        break;
      case ND_RSP:
        m_message.pingRsp.Print (os);
        break;
//      case LSP:
//	m_message.lsp.Print (os);
//	break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
LSMessage::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (m_sequenceNumber);
  i.WriteU8 (m_ttl);
  i.WriteHtonU32 (m_originatorAddress.Get ());

  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Serialize (i);
        break;
      case PING_RSP:
        m_message.pingRsp.Serialize (i);
        break;
      case ND_REQ:
        m_message.ndReq.Serialize (i);
        break;
      case ND_RSP:
        m_message.ndRsp.Serialize (i);
        break;
      case LSP:
	m_message.lsp.Serialize (i);
 	break;
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
LSMessage::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType) i.ReadU8 ();
  m_sequenceNumber = i.ReadNtohU32 ();
  m_ttl = i.ReadU8 ();
  m_originatorAddress = Ipv4Address (i.ReadNtohU32 ());

  size = sizeof (uint8_t) + sizeof (uint32_t) + sizeof (uint8_t) + IPV4_ADDRESS_SIZE;

  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.Deserialize (i);
        break;
      case PING_RSP:
        size += m_message.pingRsp.Deserialize (i);
        break;
      case ND_REQ:
        size += m_message.ndReq.Deserialize (i);
        break;
      case ND_RSP:
        size += m_message.ndRsp.Deserialize (i);
        break;
      case LSP:
	size += m_message.lsp.Deserialize (i);
	break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PING_REQ */

uint32_t 
LSMessage::PingReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = IPV4_ADDRESS_SIZE + sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
LSMessage::PingReq::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
LSMessage::PingReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (destinationAddress.Get ());
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
LSMessage::PingReq::Deserialize (Buffer::Iterator &start)
{  
  destinationAddress = Ipv4Address (start.ReadNtohU32 ());
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingReq::GetSerializedSize ();
}

void
LSMessage::SetPingReq (Ipv4Address destinationAddress, std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_REQ);
    }
  m_message.pingReq.destinationAddress = destinationAddress;
  m_message.pingReq.pingMessage = pingMessage;
}

LSMessage::PingReq
LSMessage::GetPingReq ()
{
  return m_message.pingReq;
}

/* PING_RSP */

uint32_t 
LSMessage::PingRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = IPV4_ADDRESS_SIZE + sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
LSMessage::PingRsp::Print (std::ostream &os) const
{
  os << "PingRsp:: Message: " << pingMessage << "\n";
}

void
LSMessage::PingRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (destinationAddress.Get ());
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
LSMessage::PingRsp::Deserialize (Buffer::Iterator &start)
{  
  destinationAddress = Ipv4Address (start.ReadNtohU32 ());
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingRsp::GetSerializedSize ();
}

void
LSMessage::SetPingRsp (Ipv4Address destinationAddress, std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_RSP);
    }
  m_message.pingRsp.destinationAddress = destinationAddress;
  m_message.pingRsp.pingMessage = pingMessage;
}

LSMessage::PingRsp
LSMessage::GetPingRsp ()
{
  return m_message.pingRsp;
}

//
//

uint32_t 
LSMessage::NdReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + ndMessage.length();
  return size;
}

void
LSMessage::NdReq::Print (std::ostream &os) const
{
  os << "NdReq:: Message: " << ndMessage << "\n";
}

void
LSMessage::NdReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (ndMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (ndMessage.c_str())), ndMessage.length());
}

uint32_t
LSMessage::NdReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  ndMessage = std::string (str, length);
  free (str);
  return NdReq::GetSerializedSize ();
}

void
LSMessage::SetNdReq ()
{
  if (m_messageType == 3)
    {
      m_messageType = ND_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == ND_REQ);
    }
  m_message.ndReq.ndMessage = "Hello";
}

LSMessage::NdReq
LSMessage::GetNdReq ()
{
  return m_message.ndReq;
}


//
//
uint32_t 
LSMessage::NdRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = IPV4_ADDRESS_SIZE * 2  + sizeof(uint32_t) + sizeof(uint16_t) + ndMessage.length();
  return size;
}

void
LSMessage::NdRsp::Print (std::ostream &os) const
{
  os << "NdReq:: Message: " << ndMessage << "\n";
}

void
LSMessage::NdRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (sourceAddress.Get ());
  start.WriteU32 (destinationAddress.Get ());
  start.WriteU32 (nodeId);
  start.WriteU16 (ndMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (ndMessage.c_str())), ndMessage.length());
}

uint32_t
LSMessage::NdRsp::Deserialize (Buffer::Iterator &start)
{  
  sourceAddress = Ipv4Address (start.ReadNtohU32 ());
  destinationAddress = Ipv4Address (start.ReadU32 ());
  nodeId = uint32_t (start.ReadU32 ());
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  ndMessage = std::string (str, length);
  free (str);
  return NdRsp::GetSerializedSize ();
}

void
LSMessage::SetNdRsp (Ipv4Address destinationAddress, Ipv4Address sourceAddress)
{
  if (m_messageType == 4)
    {
      m_messageType = ND_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == ND_RSP);
    }
  m_message.ndRsp.sourceAddress = sourceAddress;
  m_message.ndRsp.destinationAddress = destinationAddress;
  m_message.ndRsp.ndMessage = "Hello Reply";
}

LSMessage::NdRsp
LSMessage::GetNdRsp ()
{
  return m_message.ndRsp;
}

uint32_t 
LSMessage::Lsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = 3*sizeof(uint32_t)*ntable.size + sizeof(uint32_t) + sizeof(uint32_t);
  return size;
}
/*
void
LSMessage::Lsp::Print (std::ostream &os) const
{
  os << "NdReq:: Message: " << ndMessage << "\n";
}
*/
void
LSMessage::Lsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteHtonU32 (ntable.size);
  for(int i = 0; i < ntable.size; i++){
  start.WriteU32 (ntable.at(i).nodeNumber);
  start.WriteU32 (ntable.at(i).NeighborAddress.Get());
  start.WriteU32 (ntable.at(i).InterfaceAddress.Get());
  }
  start.WriteU32 (sourceAddress.Get());
}

uint32_t
LSMessage::Lsp::Deserialize (Buffer::Iterator &start)
{
  int size = (int)start.ReadNtohU32();
  for(int i = 0; i < size; i++)
{
  uint32_t nodeId = uint32_t(start.ReadU32());
  Ipv4Address neighborAddress = Ipv4Address (start.ReadU32 ());
  Ipv4Address interfaceAddress = Ipv4Address (start.ReadU32 ());
  nTableEntry nEntry (neighborAddress, interfaceAddress, nodeId, Simulator::Now());
  ntable.nTableInsert(nEntry);
}
  sourceAddress = Ipv4Address (start.ReadU32());
  return Lsp::GetSerializedSize ();
}

void
LSMessage::SetLsp (neighborTable nTable, Ipv4Address sourceAddress)
{
  if (m_messageType == 0)
    {
      m_messageType = LSP;
    }
  else
    {
      NS_ASSERT (m_messageType == LSP);
    }
  m_message.lsp.sourceAddress = sourceAddress;
  m_message.lsp.ntable = nTable;
}

LSMessage::Lsp
LSMessage::GetLsp ()
{
  return m_message.lsp;
}


//
//

//
//

void
LSMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

LSMessage::MessageType
LSMessage::GetMessageType () const
{
  return m_messageType;
}

void
LSMessage::SetSequenceNumber (uint32_t sequenceNumber)
{
  m_sequenceNumber = sequenceNumber;
}

uint32_t 
LSMessage::GetSequenceNumber (void) const
{
  return m_sequenceNumber;
}

void
LSMessage::SetTTL (uint8_t ttl)
{
  m_ttl = ttl;
}

uint8_t 
LSMessage::GetTTL (void) const
{
  return m_ttl;
}

void
LSMessage::SetOriginatorAddress (Ipv4Address originatorAddress)
{
  m_originatorAddress = originatorAddress;
}

Ipv4Address
LSMessage::GetOriginatorAddress (void) const
{
  return m_originatorAddress;
}

