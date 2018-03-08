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
neighborTable::at(int pos) const
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


rTableEntry::rTableEntry()
{ }

rTableEntry::rTableEntry(uint32_t destNum, Ipv4Address destAdd, uint32_t hopNum, Ipv4Address hopAdd, Ipv4Address intAdd, uint16_t cost)
{
   DestinationNumber = destNum;
   DestinationAddress = destAdd;
   NextHopNumber = hopNum;
   NextHopAddress = hopAdd;
   InterfaceAddress = intAdd;
   dijCost = cost;
}

rTableEntry::rTableEntry(uint32_t destNum, Ipv4Address destAdd, Ipv4Address intAdd)
{
   DestinationNumber = destNum;
   DestinationAddress = destAdd;
   InterfaceAddress = intAdd;
}

routeTable::routeTable()
{ size = 0; }

void routeTable::rTableInsert (rTableEntry entry)
{
  table.push_back(entry);
  size++;
}

bool routeTable::isNew(rTableEntry entry)
{
  for(int i = 0; i < size; i++){
	if(entry.DestinationNumber == table.at(i).DestinationNumber) {
		if(entry.dijCost < table.at(i).dijCost)
		{return true;}
		else
		{return false;}
	}
  }
  return true;
}

rTableEntry
routeTable::at(int pos) const
{
  return table.at(pos);
}

