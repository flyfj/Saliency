#pragma once

#include <assert.h>
#include <utility>
#include <list>
#include <vector>
using namespace std;

// specialized hash_map with key_type = unsigned int and maximal bucket size (N) known in advance
// as N buckets are allocated in advance, conflict never happens and query is strictly constant time
// insert/remove is about 3-4 times faster than hash_map, refer to test_array_hash_map_performance()
// template <class _Ty>
template <typename _Ty>
class array_hash_map : protected list< pair<unsigned int, typename _Ty> >
{	
	typedef list< pair<unsigned int, typename _Ty> > BaseClass;

public:	
	typedef typename BaseClass::value_type value_type;
	typedef unsigned int key_type;
	typedef _Ty mapped_type;
	typedef typename BaseClass::iterator iterator;
	typedef typename BaseClass::const_iterator const_iterator;
	using BaseClass::begin;
	using BaseClass::end;
	using BaseClass::size;
	
	typedef pair<iterator, bool> _Pairib;
	
	array_hash_map(){}
	explicit array_hash_map(unsigned int l) : iters(l, end()){}

	unsigned int length() const {	return iters.size();	}
	void resize(unsigned int length) 
	{
		BaseClass::clear();
		iters.resize(length, end());	
	}

	void clear()
	{
		BaseClass::clear();
		vector<BaseClass::iterator>::size_type length = iters.size();
		iters.resize(0);
		iters.resize(length, end());
	}

	// modification	
	const_iterator find(const unsigned int& n) const
	{
		assert(n < iters.size());
		return iters[n];
	}
	
	iterator find(const unsigned int& n)
	{
		assert(n < iters.size());
		return iters[n];
	}
	
	_Pairib insert(const value_type& _Val)
	{	
		const unsigned int& n = _Val.first;
		assert(n < iters.size());
		return (iters[n] != end()) ? _Pairib(iters[n], false)
			: _Pairib(iters[n]=BaseClass::insert(end(), _Val), true);
	}

	iterator erase(iterator _Where)
	{	
		unsigned int& n = _Where->first;
		assert(n < iters.size());
		iters[n] = end();
		return BaseClass::erase(_Where);
	}

private:
	// vector<BasicClass::iterator> iters;
	vector<iterator> iters;		// a vector of iterators pointing to list elements
};

void test_array_hash_map();