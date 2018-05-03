/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Pennsylvania
 *
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


#include "gu-chord.h"
#include <stdio.h>

#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"

using namespace ns3;

TypeId
GUChord::GetTypeId ()
{
  static TypeId tid = TypeId ("GUChord")
    .SetParent<GUApplication> ()
    .AddConstructor<GUChord> ()
    .AddAttribute ("AppPort",
                   "Listening port for Application",
                   UintegerValue (10001),
                   MakeUintegerAccessor (&GUChord::m_appPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PingTimeout",
                   "Timeout value for PING_REQ in milliseconds",
                   TimeValue (MilliSeconds (2000)),
                   MakeTimeAccessor (&GUChord::m_pingTimeout),
                   MakeTimeChecker ())
    .AddAttribute ("StabilizeTimeout",
                   "Timeout value for Stabilization in milliseconds",
                   TimeValue (MilliSeconds (100000)),
                   MakeTimeAccessor (&GUChord::m_stabilizeTimeout),
                   MakeTimeChecker ())
/*    .AddAttribute ("AuditFindSuccessorTimeout",
                   "Timeout value for FindSuccessor in milliseconds",
                   TimeValue (MilliSeconds (1000)),
                   MakeTimeAccessor (&GUChord::m_pingTimeout),
                   MakeTimeChecker ())
*/
    ;
  return tid;
}

GUChord::GUChord ()
  : m_auditPingsTimer (Timer::CANCEL_ON_DESTROY), m_stabilizeTimer (Timer::CANCEL_ON_DESTROY)
{
  RandomVariable random;
  SeedManager::SetSeed (time (NULL));
  random = UniformVariable (0x00000000, 0xFFFFFFFF);
  m_currentTransactionId = random.GetInteger ();
}

GUChord::~GUChord ()
{

}

void
GUChord::DoDispose ()
{
  StopApplication ();
  GUApplication::DoDispose ();
}

void
GUChord::StartApplication (void)
{
  if (m_socket == 0)
    { 
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny(), m_appPort);
      m_socket->Bind (local);
      m_socket->SetRecvCallback (MakeCallback (&GUChord::RecvMessage, this));
    }  
  
  SetChordVerbose(true);
  // Configure timers
  m_auditPingsTimer.SetFunction (&GUChord::AuditPings, this);
  m_stabilizeTimer.SetFunction(&GUChord::Stabilize, this);
  // Start timers
  m_auditPingsTimer.Schedule (m_pingTimeout);
  // m_stabilizeTimer.Schedule(m_stabilizeTimeout);
}

void
GUChord::StopApplication (void)
{
  // Close socket
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }

  // Cancel timers
  m_auditPingsTimer.Cancel ();

  m_pingTracker.clear ();
}

void
GUChord::hashtostr(unsigned char* hval, std::string &str)
{
  char out[61];
  for (int i = 0; i < 20; i++)
  {
    snprintf(out+i*3, 4, "%02x ", hval[i]);
  }
  str = out;
}

void 
GUChord::createChord()
{
  //m_pred = stoi(g_nodeId);
  //m_suc = stoi(g_nodeId);
  //wwhat else we need?
  m_stabilizeTimer.Schedule(m_stabilizeTimeout);
  // CHORD_LOG("here1" << std::endl);
  m_pred = ReverseLookup(m_local);
  m_suc = ReverseLookup(m_local);
  SHA1((unsigned char*)m_pred.c_str(), m_pred.size(), m_pred_hash);
  SHA1((unsigned char*)m_suc.c_str(), m_suc.size(), m_suc_hash);
  SHA1((unsigned char*)m_suc.c_str(), m_suc.size(), m_my_hash);
  // std::cout << "createChord called " << std::endl;

}

void 
GUChord::joinChord(std::string nodeNumber)
{
  //send a findSucReq
  //actual joining done in finduscrsp?
  SHA1((unsigned char*)ReverseLookup(m_local).c_str(), ReverseLookup(m_local).size(), m_my_hash);

  uint32_t transactionId = GetNextTransactionId ();
  Ptr<Packet> packet = Create<Packet> ();
  GUChordMessage message = GUChordMessage (GUChordMessage::JOIN_REQ, transactionId);
  message.SetJoinReq (ReverseLookup(m_local));
  packet->AddHeader (message);
  m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(nodeNumber), m_appPort));
  // std::cout << "joinChord called" << std::endl;
}

void
GUChord::startRingstate()
{

  std::string my;
  std::string pre;
  std::string suc;
  hashtostr(m_my_hash, my);
  hashtostr(m_pred_hash, pre);
  hashtostr(m_suc_hash, suc);
  CHORD_LOG ("Ringstate<" << my << ">: Pred<" << m_pred << ", " << pre << ">: Succ<" << m_suc << ", " << suc << ">");
  // std::cout <<  "Current NodeId: " << ReverseLookup(m_local) << std::endl;
  // std::cout <<  "Current IpAddress: " << m_local << std::endl;
  // //std::cout <<  "Current Hash of NodeId: " << m_my_hash << std::endl;//Hash(ReverseLookup(m_local)) << std::endl);
  // std::cout <<  "Predecessor NodeId: " << m_pred << std::endl;
  // std::cout <<  "Predecessor IpAddress: " << ResolveNodeIpAddress(m_pred) << std::endl;
  // //std::cout <<  "Predecessor Hash of NodeId: "<< m_pred_hash<< std::endl;//Hash(m_pred) << std::endl);
  // std::cout <<  "Successor NodeId: " << m_suc << std::endl;
  // std::cout <<  "Successor IpAddress: " << ResolveNodeIpAddress(m_suc) << std::endl;
  uint32_t transactionId = GetNextTransactionId ();
  Ptr<Packet> packet = Create<Packet> ();
  GUChordMessage message = GUChordMessage (GUChordMessage::RINGSTATE, transactionId);
  message.SetRingstate (ReverseLookup(m_local));
  packet->AddHeader (message);
  m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(m_suc), m_appPort));
}

void
GUChord::nodeLeave()
{
  // std::cout << "node lave " << std::endl;
  
  m_suc = ReverseLookup(m_local);
  m_pred = ReverseLookup(m_local);
  memcpy(m_my_hash, m_suc_hash, 20);
  memcpy(m_my_hash, m_pred_hash, 20);
  uint32_t transactionId2 = GetNextTransactionId ();
  Ptr<Packet> packet2 = Create<Packet> ();
  GUChordMessage message2 = GUChordMessage (GUChordMessage::NOTIFY_PRED, transactionId2);
  message2.SetNotifyPred (m_suc);
  packet2->AddHeader (message2);
  m_socket->SendTo (packet2, 0 , InetSocketAddress (ResolveNodeIpAddress(m_pred), m_appPort));
  uint32_t transactionId = GetNextTransactionId ();
  Ptr<Packet> packet = Create<Packet> ();
  GUChordMessage message = GUChordMessage (GUChordMessage::NOTIFY_SUC, transactionId);
  message.SetNotifySuc (m_pred);
  packet->AddHeader (message);
  m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(m_suc), m_appPort));
}

void
GUChord::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;
  if(command == "JOIN")
    {
      if(tokens.size() < 2)
        {
          ERROR_LOG ("Insufficient Parameters!");
          return;
        }
        iterator++;
        std::istringstream sin (*iterator);
        std::string nodeNumber;
        sin >> nodeNumber;
        iterator++;
        if(nodeNumber == ReverseLookup(m_local))
        {
          createChord();
        }
        else
        {
          joinChord(nodeNumber);
        }
    }
  else if(command == "LEAVE")
    {
      if(tokens.size() < 1)
      {
        ERROR_LOG ("Insufficient Parameters!");
        return;
      }
      nodeLeave();
      //current node leaves the chord
    }
  else if(command == "RINGSTATE")
    {
      if(tokens.size() < 1)
      {
        ERROR_LOG ("Insufficient Parameters!");
        return;
      }
      //initiate ring output message
      startRingstate();
    }
  else if(command == "INFO")
  {
    std::cout << "ProcessRingstate" << std::endl;
    std::cout <<  "Current NodeId: " << ReverseLookup(m_local) << std::endl;
    std::cout <<  "Current IpAddress: " << m_local << std::endl;
      //std::cout <<  "Current Hash of NodeId: " << m_my_hash << std::endl;//Hash(ReverseLookup(m_local)) << std::endl);
    std::cout <<  "Predecessor NodeId: " << m_pred << std::endl;
    std::cout <<  "Predecessor IpAddress: " << ResolveNodeIpAddress(m_pred) << std::endl;
      //std::cout <<  "Predecessor Hash of NodeId: "<< m_pred_hash<< std::endl;//Hash(m_pred) << std::endl);
    std::cout <<  "Successor NodeId: " << m_suc << std::endl;
    std::cout <<  "Successor IpAddress: " << ResolveNodeIpAddress(m_suc) << std::endl;
  }

}

void
GUChord::SendPing (Ipv4Address destAddress, std::string pingMessage)
{
  if (destAddress != Ipv4Address::GetAny ())
    {
      uint32_t transactionId = GetNextTransactionId ();
      CHORD_LOG ("Sending PING_REQ to Node: " << ReverseLookup(destAddress) << " IP: " << destAddress << " Message: " << pingMessage << " transactionId: " << transactionId);
      Ptr<PingRequest> pingRequest = Create<PingRequest> (transactionId, Simulator::Now(), destAddress, pingMessage);
      // Add to ping-tracker
      m_pingTracker.insert (std::make_pair (transactionId, pingRequest));
      Ptr<Packet> packet = Create<Packet> ();
      GUChordMessage message = GUChordMessage (GUChordMessage::PING_REQ, transactionId);
      message.SetPingReq (pingMessage);
      packet->AddHeader (message);
      m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
    }
  else
    {
      // Report failure   
      m_pingFailureFn (destAddress, pingMessage);
      
    }
}

void
GUChord::RecvMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  //std::cout << "RecvMessage" << std::endl;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  uint16_t sourcePort = inetSocketAddr.GetPort ();
  //std::cout << "RecvMessage" << std::endl;
  GUChordMessage message;
  packet->RemoveHeader (message);
  //std::cout << "RecvMessage" << std::endl;
  // std::cout << message.GetMessageType() << std::endl;
  switch (message.GetMessageType ())
    {
      case GUChordMessage::PING_REQ:
        ProcessPingReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::PING_RSP:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::JOIN_REQ:
        ProcessJoinReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::JOIN_RSP:
        ProcessJoinRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_SUC_REQ:
        ProcessFindSucReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::FIND_SUC_RSP:
        ProcessFindSucRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::GET_PRED_SUC_REQ:
        ProcessGetPredSucReq (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::GET_PRED_SUC_RSP:
        ProcessGetPredSucRsp (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::RINGSTATE:
        ProcessRingstate (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::NOTIFY_SUC:
        ProcessNotifySuc (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::NOTIFY_PRED:
        ProcessNotifyPred (message, sourceAddress, sourcePort);
        break;
      case GUChordMessage::DHASH_LOOK:
        ProcessPingRsp (message, sourceAddress, sourcePort);
        break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

bool
GUChord::CompareHash(unsigned char* hash1, unsigned char* hash2)
{
  // returns true if rhs is bigger and false if lhs is bigger

  for(int i = 0; i < 20; i++)
  {
    if(hash1[i]>hash2[i])
      return true;
  }
  return false;
}

void
GUChord::ProcessJoinReq(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessJoinReq" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  // std::cout << "From node : " << fromNode << std::endl;
  Ipv4Address destAddress;
  GUChordMessage resp;
  CHORD_LOG ("Received JOIN_REQ, From Node: " << fromNode << " to node : " << message.GetJoinReq().NodeId);
  if (ReverseLookup(m_local) == m_suc)
  {
    resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
    resp.SetJoinRsp (ReverseLookup (m_local), ReverseLookup (m_local));
    // std::cout << "ONLY ONE NODE" << std::endl;
    Ptr<Packet> packet = Create<Packet> ();
  //std::cout << "ProcessJoinReq" << std::endl;
  packet->AddHeader (resp);
  //std::cout << "ProcessJoinReq" << std::endl;
  m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
  return;
  }
  else
  {
    resp = GUChordMessage (GUChordMessage::FIND_SUC_REQ, message.GetTransactionId());
    resp.SetFindSucReq (message.GetJoinReq().NodeId, ReverseLookup(m_local));
    destAddress = ResolveNodeIpAddress(m_suc);
  }
  Ptr<Packet> packet = Create<Packet> ();
  //std::cout << "ProcessJoinReq" << std::endl;
  packet->AddHeader (resp);
  //std::cout << "ProcessJoinReq" << std::endl;
  m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
}

void
GUChord::ProcessFindSucReq(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessFindSucReq" << std::endl;
  Ipv4Address destAddress;
  GUChordMessage resp;
  std::string fromNode = ReverseLookup (sourceAddress);
  // std::cout << fromNode << std::endl;
  CHORD_LOG ("Received FindSucReq, From Node: " << fromNode << " for node : " << message.GetFindSucReq().NodeId);
  unsigned char hash[20];
  SHA1((unsigned char*)message.GetFindSucReq().NodeId.c_str(), message.GetFindSucReq().NodeId.size(), hash);
  // std::cout << message.GetFindSucReq().NodeId << "   " << message.GetFindSucReq().HNodeId << "    " << ReverseLookup(m_local) << std::endl;
  if(memcmp(hash,m_my_hash, 20) == 0)
  {
    // std::cout << "I already exist in the chord" << std::endl;
    return;
  }
  else if(memcmp(hash, m_my_hash, 20) > 0)
  {
    if(memcmp(m_my_hash, m_pred_hash, 20) > 0)
    {
      resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
      resp.SetJoinRsp (ReverseLookup(m_local), m_suc);
      destAddress = ResolveNodeIpAddress(message.GetFindSucReq().NodeId);  
      m_pred = message.GetFindSucReq().NodeId;
      // std::cout << "WHERE TO BE" << ReverseLookup(m_local) << "   " << m_pred << "   " << message.GetFindSucReq().NodeId<< std::endl;
    }
    else
    {
      resp = GUChordMessage (GUChordMessage::FIND_SUC_REQ, message.GetTransactionId());
      resp.SetFindSucReq (message.GetFindSucReq().NodeId, message.GetFindSucReq().HNodeId);
      destAddress = ResolveNodeIpAddress(m_suc);
      // std::cout << "GREATER THAN" << std::endl;
    }
  }
  else
  {
    resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
    resp.SetJoinRsp (ReverseLookup(m_local), m_pred);
    destAddress = ResolveNodeIpAddress(message.GetFindSucReq().NodeId);
    m_pred = message.GetFindSucReq().NodeId;
  
    SHA1((unsigned char*)m_pred.c_str(), m_pred.size(), m_pred_hash);
    // std::cout << "LESS THAN" << std::endl;
    
  }
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (resp);
  m_socket->SendTo (packet, 0 , InetSocketAddress (destAddress, m_appPort));
}

void
GUChord::ProcessFindSucRsp(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessFindSucRsp" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  // std::cout << fromNode << std::endl;
  CHORD_LOG ("Received FindSucRsp, From Node: " << fromNode << " to node : " << message.GetFindSucRsp().NodeId);
  GUChordMessage resp = GUChordMessage (GUChordMessage::JOIN_RSP, message.GetTransactionId());
  resp.SetJoinRsp (message.GetFindSucRsp().SNodeId, message.GetFindSucRsp().PNodeId);
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (resp);
  m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(message.GetFindSucRsp().NodeId), m_appPort));
}

void
GUChord::ProcessJoinRsp(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessJoinRsp" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  // std::cout << fromNode << std::endl;
  CHORD_LOG ("Received JOIN_RSP, From Node: " << fromNode << " to node : " << message.GetJoinRsp().SNodeId);
  //Actually set successor to this one.
  m_suc = message.GetJoinRsp().SNodeId;
  m_pred = message.GetJoinRsp().PNodeId;
  SHA1((unsigned char*)m_pred.c_str(), m_pred.size(), m_pred_hash);
  SHA1((unsigned char*)m_suc.c_str(), m_suc.size(), m_suc_hash);
  GUChordMessage resp = GUChordMessage (GUChordMessage::NOTIFY_PRED, message.GetTransactionId());
  resp.SetNotifyPred(ReverseLookup(m_local));
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (resp);
  m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(m_pred), m_appPort));
}

void 
GUChord::ProcessNotifyPred(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessNotifyPred" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  CHORD_LOG ("Received NOTFY_PRED, From Node: " << fromNode << " to node : " << message.GetNotifyPred().NodeId);
  //Actually set successor to this one.
  unsigned char hash[20];
  SHA1((unsigned char*)message.GetNotifyPred().NodeId.c_str(), message.GetNotifyPred().NodeId.size(), hash);
  // if(memcmp(m_suc_hash, hash, 20) > 0 || m_suc.empty())
  // {
    m_suc = message.GetNotifyPred().NodeId;
    SHA1((unsigned char*)m_suc.c_str(), m_suc.size(), m_suc_hash);
  if(m_pred == ReverseLookup(m_local))
  {
    m_pred = message.GetNotifyPred().NodeId;
    SHA1((unsigned char*)m_pred.c_str(), m_pred.size(), m_pred_hash);
  }
  // }
}

void 
GUChord::ProcessNotifySuc(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessNotifySuc" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  CHORD_LOG ("Received NOTFY_SUC, From Node: " << fromNode << " to node : " << message.GetNotifySuc().NodeId);
  //Actually set successor to this one.
  unsigned char hash[20];
  SHA1((unsigned char*)message.GetNotifySuc().NodeId.c_str(), message.GetNotifySuc().NodeId.size(), hash);
  // if(memcmp(m_pred_hash, hash, 20) < 0 || m_pred.empty())
  // {
  // std::cout << "ProcessNotifySuc" << std::endl;
    m_pred = message.GetNotifySuc().NodeId;
    SHA1((unsigned char*)m_pred.c_str(), m_pred.size(), m_pred_hash);
  // }
    // std::cout << "ProcessNotifySuc" << std::endl;
    if(m_suc == ReverseLookup(m_local))
    {
      //std::cout << "ProcessNotifySuc" << std::endl;
      m_suc = message.GetNotifySuc().NodeId;
      SHA1((unsigned char*)m_suc.c_str(), m_suc.size(), m_suc_hash);
    }
    // std::cout << "ProcessNotifySuc" << std::endl;
}

void
GUChord::ProcessRingstate (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
    // std::cout << "ProcessRingstate" << std::endl;
    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);
    CHORD_LOG ("Ringstate From Node: " << fromNode << ", At Node: " << message.GetRingstate().NodeId);
    std::string my;
  std::string pre;
  std::string suc;
  hashtostr(m_my_hash, my);
  hashtostr(m_pred_hash, pre);
  hashtostr(m_suc_hash, suc);
    if(message.GetRingstate().NodeId != ReverseLookup(m_local))
    {
      // std::cout << "ProcessRingstate" << std::endl;
      // std::cout <<  "Current NodeId: " << ReverseLookup(m_local) << std::endl;
      // std::cout <<  "Current IpAddress: " << m_local << std::endl;
      // //std::cout <<  "Current Hash of NodeId: " << m_my_hash << std::endl;//Hash(ReverseLookup(m_local)) << std::endl);
      // std::cout <<  "Predecessor NodeId: " << m_pred << std::endl;
      // std::cout <<  "Predecessor IpAddress: " << ResolveNodeIpAddress(m_pred) << std::endl;
      // //std::cout <<  "Predecessor Hash of NodeId: "<< m_pred_hash<< std::endl;//Hash(m_pred) << std::endl);
      // std::cout <<  "Successor NodeId: " << m_suc << std::endl;
      // std::cout <<  "Successor IpAddress: " << ResolveNodeIpAddress(m_suc) << std::endl;
      // //std::cout <<  "Successor Hash of NodeId: " << m_suc_hash<< std::endl;//Hash(m_suc << std::endl);
      CHORD_LOG ("Ringstate<" << my << ">: Pred<" << m_pred << ", " << pre << ">: Succ<" << m_suc << ", " << suc << ">");
      GUChordMessage nextRing = GUChordMessage (GUChordMessage::RINGSTATE, message.GetTransactionId());
      nextRing.SetRingstate (message.GetRingstate().NodeId);
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (nextRing);
      m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(m_suc), m_appPort));
    }
    else if (ReverseLookup(m_local) == m_suc)
    {
      CHORD_LOG ("Ringstate<" << my << ">: Pred<" << m_pred << ", " << pre << ">: Succ<" << m_suc << ", " << suc << ">");
      // std::cout << "ProcessRingstate" << std::endl;
      // std::cout <<  "Current NodeId: " << ReverseLookup(m_local) << std::endl;
      // std::cout <<  "Current IpAddress: " << m_local << std::endl;
      // //std::cout <<  "Current Hash of NodeId: " << m_my_hash<< std::endl;//Hash(ReverseLookup(m_local)) << std::endl);
      // std::cout <<  "Predecessor NodeId: " << m_pred << std::endl;
      // std::cout <<  "Predecessor IpAddress: " << ResolveNodeIpAddress(m_pred) << std::endl;
      // //std::cout <<  "Predecessor Hash of NodeId: "<< m_pred_hash <<std::endl;//Hash(m_pred) << std::endl);
      // std::cout <<  "Successor NodeId: " << m_suc << std::endl;
      // std::cout <<  "Successor IpAddress: " << ResolveNodeIpAddress(m_suc) << std::endl;
      // //std::cout <<  "Successor Hash of NodeId: " << m_suc_hash<< std::endl;
    }
    // Send indication to application layer
    // m_pingRecvFn (sourceAddress, message.GetPingReq().pingMessage);
}

void
GUChord::ProcessGetPredSucReq(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessPredSucReq" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  CHORD_LOG ("Received PredSucReq, From Node: " << fromNode << " to node : " << message.GetGetPredSucReq().NodeId);
  GUChordMessage resp = GUChordMessage (GUChordMessage::GET_PRED_SUC_RSP, message.GetTransactionId());
  resp.SetGetPredSucRsp (m_pred);
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (resp);
  m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
}

void
GUChord::ProcessGetPredSucRsp(GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // std::cout << "ProcessPredSucRsp" << std::endl;
  std::string fromNode = ReverseLookup (sourceAddress);
  CHORD_LOG ("Received PredSucRsp, From Node: " << fromNode << " to node : " << message.GetGetPredSucRsp().NodeId);
  unsigned char hash[20];
  SHA1((unsigned char*)message.GetGetPredSucRsp().NodeId.c_str(), message.GetGetPredSucRsp().NodeId.size(), hash);
  if(memcmp(hash,m_my_hash, 20) == 0)
  {
    // std::cout << "WORKING!!!" << std::endl;
    return;
  }
  else if(memcmp(hash, m_my_hash, 20) > 0)
  {
    m_suc = message.GetGetPredSucRsp().NodeId;
    SHA1((unsigned char*)m_pred.c_str(), m_suc.size(), m_suc_hash);
    GUChordMessage resp = GUChordMessage (GUChordMessage::NOTIFY_SUC, message.GetTransactionId());
    resp.SetNotifySuc(ReverseLookup(m_local));
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(m_suc), m_appPort));
  }
}

void
GUChord::ProcessPingReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{

    // Use reverse lookup for ease of debug
    std::string fromNode = ReverseLookup (sourceAddress);
    CHORD_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << message.GetPingReq().pingMessage);
    // Send Ping Response
    GUChordMessage resp = GUChordMessage (GUChordMessage::PING_RSP, message.GetTransactionId());
    resp.SetPingRsp (message.GetPingReq().pingMessage);
    Ptr<Packet> packet = Create<Packet> ();
    packet->AddHeader (resp);
    m_socket->SendTo (packet, 0 , InetSocketAddress (sourceAddress, sourcePort));
    // Send indication to application layer
    m_pingRecvFn (sourceAddress, message.GetPingReq().pingMessage);
}

void
GUChord::ProcessPingRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort)
{
  // Remove from pingTracker
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  iter = m_pingTracker.find (message.GetTransactionId ());
  if (iter != m_pingTracker.end ())
    {
      std::string fromNode = ReverseLookup (sourceAddress);
      CHORD_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << message.GetPingRsp().pingMessage);
      m_pingTracker.erase (iter);
      // Send indication to application layer
      m_pingSuccessFn (sourceAddress, message.GetPingRsp().pingMessage);
    }
  else
    {
      DEBUG_LOG ("Received invalid PING_RSP!");
    }
}

void
GUChord::AuditPings ()
{
  std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
  for (iter = m_pingTracker.begin () ; iter != m_pingTracker.end();)
    {
      Ptr<PingRequest> pingRequest = iter->second;
      if (pingRequest->GetTimestamp().GetMilliSeconds() + m_pingTimeout.GetMilliSeconds() <= Simulator::Now().GetMilliSeconds())
        {
          DEBUG_LOG ("Ping expired. Message: " << pingRequest->GetPingMessage () << " Timestamp: " << pingRequest->GetTimestamp().GetMilliSeconds () << " CurrentTime: " << Simulator::Now().GetMilliSeconds ());
          // Remove stale entries
          m_pingTracker.erase (iter++);
          // Send indication to application layer
          m_pingFailureFn (pingRequest->GetDestinationAddress(), pingRequest->GetPingMessage ());
        }
      else
        {
          ++iter;
        }
    }
  // Rechedule timer
  m_auditPingsTimer.Schedule (m_pingTimeout); 
}

uint32_t
GUChord::GetNextTransactionId ()
{
  return m_currentTransactionId++;
}

void
GUChord::StopChord ()
{
  StopApplication ();
}

void
GUChord::SetPingSuccessCallback (Callback <void, Ipv4Address, std::string> pingSuccessFn)
{
  m_pingSuccessFn = pingSuccessFn;
}


void
GUChord::SetPingFailureCallback (Callback <void, Ipv4Address, std::string> pingFailureFn)
{
  m_pingFailureFn = pingFailureFn;
}

void
GUChord::SetPingRecvCallback (Callback <void, Ipv4Address, std::string> pingRecvFn)
{
  m_pingRecvFn = pingRecvFn;
}

// void
// GUChord::SetGUSearchCallback(Callback <void, uint32_t, bool, Ipv4Address> GUsearchFoundKey)
// {
//   m_GUsearchFoundKey = GUsearchFoundKey;
// }

void
GUChord::Stabilize()
{
  // std::cout << "stabilize" << std::endl;
  CHORD_LOG ("Calling stabilize");
  uint32_t transactionId = GetNextTransactionId ();
  GUChordMessage resp = GUChordMessage (GUChordMessage::GET_PRED_SUC_REQ, transactionId);
  resp.SetGetPredSucReq (ReverseLookup(m_local));
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (resp);
  m_socket->SendTo (packet, 0 , InetSocketAddress (ResolveNodeIpAddress(m_suc), m_appPort));
  m_stabilizeTimer.Schedule(m_stabilizeTimeout);
}


