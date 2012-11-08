#pragma once

#include <iostream>
#include <string>
using namespace std;


//typedef unsigned int HistSparseBinCountType;
// the bin count should always be positive, the sign is used in
//HistSparse::find_and_mark() and AccuHistArrayIncr::update_and_compute()
typedef int HistSparseBinCountType;

#define USING_ARRRY_HASH_MAP
#ifdef USING_ARRRY_HASH_MAP

// using array_hash_map can significantly improve insert/erase
#include <array_hash_map.h>
typedef array_hash_map<HistSparseBinCountType> hash_uint_map_base;

#else

#include <hash_map>
using namespace stdext;
typedef hash_map<unsigned int, HistSparseBinCountType> hash_uint_map_base;

#endif

// an associate container that contains only non-zero bin counters of a histogram
class HistSparse : protected hash_uint_map_base
{
	typedef hash_uint_map_base BaseClass;

	unsigned int hist_length;

public:
#ifdef USING_ARRRY_HASH_MAP
	static string ConfigName() {	return "ArrMap";	}
#else
	static string ConfigName() {	return "Map";	}
#endif	

	static unsigned int s_numOfAddCalls, s_numOfRemoveCalls;
	static unsigned int s_numOfInsert, s_numOfErase;

	using BaseClass::iterator;			// only for AccuHistArrayIncr::update_and_compute
	using BaseClass::const_iterator;
	using BaseClass::begin;
	using BaseClass::end;

	explicit HistSparse(unsigned int length) : hist_length(length)
#ifdef USING_ARRRY_HASH_MAP	
		,BaseClass(length)
#endif
	{}

	HistSparse() : hist_length(0)
#ifdef USING_ARRRY_HASH_MAP	
		,BaseClass(0)
#endif
	{}

	unsigned int Length() const {	return hist_length;	}

	void resize(unsigned int length) 
	{ 
		hist_length = length;
#ifdef USING_ARRRY_HASH_MAP	
		BaseClass::resize(length);
#endif
	}

	void reset()	{	clear();	}

	void operator+=(const unsigned int& b) {	Add(b, 1);	}
	void operator-=(const unsigned int& b) {	Remove(b, 1);	}
	void operator+=(const HistSparse& h) {	for(const_iterator it = h.begin(); it != h.end(); it++) Add(it->first, it->second);	}
	void operator-=(const HistSparse& h) {	for(const_iterator it = h.begin(); it != h.end(); it++)	Remove(it->first, it->second);	}

	unsigned int operator[](const unsigned int& b) const
	{
		const_iterator it = find(b);
		return it != end() ? it->second : 0;
	}

	template<class _Ty>
	bool find_and_mark(const unsigned int& b, _Ty& count)
	{
		iterator it = find(b);
		if (it == end()) return false;
		
		count = it->second;
		it->second = -it->second;
		return true;
	}

protected:
	void Add(const unsigned int& b, const HistSparseBinCountType& count)	
	{	
		s_numOfAddCalls++;
		// (*this)[b] += count;	// this is simpler but invalid since operator[] is re-defined
		iterator it = find(b);
		if (it != end()) 
			it->second += count;
		else 
		{
			insert(make_pair(b, count));
			s_numOfInsert++;
		}
	}

	void Remove(const unsigned int& b, const HistSparseBinCountType& count)
	{
		s_numOfRemoveCalls++;
		iterator it = find(b);	assert(it != end());
		if (it->second > count)	it->second -= count;
		else 
		{
			if (it->second < count) 
				cerr << "error in HistSparse::Remove() : bin " << b << " is less than 0" << endl;
			erase(it);	// it->second == count
			s_numOfErase++;
		}
	}
};

#define USE_FAST_ARRAY
#ifdef USE_FAST_ARRAY
#include <fast_array.h>
typedef fast_array_ushort UShortArrayType;
#else
#include <valarray>
typedef valarray<unsigned short> UShortArrayType;
#endif

// bin max value is sizeof(unsigned short) = 65535
// therefore the sliding window cannot be larger than 256x256
class HistArray : protected UShortArrayType
{
	typedef UShortArrayType BaseClass;

public:
	static string ConfigName() {	return "Vector";	}

	explicit HistArray(unsigned int size) : BaseClass(static_cast<value_type>(0), size){}
	HistArray() {}

	using BaseClass::size;
	unsigned int Length() const {	return size();	}
	using BaseClass::operator[];
	using BaseClass::resize;
	void reset()	{	for(unsigned int n = 0; n < size(); n++)		(*this)[n] = 0;	}

	void operator+=(const unsigned int& b)	{	(*this)[b]++;	}
	void operator-=(const unsigned int& b) {	(*this)[b]--;	assert((*this)[b]>=0 && "bin count cannot be less than 0");	}
	void operator+=(const HistArray& h)	{	BaseClass::operator+=(h);	}
	void operator-=(const HistArray& h)	{	BaseClass::operator-=(h);	}
	void operator+=(const HistSparse& h) { for(HistSparse::const_iterator it = h.begin(); it != h.end(); it++) (*this)[it->first] += it->second; }
	void operator-=(const HistSparse& h) { for(HistSparse::const_iterator it = h.begin(); it != h.end(); it++) (*this)[it->first] -= it->second; }
};