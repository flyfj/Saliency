#ifndef DISJOINT_SET_H
#define DISJOINT_SET_H

#include "disjoint_elt.h"

class Disjoint_set
{
public:
	//methods:
	Disjoint_set(int num_elts);

	~Disjoint_set();
	int find(int x);
	int joint(int x, int y);
public:
	//members:
	disjoint_elt *m_elts;

};

#endif