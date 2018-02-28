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


#include "ns3/ls-routing-protocol.h"
#include "ns3/socket-factory.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/random-variable.h"
#include "ns3/inet-socket-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv4-route.h"
#include "ns3/uinteger.h"
#include "ns3/test-result.h"
#include <sys/time.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("LSRoutingProtocol");
NS_OBJECT_ENSURE_REGISTERED (LSRoutingProtocol);

TypeId
LSRoutingProtocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("LSRoutingProtocol")
  .SetParent<GURoutingProtocol> ()
  .AddConstructor<LSRoutingProtocol> ()
  .AddAttribute ("LSPort",
                 "Listening port for LS packets",
                 UintegerValue (5000),
                 MakeUintegerAccessor (&LSRoutingProtocol::m_lsPort),
                 MakeUintegerChecker<uint16_t> ())
  .AddAttribute ("PingTimeout",
                 "Timeout value for PING_REQ in milliseconds",
                 TimeValue (MilliSeconds (2000)),
                 MakeTimeAccessor (&LSRoutingProtocol::m_pingTimeout),
                 MakeTimeChecker ())
  .AddAttribute ("NdTimeout",
                 "Timeout value for ND_REQ in milliseconds",
                 TimeValue (MilliSeconds (100)),
                 MakeTimeAccessor (&LSRoutingProtocol::m_ndTimeout),
                 MakeTimeChecker ())
  .AddAttribute ("NdLong",
                 "Timeout value for ND_REQ in milliseconds",
                 TimeValue (MilliSeconds (10000)),
                 MakeTimeAccessor (&LSRoutingProtocol::m_ndLong),
                 MakeTimeChecker ())

  .AddAttribute ("MaxTTL",
                 "Maximum TTL value for LS packets",
                 UintegerValue (16),
                 MakeUintegerAccessor (&LSRoutingProtocol::m_maxTTL),
                 MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

LSRoutingProtocol::LSRoutingProtocol ()
  : m_auditPingsTimer (Timer::CANCEL_ON_DESTROY), m_checkNeighborTimer (Timer::CANCEL_ON_DESTROY)
{
  RandomVariable random;
  SeedManager::SetSeed (time (NULL));
  random = UniformVariable (0x00000000, 0xFFFFFFFF);
  m_currentSequenceNumber = random.GetInteger ();
  // Setup static routing 
  m_staticRouting = Create<Ipv4StaticRouting> ();
}

LSRoutingProtocol::~LSRoutingProtocol ()
{
}

void 
LSRoutingProtocol::DoDispose ()
{
  // Close sockets
  for (std::map< Ptr<Socket>, Ipv4InterfaceAddress >::iterator iter = m_socketAddresses.begin ();
       iter != m_socketAddresses.end (); iter++)
    {
      iter->first->Close ();
    }
  m_socketAddresses.clear ();
  
  // Clear static routing
  m_staticRouting = 0;

  // Cancel timers
  m_auditPingsTimer.Cancel ();
  m_checkNeighborTimer.Cancel(); 

  m_pingTracker.clear (); 
//  m_checkNeighborTimer.clear();

  GURoutingProtocol::DoDispose ();
}

void
LSRoutingProtocol::SetMainInterface (uint32_t mainInterface)
{
  m_mainAddress = m_ipv4->GetAddress (mainInterface, 0).GetLocal ();
}

void
LSRoutingProtocol::SetNodeAddressMap (std::map<uint32_t, Ipv4Address> nodeAddressMap)
{
  m_nodeAddressMap = nodeAddressMap;
}

void
LSRoutingProtocol::SetAddressNodeMap (std::map<Ipv4Address, uint32_t> addressNodeMap)
{
  m_addressNodeMap = addressNodeMap;
}

Ipv4Address
LSRoutingProtocol::ResolveNodeIpAddress (uint32_t nodeNumber)
{
  std::map<uint32_t, Ipv4Address>::iterator iter = m_nodeAddressMap.find (nodeNumber);
  if (iter != m_nodeAddressMap.end ())
    { 
      return iter->second;
    }
  return Ipv4Address::GetAny ();
}

std::string
LSRoutingProtocol::ReverseLookup (Ipv4Address ipAddress)
{
  std::map<Ipv4Address, uint32_t>::iterator iter = m_addressNodeMap.find (ipAddress);
  if (iter != m_addressNodeMap.end ())
    { 
      std::ostringstream sin;
      uint32_t nodeNumber = iter->second;
      sin << nodeNumber;    
      return sin.str();
    }
  return "Unknown";
}

void
LSRoutingProtocol::DoStart ()
{
  // Create sockets
  for (uint32_t i = 0 ; i < m_ipv4->GetNInterfaces () ; i++)
    {
      Ipv4Address ipAddress = m_ipv4->GetAddress (i, 0).GetLocal ();
      if (ipAddress == Ipv4Address::GetLoopback ())
        continue;
      // Create socket on this interface
      Ptr<Socket> socket = Socket::CreateSocket (GetObject<Node> (),
          UdpSocketFactory::GetTypeId ());
      socket->SetAllowBroadcast (true);
      InetSocketAddress inetAddr (m_ipv4->GetAddress (i, 0).GetLocal (), m_lsPort);
      socket->SetRecvCallback (MakeCallback (&LSRoutingProtocol::RecvLSMessage, this));
      if (socket->Bind (inetAddr))
        {
          NS_FATAL_ERROR ("LSRoutingProtocol::DoStart::Failed to bind socket!");
        }
      Ptr<NetDevice> netDevice = m_ipv4->GetNetDevice (i);
      socket->BindToNetDevice (netDevice);
      m_socketAddresses[socket] = m_ipv4->GetAddress (i, 0);
    }
  // Configure timers
  m_auditPingsTimer.SetFunction (&LSRoutingProtocol::AuditPings, this);
  m_checkNeighborTimer.SetFunction (&LSRoutingProtocol::checkNTEntry, this);

  // Start timers
  m_auditPingsTimer.Schedule (m_pingTimeout);
  m_checkNeighborTimer.Schedule (m_ndTimeout);

        uint32_t sequenceNumber = GetNextSequenceNumber ();
      TRAFFIC_LOG ("Sending ND_REQ from Node: " << ReverseLookup(m_mainAddress)  << " IP: " << m_mainAddress << " SequenceNumber: " << sequenceNumber);
      Ptr<Packet> packet = Create<Packet> ();
      LSMessage lsMessage = LSMessage (LSMessage::ND_REQ, sequenceNumber, 1, m_mainAddress);
      lsMessage.SetNdReq ();
      packet->AddHeader (lsMessage);
      BroadcastPacket (packet);


}

Ptr<Ipv4Route>
LSRoutingProtocol::RouteOutput (Ptr<Packet> packet, const Ipv4Header &header, Ptr<NetDevice> outInterface, Socket::SocketErrno &sockerr)
{
  Ptr<Ipv4Route> ipv4Route = m_staticRouting->RouteOutput (packet, header, outInterface, sockerr);
  if (ipv4Route)
    {
      DEBUG_LOG ("Found route to: " << ipv4Route->GetDestination () << " via next-hop: " << ipv4Route->GetGateway () << " with source: " << ipv4Route->GetSource () << " and output device " << ipv4Route->GetOutputDevice());
    }
  else
    {
      DEBUG_LOG ("No Route to destination: " << header.GetDestination ());
    }
  return ipv4Route;
}

bool 
LSRoutingProtocol::RouteInput  (Ptr<const Packet> packet, 
  const Ipv4Header &header, Ptr<const NetDevice> inputDev,                            
  UnicastForwardCallback ucb, MulticastForwardCallback mcb,             
  LocalDeliverCallback lcb, ErrorCallback ecb)
{
  Ipv4Address destinationAddress = header.GetDestination ();
  Ipv4Address sourceAddress = header.GetSource ();

  // Drop if packet was originated by this node
  if (IsOwnAddress (sourceAddress) == true)
    {
      return true;
    }

  // Check for local delivery
  uint32_t interfaceNum = m_ipv4->GetInterfaceForDevice (inputDev);
  if (m_ipv4->IsDestinationAddress (destinationAddress, interfaceNum))
    {
      if (!lcb.IsNull ())
        {
          lcb (packet, header, interfaceNum);
          return true;
        }
      else
        {
          return false;
        }
    }

  // Check static routing table
  if (m_staticRouting->RouteInput (packet, header, inputDev, ucb, mcb, lcb, ecb))
    {
      return true;
    }
  DEBUG_LOG ("Cannot forward packet. No Route to destination: " << header.GetDestination ());
  return false;
}

void
LSRoutingProtocol::BroadcastPacket (Ptr<Packet> packet)
{
  for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
      m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    {
      Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
      i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_lsPort));
    }
}

/*
void
LSRoutingProtocol::BroadcastNdRsp (Ptr<Packet> packet)
{
LSMessage lsMesssage;
packet->RemoveHeader(lsMessage);
  for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
      m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    {
      lsMessage.SetNdRspInterfaceAddr(i->second);
      packet.AddHeader(lsMessage);
      Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
      i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_lsPort));
    }
}
*/


void
LSRoutingProtocol::ProcessCommand (std::vector<std::string> tokens)
{
  std::vector<std::string>::iterator iterator = tokens.begin();
  std::string command = *iterator;
  if (command == "PING")
    {
      if (tokens.size() < 3)
        {
          ERROR_LOG ("Insufficient PING params..."); 
          return;
        }
      iterator++;
      std::istringstream sin (*iterator);
      uint32_t nodeNumber;
      sin >> nodeNumber;
      iterator++;
      std::string pingMessage = *iterator;
      Ipv4Address destAddress = ResolveNodeIpAddress (nodeNumber);
      if (destAddress != Ipv4Address::GetAny ())
        {
          uint32_t sequenceNumber = GetNextSequenceNumber ();
          TRAFFIC_LOG ("Sending PING_REQ to Node: " << nodeNumber << " IP: " << destAddress << " Message: " << pingMessage << " SequenceNumber: " << sequenceNumber);
          Ptr<PingRequest> pingRequest = Create<PingRequest> (sequenceNumber, Simulator::Now(), destAddress, pingMessage);
          // Add to ping-tracker
          m_pingTracker.insert (std::make_pair (sequenceNumber, pingRequest));
          Ptr<Packet> packet = Create<Packet> ();
          LSMessage lsMessage = LSMessage (LSMessage::PING_REQ, sequenceNumber, m_maxTTL, m_mainAddress);
          lsMessage.SetPingReq (destAddress, pingMessage);
          packet->AddHeader (lsMessage);
          BroadcastPacket (packet);
        }
    }
/*  else if (command == "ND")
    {
      uint32_t sequenceNumber = GetNextSequenceNumber ();
      TRAFFIC_LOG ("Sending ND_REQ from Node: " << nodeNumber << " IP: " << destAddress << " SequenceNumber: " << sequenceNumber);
      Ptr<Packet> apcket = Create<Packet> ();
      LSMessage lsMessage = LSMessage (LSMessage::ND_REQ, sequenceNumber, m_maxTTL, m_mainAddress);
      lsMessage.SetNdReq  ();
      packet->AddHeader (lsMessage);
      BroadcastPacket (packet);
    }*/
  else if (command == "DUMP")
    {
      if (tokens.size() < 2)
        {
          ERROR_LOG ("Insufficient Parameters!");
          return;
        }
      iterator++;
      std::string table = *iterator;
      if (table == "ROUTES" || table == "ROUTING")
        {
          DumpRoutingTable ();
        }
      else if (table == "NEIGHBORS" || table == "NEIGHBOURS")
        {
          DumpNeighbors ();
        }
      else if (table == "LSA")
        {
          DumpLSA ();
        }
    }
}

void
LSRoutingProtocol::DumpLSA ()
{
  STATUS_LOG (std::endl << "**************** LSA DUMP ********************" << std::endl
              << "Node\t\tNeighbor(s)");
  PRINT_LOG ("");
}

void
LSRoutingProtocol::DumpNeighbors ()
{
  STATUS_LOG (std::endl << "**************** Neighbor List ********************" << std::endl
              << "NeighborNumber\t\tNeighborAddr\t\tInterfaceAddr");
  PRINT_LOG ("");

  /*NOTE: For purpose of autograding, you should invoke the following function for each
  neighbor table entry. The output format is indicated by parameter name and type.
  */
//    ntable.checkNeighborTableEntry();
//  PRINT_LOG ("Size of the table is :: " << nTable.size << std::endl);
//  PRINT_LOG ("PRNIT IF WORKING");
  
  printnTable();
//  m_checkNeighborTimer.Cancel();

//  checkNTEntry();

}

void
LSRoutingProtocol::printnTable()
{
  PRINT_LOG("Size of table is: " << nTable.size);
  for(int i = 0; i < nTable.size; i++)
  {
     PRINT_LOG (std::endl << nTable.at(i).nodeNumber << "\t"   << nTable.at(i).NeighborAddress << "\t" << nTable.at(i).InterfaceAddress);
  }
}

void
LSRoutingProtocol::DumpRoutingTable ()
{
	STATUS_LOG (std::endl << "**************** Route Table ********************" << std::endl
			  << "DestNumber\t\tDestAddr\t\tNextHopNumber\t\tNextHopAddr\t\tInterfaceAddr\t\tCost");

	PRINT_LOG ("");

	/*NOTE: For purpose of autograding, you should invoke the following function for each
	routing table entry. The output format is indicated by parameter name and type.
	*/
	//checkRouteTableEntry();
}
void
LSRoutingProtocol::RecvLSMessage (Ptr<Socket> socket)
{
  Address sourceAddr;
  Ptr<Packet> packet = socket->RecvFrom (sourceAddr);
  InetSocketAddress inetSocketAddr = InetSocketAddress::ConvertFrom (sourceAddr);
  Ipv4Address sourceAddress = inetSocketAddr.GetIpv4 ();
  LSMessage lsMessage;
  packet->RemoveHeader (lsMessage);

  switch (lsMessage.GetMessageType ())
    {
      case LSMessage::PING_REQ:
        ProcessPingReq (lsMessage);
        break;
      case LSMessage::PING_RSP:
        ProcessPingRsp (lsMessage);
        break;
      case LSMessage::ND_REQ:
	ProcessNdReq (lsMessage);
	break;
      case LSMessage::ND_RSP:
	lsMessage.SetOriginatorAddress(sourceAddress);
//	PRINT_LOG("This is the source(interace) address " << sourceAddress << std::endl)
	ProcessNdRsp (lsMessage);
	break;
      default:
        ERROR_LOG ("Unknown Message Type!");
        break;
    }
}

void LSRoutingProtocol::ProcessPingReq (LSMessage lsMessage) {
  // Check destination address
  if (IsOwnAddress (lsMessage.GetPingReq().destinationAddress))
    {
      // Use reverse lookup for ease of debug
      std::string fromNode = ReverseLookup (lsMessage.GetOriginatorAddress ());
      TRAFFIC_LOG ("Received PING_REQ, From Node: " << fromNode << ", Message: " << lsMessage.GetPingReq().pingMessage);
      // Send Ping Response
      LSMessage lsResp = LSMessage (LSMessage::PING_RSP, lsMessage.GetSequenceNumber(), m_maxTTL, m_mainAddress);
      lsResp.SetPingRsp (lsMessage.GetOriginatorAddress(), lsMessage.GetPingReq().pingMessage);
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (lsResp);
      BroadcastPacket (packet);
    }
}

void
LSRoutingProtocol::ProcessPingRsp (LSMessage lsMessage)
{
  // Check destination address
  if (IsOwnAddress (lsMessage.GetPingRsp().destinationAddress))
    {
      // Remove from pingTracker
      std::map<uint32_t, Ptr<PingRequest> >::iterator iter;
      iter = m_pingTracker.find (lsMessage.GetSequenceNumber ());
      if (iter != m_pingTracker.end ())
        {
          std::string fromNode = ReverseLookup (lsMessage.GetOriginatorAddress ());
          TRAFFIC_LOG ("Received PING_RSP, From Node: " << fromNode << ", Message: " << lsMessage.GetPingRsp().pingMessage);
          m_pingTracker.erase (iter);
        }
      else
        {
          DEBUG_LOG ("Received invalid PING_RSP!");
        }
    }
}

void
LSRoutingProtocol::ProcessNdReq (LSMessage lsMessage)
{
  // Check destination address
	//don't need to right?
//  if (IsOwnAddress (lsMessage.GetNdReq().destinationAddress))
    {
      // Use reverse lookup for ease of debug
      std::string fromNode = ReverseLookup (lsMessage.GetOriginatorAddress ());
      TRAFFIC_LOG ("Received ND_REQ, From Node: " << fromNode << ", Message: " << lsMessage.GetNdReq().ndMessage);
      // Send Nd Response
      LSMessage lsResp = LSMessage (LSMessage::ND_RSP, lsMessage.GetSequenceNumber(), m_maxTTL, m_mainAddress);
      lsResp.SetNdRsp (lsMessage.GetOriginatorAddress (), m_mainAddress);
//	PRINT_LOG("OriginatorAddress is  " << lsMessage.GetOriginatorAddress() << "  mainAddress is  " << m_mainAddress << std::endl);
      Ptr<Packet> packet = Create<Packet> ();
      packet->AddHeader (lsResp);
      BroadcastPacket (packet);
/*
  for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i =
      m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    {
      Ipv4Address orgAdd = lsMessage.GetOriginatorAddress().GetSubnetDirectedBroadcast(i->second.GetMask());
      Ipv4Address broadcastAddr = i->second.GetLocal ().GetSubnetDirectedBroadcast (i->second.GetMask ());
//      Ipv4Address tempAddr = i->second.GetLocal();
      Ipv4Address org2Add = lsMessage.GetOriginatorAddress();

//	PRINT_LOG (org2Add << std::endl);
//	PRINT_LOG (broadcastAddr << std::endl);
//      PRINT_LOG (i->second.GetMask() << " Mask is this " << std::endl);
//      PRINT_LOG ("Original address: " << lsMessage.GetOriginatorAddress().CombineMask(i->second.GetMask()) << "  Broadcast Address: " << broadcastAddr << std::endl);
 //     PRINT_LOG ("Are they equal? " << orgAdd);
//	PRINT_LOG (ReverseLookup(lsMessage.GetOriginatorAddress()) << "  " <<  ReverseLookup(broadcastAddr) << "  " << ReverseLookup(tempAddr) << std::endl);

      if(broadcastAddr == orgAdd)
      {
//	PRINT_LOG ("SENDING NDRSP CORRECTLY" << std::endl);
	i->first->SendTo (packet, 0, InetSocketAddress (broadcastAddr, m_lsPort));
	return;
      }
    }
*/

	// should be one-to-one not broadcast

    }
}

void
LSRoutingProtocol::ProcessNdRsp (LSMessage lsMessage)
{
  // Check destination address
  if (IsOwnAddress (lsMessage.GetNdRsp().destinationAddress))
    {
//      std::map<uint32_t, Ptr<NdRequest> >::iterator iter;
//      iter = m_pingTracker.find (lsMessage.GetSequenceNumber ());
//      if (iter != m_pingTracker.end ())
//        {
	// add to neighbor table entry
//	PRINT_LOG("OriginatorAddress is  " << lsMessage.GetOriginatorAddress() << "  mainAddress is  " << m_mainAddress << std::endl);
	  Ipv4Address interfaceAddress = lsMessage.GetOriginatorAddress ();
	  Ipv4Address sceAddress = lsMessage.GetNdRsp().sourceAddress;
uint32_t nodeNumber;
  std::map<Ipv4Address, uint32_t>::iterator iter = m_addressNodeMap.find (sceAddress);
  if (iter != m_addressNodeMap.end ())
    {
       nodeNumber = iter->second;
    }
//	  Ipv4Address interfaceAddress = lsMessage.GetOriginatorAddress ();
//	  uint32_t nodeNumber = ReverseLookup(interfaceAddress);
//	  Ipv4Address interfaceAddress = lsMessage.GetNdRsp().sourceAddress;
	  nTableEntry entry (sceAddress, interfaceAddress, nodeNumber, Simulator::Now());
	  nTable.nTableInsert(entry);
          //nTable.table.push_back(entry);
	  //nTable.size++;
//	  printnTable();
          std::string fromNode = ReverseLookup (lsMessage.GetOriginatorAddress ());
          TRAFFIC_LOG ("Received ND_RSP, From Node: " << fromNode << ", Message: " << lsMessage.GetNdRsp().ndMessage);
//          m_pingTracker.erase (iter);
     }
//      else
//     {
//          DEBUG_LOG ("Received invalid ND_RSP!");
//     }
}


bool
LSRoutingProtocol::IsOwnAddress (Ipv4Address originatorAddress)
{
  // Check all interfaces
  for (std::map<Ptr<Socket> , Ipv4InterfaceAddress>::const_iterator i = m_socketAddresses.begin (); i != m_socketAddresses.end (); i++)
    {
      Ipv4InterfaceAddress interfaceAddr = i->second;
      if (originatorAddress == interfaceAddr.GetLocal ())
        {
          return true;
        }
    }
  return false;

}

void
LSRoutingProtocol::AuditPings ()
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
        }
      else
        {
          ++iter;
        }
    }
  // Rechedule timer
  m_auditPingsTimer.Schedule (m_pingTimeout); 
}


void
LSRoutingProtocol::checkNTEntry ()
{
//  nTable.checkNeighborTableEntry();
  for(int i = 0; i < nTable.size; i++)
  {
    if(nTable.at(i).tStamp.GetMilliSeconds() + m_ndLong.GetMilliSeconds() <= Simulator::Now().GetMilliSeconds())
    {
         DEBUG_LOG ("Node Discovery expired. Node Number: " << nTable.at(i).nodeNumber << "  Neighbor Address: " << nTable.at(i).NeighborAddress << " InterfaceAddress : " << nTable.at(i).InterfaceAddress);
    }
  }
//  PRINT_LOG (m_checkNeighborTimer.GetDelayLeft());
//  m_checkNeighborTimer.Cancel();
  m_checkNeighborTimer.Schedule (m_ndTimeout);
}


uint32_t
LSRoutingProtocol::GetNextSequenceNumber ()
{
  return m_currentSequenceNumber++;
}

void 
LSRoutingProtocol::NotifyInterfaceUp (uint32_t i)
{
  m_staticRouting->NotifyInterfaceUp (i);
}
void 
LSRoutingProtocol::NotifyInterfaceDown (uint32_t i)
{
  m_staticRouting->NotifyInterfaceDown (i);
}
void 
LSRoutingProtocol::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  m_staticRouting->NotifyAddAddress (interface, address);
}
void 
LSRoutingProtocol::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  m_staticRouting->NotifyRemoveAddress (interface, address);
}

void
LSRoutingProtocol::SetIpv4 (Ptr<Ipv4> ipv4)
{
  m_ipv4 = ipv4;
  m_staticRouting->SetIpv4 (m_ipv4);
}
