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

#ifndef GU_CHORD_H
#define GU_CHORD_H

#include "ns3/gu-application.h"
#include "ns3/gu-chord-message.h"
#include "ns3/ping-request.h"
#include "ns3/chord.h"
#include "ns3/ipv4-address.h"

#include <openssl/sha.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include "ns3/socket.h"
#include "ns3/nstime.h"
#include "ns3/timer.h"
#include "ns3/uinteger.h"
#include "ns3/boolean.h"

using namespace ns3;

class GUChord : public GUApplication
{
  public:
    static TypeId GetTypeId (void);
    GUChord ();
    //chord_node node;
    virtual ~GUChord ();

    void SendPing (Ipv4Address destAddress, std::string pingMessage);
    void RecvMessage (Ptr<Socket> socket);
    void ProcessPingReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessPingRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessJoinReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessJoinRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessFindSucReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessFindSucRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessGetPredSucReq (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessGetPredSucRsp (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessNotifyPred (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessNotifySuc (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    void ProcessRingstate (GUChordMessage message, Ipv4Address sourceAddress, uint16_t sourcePort);
    bool CompareHash(unsigned char*, unsigned char*);
    void AuditPings ();
    uint32_t GetNextTransactionId ();
    void StopChord ();
    void joinChord(std::string);
    void createChord();
    void Stabilize();
    void startRingstate();
    void nodeLeave();
    void hashtostr(unsigned char*, std::string &);

    // Callback with Application Layer (add more when required)
    void SetPingSuccessCallback (Callback <void, Ipv4Address, std::string> pingSuccessFn);
    void SetPingFailureCallback (Callback <void, Ipv4Address, std::string> pingFailureFn);
    void SetPingRecvCallback (Callback <void, Ipv4Address, std::string> pingRecvFn);

    // From GUApplication
    virtual void ProcessCommand (std::vector<std::string> tokens);
    
  protected:
    virtual void DoDispose ();
    
  private:
    virtual void StartApplication (void);
    virtual void StopApplication (void);

    uint32_t m_currentTransactionId;
    Ptr<Socket> m_socket;
    Time m_pingTimeout;
    Time m_stabilizeTimeout;
    uint16_t m_appPort;
    std::string m_pred;
    std::string m_suc;
    unsigned char m_pred_hash [20];
    unsigned char m_suc_hash [20];
    unsigned char m_my_hash [20];
    // Timers
    Timer m_auditPingsTimer;
    Timer m_stabilizeTimer;
    // Ping tracker
    std::map<uint32_t, Ptr<PingRequest> > m_pingTracker;
    // Callbacks
    Callback <void, Ipv4Address, std::string> m_pingSuccessFn;
    Callback <void, Ipv4Address, std::string> m_pingFailureFn;
    Callback <void, Ipv4Address, std::string> m_pingRecvFn;
};

#endif


