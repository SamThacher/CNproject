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


class neighborTable
{
  public:
   void nTableInsert (nTableEntry entry);
//   void checkNeighborTableEntry();
   neighborTable();
//   ~neighborTable();
   int size;
   nTableEntry at(int);

  private:
 std:: vector<nTableEntry> table;
  
};

#endif
