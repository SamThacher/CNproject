#include "tables.h"

using namespace ns3;

neighborTable::neighborTable()
{ size = 0; }

void neighborTable::nTableInsert (nTableEntry entry)
{
  table.push_back(entry);
  size++;
}
/*
void neighborTable::checkNeighborTableEntry()
{
 for(int i = 0; i < size; i++)
 {
   std::time_t curTime = std::time(0);
   if( curTime - table.at(i).tStamp >= 10)
     table.erase(table.begin() + i);
 }
}
*/

nTableEntry
neighborTable::at(int pos)
{
  return table.at(pos);
}


nTableEntry::nTableEntry(Ipv4Address nAdd, Ipv4Address iAdd, uint32_t nNum, Time time)
{
  nodeNumber = nNum;
  NeighborAddress = nAdd;
  InterfaceAddress = iAdd;
  tStamp = time;
}

nTableEntry::nTableEntry()
{ }

