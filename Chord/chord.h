#ifndef CHORD_H
#define CHORD_H


#include "ns3/ipv4-address.h"

using namespace ns3;
class chord_node{
public:
	chord_node *next;
	chord_node *prev;
	int key_id;
	int node_id;
	chord_node();
	chord_node(Ipv4Address, int);
	~chord_node();
	chord_node(int);
	void notify(chord_node *);
	void join(chord_node*); //node_id (hashed ip address)
	void stabilize();
	void leave_chord(); // node_id (hashed ip address)
	void chord_debug();
	bool is_root;

};


class chord{
public:
	chord_node *lmnode;
	chord();
	chord(Ipv4Address, int);
	~chord();
	void join(chord_node*); //node_id (hashed ip address)
	void stabilize();
	void leave_chord(int); // node_id (hashed ip address)
	void chord_debug();
	//chord_node find_node(int);
};


#endif