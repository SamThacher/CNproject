#ifndef TABLES_H
#define TABLES_H

#include "ns3/ipv4.h"
#include <ns3/nstime.h>
#include <vector>

using namespace ns3;

struct nTableEntry
{
   uint32_t nodeNumber;
   Ipv4Address NeighborAddress;
   Ipv4Address InterfaceAddress;
   Time tStamp;
   nTableEntry();
   nTableEntry(Ipv4Address, Ipv4Address, uint32_t, Time);
//   ~nTableEntry();
};

struct rTableEntry
{
   uint32_t DestinationNumber;
   uint32_t NextHopNumber;
   uint16_t dijCost;
   Ipv4Address DestinationAddress;
   Ipv4Address NextHopAddress;
   Ipv4Address InterfaceAddress;
   rTableEntry();
   rTableEntry(uint32_t, Ipv4Address, uint32_t, Ipv4Address, Ipv4Address, uint16_t);
   rTableEntry(uint32_t, Ipv4Address, Ipv4Address);
};

class routeTable
{
   public:
     void rTableInsert (rTableEntry entry);
     bool isNew(rTableEntry);
     int isNewPos(rTableEntry);
     routeTable();
     int size;
     rTableEntry at(int) const;
     Ipv4Address nextHop(Ipv4Address);
     void remove(int);
   private:
     std:: vector<rTableEntry> table;
};

class neighborTable
{
  public:
   void nTableInsert (nTableEntry entry);
//   void checkNeighborTableEntry();
   neighborTable();
//   ~neighborTable();
   int size;
   nTableEntry at(int) const;
   void remove(int);
  private:
 std:: vector<nTableEntry> table;
  
};

#endif
