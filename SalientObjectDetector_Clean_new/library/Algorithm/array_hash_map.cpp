#include "array_hash_map.h"

#include <stdlib.h>
#include <iostream>
#include <hash_map>
using namespace stdext;

#include <time_ex.h>

template<class Map>
void print_map(Map& m)
{
	for(Map::iterator it = m.begin(); it != m.end(); it++)
		cout << "(" << it->first << ", " << it->second << ") ";
	cout << endl << endl;
}

template<class Map>
void insert_random(Map& m, unsigned int pos)
{	
	Map::_Pairib ib = m.insert(make_pair(pos, -1));
	cout << "insert at pos " << pos << (ib.second ? " succeeds" : " fails") << endl;
	if (ib.second)
		print_map(m);
}

template<class Map>
void remove_random(Map& m, unsigned int pos)
{	
	Map::iterator it = m.find(pos);
	if (it != m.end())
	{
		m.erase(it);
		cout << "erase at pos " << pos << endl;
		print_map(m);
	}
	else cout << "cannot erase pos " << pos << endl;
}

void test_array_hash_map_soundness()
{
	const unsigned int LENGTH = 10;
	hash_map<unsigned int, int> hashmap;
	array_hash_map<int> arraymap(LENGTH);

	for(int n = 0; n < 10; n++)
	{
		unsigned int pos = rand() % LENGTH;
		insert_random(arraymap, pos);
		insert_random(hashmap, pos);
	}

	for(int n = 0; n < 10; n++)
	{
		unsigned int pos = rand() % LENGTH;
		//if (rand() % 2)	insert_random(arraymap, LENGTH);
		//else 
			remove_random(arraymap, pos);
			remove_random(hashmap, pos);
	}
}

void test_array_hash_map_performance()
{
	const unsigned int LENGTH = 100000;
	const unsigned int N_TRIALS = 200000;

	unsigned int random_pos[N_TRIALS];	// larger N_TRIALS causes memory error
	for(int n = 0; n < N_TRIALS; n++)
		random_pos[n] = rand() % LENGTH;

	hash_map<unsigned int, int> hashmap;
	array_hash_map<int> arraymap(LENGTH);

	cout << "array length : " << LENGTH << ", number of trials : " << N_TRIALS << endl;
	{
		Timer t;
		for(int n = 0; n < N_TRIALS; n++)			
		{
			arraymap.insert(make_pair(random_pos[n], -1));
		}
		cout << "array map insert : " << t.Stamp() << " seconds, " << arraymap.size() << " elements" << endl;
	}
	
	{
		Timer t;
		for(int n = 0; n < N_TRIALS; n++)
		{			
			hashmap.insert(make_pair(random_pos[n], -1));
		}
		cout << "hash map insert : " << t.Stamp() << " seconds, " << hashmap.size() << " elements " << endl;
	}

	for(int n = 0; n < N_TRIALS; n++)
		random_pos[n] = rand() % LENGTH;

	// under debug mode, array map erase is much slower than hash map erase, don't know why...
	{
		Timer t;
		for(int n = 0; n < N_TRIALS; n++)
		{			
			array_hash_map<int>::iterator it = arraymap.find(random_pos[n]);
			if (it != arraymap.end())	arraymap.erase(it);
		}
		cout << "array map erase : " << t.Stamp() << " seconds, " << arraymap.size() << " elements" << endl;
	}

	{
		Timer t;
		for(int n = 0; n < N_TRIALS; n++)
		{
			hash_map<unsigned int, int>::iterator it = hashmap.find(random_pos[n]);
			if (it != hashmap.end())	hashmap.erase(it);
		}
		cout << "hash map erase : " << t.Stamp() << " seconds, " << hashmap.size() << " elements" << endl;
	}	
}

void test_array_hash_map()
{
	test_array_hash_map_performance();
}