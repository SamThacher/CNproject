#include "chord.h"

using namespace ns3;
using namespace std;

chord_node::chord_node(){
	prev = NULL;
	next = NULL;
	key_id = 0;
	node_id = 0;
}

chord_node::chord_node(Ipv4Address ipadd, int key){
	prev = NULL;
	next = NULL;
	int iphash = 0;
	int keyhash= 0;
	key_id = keyhash;
	node_id = iphash;
}

chord::chord(Ipv4Address ipadd, int key){
	chord_node *main = new chord_node(ipadd,key);
	//hash ipadd and hash key
	lmnode = main;
}
/*
chord_node chord::find_node(int id){
	//find chord with node_id id
	return new chord_node();
}
*/

void chord::join(chord_node* node){
	chord_node * current = lmnode;
	chord_node *main = node;
	if(current->next == NULL)
	{
		current->next = main;
		current->prev = main;
		main->next = current;
		main->prev = current;

		// if node added is smaller than the 
	}
	else
	{
		while(current->node_id < node->node_id)
		{
			current = current->next;
		}

		main->next = current;
		main->prev = current->prev;
		current->prev = main;
	}
}

void chord::leave_chord(int id){
	chord_node *current = lmnode;
	while(current->node_id != id)
	{
		current = current->next;
	}
	if(current->node_id == id){
		current->prev->next = current->next;
		current->next->prev = current->prev;
	} else{
		cout << "FAIL" << endl;
	}
}

void chord::stabilize(){
	chord_node *current = lmnode;
	chord_node *tmp = current->next->prev;
	if(tmp->node_id > current->node_id){
		current->next = tmp;
	}
	tmp->notify(current);
}

void chord_node::notify(chord_node* node){
	if(node->prev == NULL || this->prev->node_id > node->node_id){
		this->prev = node;
	}
}