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

#include "ns3/gu-chord-message.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("GUChordMessage");
NS_OBJECT_ENSURE_REGISTERED (GUChordMessage);

GUChordMessage::GUChordMessage ()
{
}

GUChordMessage::~GUChordMessage ()
{
}

GUChordMessage::GUChordMessage (GUChordMessage::MessageType messageType, uint32_t transactionId)
{
  m_messageType = messageType;
  m_transactionId = transactionId;
}

TypeId 
GUChordMessage::GetTypeId (void)
{
  static TypeId tid = TypeId ("GUChordMessage")
    .SetParent<Header> ()
    .AddConstructor<GUChordMessage> ()
  ;
  return tid;
}

TypeId
GUChordMessage::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}


uint32_t
GUChordMessage::GetSerializedSize (void) const
{
  // size of messageType, transaction id
  uint32_t size = sizeof (uint8_t) + sizeof (uint32_t);
  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.GetSerializedSize ();
        break;
      case PING_RSP:
        size += m_message.pingRsp.GetSerializedSize ();
        break;
      case FIND_SUC_REQ:
        size += m_message.findSucReq.GetSerializedSize ();
        break;
      case FIND_SUC_RSP:
      size += m_message.findSucRsp.GetSerializedSize ();
        break;
      case GET_PRED_SUC_REQ:
      size += m_message.getPredSucReq.GetSerializedSize ();
        break;
      case GET_PRED_SUC_RSP:
      size += m_message.getPredSucRsp.GetSerializedSize ();
        break;
      case RINGSTATE:
      size += m_message.ringstate.GetSerializedSize ();
        break;
      case NOTIFY_SUC:
      size += m_message.notifySuc.GetSerializedSize ();
        break;
      case NOTIFY_PRED:
      size += m_message.notifyPred.GetSerializedSize ();
        break;
      // case DHASH_LOOK:
      // size += m_message.dhashLook.GetSerializedSize ();
      case JOIN_REQ:
        size += m_message.joinReq.GetSerializedSize ();
      break;
      case JOIN_RSP:
        size += m_message.joinRsp.GetSerializedSize ();
      break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

void
GUChordMessage::Print (std::ostream &os) const
{
  os << "\n****GUChordMessage Dump****\n" ;
  os << "messageType: " << m_messageType << "\n";
  os << "transactionId: " << m_transactionId << "\n";
  os << "PAYLOAD:: \n";
  
  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Print (os);
        break;
      case PING_RSP:
        m_message.pingRsp.Print (os);
        break;
      default:
        break;  
    }
  os << "\n****END OF MESSAGE****\n";
}

void
GUChordMessage::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteU8 (m_messageType);
  i.WriteHtonU32 (m_transactionId);

  switch (m_messageType)
    {
      case PING_REQ:
        m_message.pingReq.Serialize (i);
        break;
      case PING_RSP:
        m_message.pingRsp.Serialize (i);
        break;
      case FIND_SUC_REQ:
        m_message.findSucReq.Serialize (i);
        break;
      case FIND_SUC_RSP:
        m_message.findSucRsp.Serialize (i);
        break;
      case GET_PRED_SUC_REQ:
        m_message.getPredSucReq.Serialize (i);
        break;
      case GET_PRED_SUC_RSP:
        m_message.getPredSucRsp.Serialize (i);
        break;
      case RINGSTATE:
        m_message.ringstate.Serialize (i);
        break;
      case NOTIFY_SUC:
        m_message.notifySuc.Serialize (i);
        break;
      case NOTIFY_PRED:
        m_message.notifyPred.Serialize (i);
        break;
      // case DHASH_LOOK:
      //   m_message.dhashLook.Serialize (i);
        // break;
      case JOIN_REQ:
        m_message.joinReq.Serialize (i);
        break;
      case JOIN_RSP:
        m_message.joinRsp.Serialize (i);
        break;
      default:
        NS_ASSERT (false);   
    }
}

uint32_t 
GUChordMessage::Deserialize (Buffer::Iterator start)
{
  uint32_t size;
  Buffer::Iterator i = start;
  m_messageType = (MessageType) i.ReadU8 ();
  m_transactionId = i.ReadNtohU32 ();

  size = sizeof (uint8_t) + sizeof (uint32_t);

  switch (m_messageType)
    {
      case PING_REQ:
        size += m_message.pingReq.Deserialize (i);
        break;
      case PING_RSP:
        size += m_message.pingRsp.Deserialize (i);
        break;
      case FIND_SUC_REQ:
        m_message.findSucReq.Deserialize (i);
        break;
      case FIND_SUC_RSP:
        m_message.findSucRsp.Deserialize (i);
        break;
      case GET_PRED_SUC_REQ:
        m_message.getPredSucReq.Deserialize (i);
        break;
      case GET_PRED_SUC_RSP:
        m_message.getPredSucRsp.Deserialize (i);
        break;
      case RINGSTATE:
        m_message.ringstate.Deserialize (i);
        break;
      case NOTIFY_SUC:
        m_message.notifySuc.Deserialize (i);
        break;
      case NOTIFY_PRED:
        m_message.notifyPred.Deserialize (i);
        break;
      // case DHASH_LOOK:
      //   m_message.dhashLook.Deserialize (i);
      // break;
      case JOIN_REQ:
        m_message.joinReq.Deserialize (i);
        break;
      case JOIN_RSP:
        size += m_message.joinRsp.Deserialize (i);
        break;
      default:
        NS_ASSERT (false);
    }
  return size;
}

/* PING_REQ */

uint32_t 
GUChordMessage::PingReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
GUChordMessage::PingReq::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
GUChordMessage::PingReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
GUChordMessage::PingReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingReq::GetSerializedSize ();
}

void
GUChordMessage::SetPingReq (std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_REQ);
    }
  m_message.pingReq.pingMessage = pingMessage;
}

GUChordMessage::PingReq
GUChordMessage::GetPingReq ()
{
  return m_message.pingReq;
}

/* PING_RSP */

uint32_t 
GUChordMessage::PingRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + pingMessage.length();
  return size;
}

void
GUChordMessage::PingRsp::Print (std::ostream &os) const
{
  os << "PingReq:: Message: " << pingMessage << "\n";
}

void
GUChordMessage::PingRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (pingMessage.length ());
  start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
}

uint32_t
GUChordMessage::PingRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  pingMessage = std::string (str, length);
  free (str);
  return PingRsp::GetSerializedSize ();
}

void
GUChordMessage::SetPingRsp (std::string pingMessage)
{
  if (m_messageType == 0)
    {
      m_messageType = PING_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == PING_RSP);
    }
  m_message.pingRsp.pingMessage = pingMessage;
}

GUChordMessage::PingRsp
GUChordMessage::GetPingRsp ()
{
  return m_message.pingRsp;
}

//
// //


uint32_t 
GUChordMessage::FindSucRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  size += sizeof(uint16_t) + SNodeId.length();
  size += sizeof(uint16_t) + PNodeId.length();
  return size;
}

void
GUChordMessage::FindSucRsp::Print (std::ostream &os) const
{
  os << "FindSucRsp:: Message: " << NodeId << "\n";
}

void
GUChordMessage::FindSucRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
  start.WriteU16 (SNodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (SNodeId.c_str())), SNodeId.length());
  start.WriteU16 (PNodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (PNodeId.c_str())), PNodeId.length());
}

uint32_t
GUChordMessage::FindSucRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  uint16_t Slength = start.ReadU16 ();
  char* Sstr = (char*) malloc (Slength);
  start.Read ((uint8_t*)Sstr, Slength);
  SNodeId = std::string (Sstr, Slength);
  free (Sstr);
  uint16_t Plength = start.ReadU16 ();
  char* Pstr = (char*) malloc (Plength);
  start.Read ((uint8_t*)Pstr, Plength);
  PNodeId = std::string (Pstr, Plength);
  free (Pstr);
  return FindSucRsp::GetSerializedSize ();
}

void
GUChordMessage::SetFindSucRsp (std::string NodeId, std::string SNodeId, std::string PNodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = FIND_SUC_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_SUC_RSP);
    }
  m_message.findSucRsp.NodeId = NodeId;
  m_message.findSucRsp.SNodeId = SNodeId;
  m_message.findSucRsp.PNodeId = PNodeId;
}

GUChordMessage::FindSucRsp
GUChordMessage::GetFindSucRsp ()
{
  return m_message.findSucRsp;
}



uint32_t 
GUChordMessage::FindSucReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  size += sizeof(uint16_t) + HNodeId.length();
  return size;
}

void
GUChordMessage::FindSucReq::Print (std::ostream &os) const
{
  os << "FindSucReq::Message: " << NodeId << "\n";
}

void
GUChordMessage::FindSucReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
  start.WriteU16 (HNodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (HNodeId.c_str())), HNodeId.length());
}

uint32_t
GUChordMessage::FindSucReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  uint16_t Hlength = start.ReadU16 ();
  char* Hstr = (char*) malloc (Hlength);
  start.Read ((uint8_t*)Hstr, Hlength);
  HNodeId = std::string (Hstr, Hlength);
  free (Hstr);
  return FindSucReq::GetSerializedSize ();
}

void
GUChordMessage::SetFindSucReq (std::string NodeId, std::string HNodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = FIND_SUC_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == FIND_SUC_REQ);
    }
  m_message.findSucReq.NodeId = NodeId;
  m_message.findSucReq.HNodeId = HNodeId;
}

GUChordMessage::FindSucReq
GUChordMessage::GetFindSucReq ()
{
  return m_message.findSucReq;
}



uint32_t 
GUChordMessage::GetPredSucReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  return size;
}

void
GUChordMessage::GetPredSucReq::Print (std::ostream &os) const
{
  os << "getPredSucReq:: Message: " << NodeId << "\n";
}

void
GUChordMessage::GetPredSucReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
}

uint32_t
GUChordMessage::GetPredSucReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  return GetPredSucReq::GetSerializedSize ();
}

void
GUChordMessage::SetGetPredSucReq (std::string NodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = GET_PRED_SUC_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == GET_PRED_SUC_REQ);
    }
  m_message.getPredSucReq.NodeId = NodeId;
}

GUChordMessage::GetPredSucReq
GUChordMessage::GetGetPredSucReq()
{
  return m_message.getPredSucReq;
}



uint32_t 
GUChordMessage::GetPredSucRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  return size;
}

void
GUChordMessage::GetPredSucRsp::Print (std::ostream &os) const
{
  os << "getPredSucRsp:: Message: " << NodeId << "\n";
}

void
GUChordMessage::GetPredSucRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
}

uint32_t
GUChordMessage::GetPredSucRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  return GetPredSucRsp::GetSerializedSize ();
}

void
GUChordMessage::SetGetPredSucRsp (std::string NodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = GET_PRED_SUC_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == GET_PRED_SUC_RSP);
    }
  m_message.getPredSucRsp.NodeId = NodeId;
}

GUChordMessage::GetPredSucRsp
GUChordMessage::GetGetPredSucRsp()
{
  return m_message.getPredSucRsp;
}


uint32_t 
GUChordMessage::Ringstate::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  return size;
}

void
GUChordMessage::Ringstate::Print (std::ostream &os) const
{
  os << "Ringstate:: Message: " << NodeId << "\n";
}

void
GUChordMessage::Ringstate::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
}

uint32_t
GUChordMessage::Ringstate::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  return Ringstate::GetSerializedSize ();
}

void
GUChordMessage::SetRingstate (std::string NodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = RINGSTATE;
    }
  else
    {
      NS_ASSERT (m_messageType == RINGSTATE);
    }
  m_message.ringstate.NodeId = NodeId;
}

GUChordMessage::Ringstate
GUChordMessage::GetRingstate()
{
  return m_message.ringstate;
}


uint32_t 
GUChordMessage::NotifySuc::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  return size;
}

void
GUChordMessage::NotifySuc::Print (std::ostream &os) const
{
  os << "NotifySuc:: Message: " << NodeId<< "\n";
}

void
GUChordMessage::NotifySuc::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
}

uint32_t
GUChordMessage::NotifySuc::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  return NotifySuc::GetSerializedSize ();
}

void
GUChordMessage::SetNotifySuc (std::string NodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = NOTIFY_SUC;
    }
  else
    {
      NS_ASSERT (m_messageType == NOTIFY_SUC);
    }
  m_message.notifySuc.NodeId = NodeId;
}

GUChordMessage::NotifySuc
GUChordMessage::GetNotifySuc()
{
  return m_message.notifySuc;
}

uint32_t 
GUChordMessage::NotifyPred::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  return size;
}

void
GUChordMessage::NotifyPred::Print (std::ostream &os) const
{
  os << "NotifyPred:: Message: " << NodeId << "\n";
}

void
GUChordMessage::NotifyPred::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
}

uint32_t
GUChordMessage::NotifyPred::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  free (str);
  return NotifyPred::GetSerializedSize ();
}

void
GUChordMessage::SetNotifyPred (std::string NodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = NOTIFY_PRED;
    }
  else
    {
      NS_ASSERT (m_messageType == NOTIFY_PRED);
    }
  m_message.notifyPred.NodeId = NodeId;
}

GUChordMessage::NotifyPred
GUChordMessage::GetNotifyPred()
{
  return m_message.notifyPred;
}

uint32_t 
GUChordMessage::JoinRsp::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + SNodeId.length();
  size += sizeof(uint16_t) + PNodeId.length();
  return size;
}

void
GUChordMessage::JoinRsp::Print (std::ostream &os) const
{
  os << "JoinRsp::Message: " << SNodeId << "\n";
}

void
GUChordMessage::JoinRsp::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (SNodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (SNodeId.c_str())), SNodeId.length());
  start.WriteU16 (PNodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (PNodeId.c_str())), PNodeId.length());
}

uint32_t
GUChordMessage::JoinRsp::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  SNodeId = std::string (str, length);
  free (str);
  uint16_t Hlength = start.ReadU16 ();
  char* Hstr = (char*) malloc (Hlength);
  start.Read ((uint8_t*)Hstr, Hlength);
  PNodeId = std::string (Hstr, Hlength);
  free (Hstr);
  return JoinRsp::GetSerializedSize ();
}

void
GUChordMessage::SetJoinRsp (std::string NodeId,std::string HNodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = JOIN_RSP;
    }
  else
    {
      NS_ASSERT (m_messageType == JOIN_RSP);
    }
  m_message.joinRsp.SNodeId = NodeId;
  m_message.joinRsp.PNodeId = HNodeId;
}

GUChordMessage::JoinRsp
GUChordMessage::GetJoinRsp()
{
  return m_message.joinRsp;
}

uint32_t 
GUChordMessage::JoinReq::GetSerializedSize (void) const
{
  uint32_t size;
  size = sizeof(uint16_t) + NodeId.length();
  return size;
}

void
GUChordMessage::JoinReq::Print (std::ostream &os) const
{
  os << "JoinReq::Message: " << NodeId << "\n";
}

void
GUChordMessage::JoinReq::Serialize (Buffer::Iterator &start) const
{
  start.WriteU16 (NodeId.length ());
  start.Write ((uint8_t *) (const_cast<char*> (NodeId.c_str())), NodeId.length());
}

uint32_t
GUChordMessage::JoinReq::Deserialize (Buffer::Iterator &start)
{  
  uint16_t length = start.ReadU16 ();
  char* str = (char*) malloc (length);
  start.Read ((uint8_t*)str, length);
  NodeId = std::string (str, length);
  return JoinReq::GetSerializedSize ();
}

void
GUChordMessage::SetJoinReq (std::string NodeId)
{
  if (m_messageType == 0)
    {
      m_messageType = JOIN_REQ;
    }
  else
    {
      NS_ASSERT (m_messageType == JOIN_REQ);
    }
  m_message.joinReq.NodeId = NodeId;
}

GUChordMessage::JoinReq
GUChordMessage::GetJoinReq()
{
  return m_message.joinReq;
}




// uint32_t 
// GUChordMessage::DhashLook::GetSerializedSize (void) const
// {
//   uint32_t size;
//   size = sizeof(uint16_t) + pingMessage.length();
//   return size;
// }

// void
// GUChordMessage::DhashLook::Print (std::ostream &os) const
// {
//   os << "FindSucReq:: Message: " << pingMessage << "\n";
// }

// void
// GUChordMessage::DhashLook::Serialize (Buffer::Iterator &start) const
// {
//   start.WriteU16 (pingMessage.length ());
//   start.Write ((uint8_t *) (const_cast<char*> (pingMessage.c_str())), pingMessage.length());
// }

// uint32_t
// GUChordMessage::DhashLook::Deserialize (Buffer::Iterator &start)
// {  
//   uint16_t length = start.ReadU16 ();
//   char* str = (char*) malloc (length);
//   start.Read ((uint8_t*)str, length);
//   pingMessage = std::string (str, length);
//   free (str);
//   return FindSucReq::GetSerializedSize ();
// }

// void
// GUChordMessage::DhashLook (std::string pingMessage)
// {
//   if (m_messageType == 0)
//     {
//       m_messageType = FIND_SUC_REQ;
//     }
//   else
//     {
//       NS_ASSERT (m_messageType == FIND_SUC_REQ);
//     }
//   m_message.FindSucReq.pingMessage = pingMessage;
// }

// GUChordMessage::DhashLook
// GUChordMessage::GetDHashLook()
// {
//   return m_message.findSucReq;
// }







void
GUChordMessage::SetMessageType (MessageType messageType)
{
  m_messageType = messageType;
}

GUChordMessage::MessageType
GUChordMessage::GetMessageType () const
{
  return m_messageType;
}

void
GUChordMessage::SetTransactionId (uint32_t transactionId)
{
  m_transactionId = transactionId;
}

uint32_t 
GUChordMessage::GetTransactionId (void) const
{
  return m_transactionId;
}

