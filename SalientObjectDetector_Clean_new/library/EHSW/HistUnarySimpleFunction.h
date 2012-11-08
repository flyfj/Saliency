#pragma once

#include "HistUnaryFunction.h"

// function instances
//class HistL0NormFunc;
//template<class RealType> class HistEntropyFunc;
//class HistFastEntropyFunc;

//////////////////////////////////////////////////////////////////////////
// bin function that only depends on the count but not the index
// CountOp is derived from unary_function
template<class CountOp, class BinIndexType = unsigned int, class BinCountType = unsigned int>
class BinCountFunction : public BinUnaryFunctionBase<typename CountOp::result_type, BinIndexType, BinIndexType>
{	
public:
	CountOp op;
	BinCountFunction(const CountOp& c_op) : op(c_op){}
	bin_result_type operator()(const bin_index_type& b, const bin_count_type& c) const	{	return op(c);	}
};

// function that returns the number of bins whose value is larger than a threshold T (equivalent to L0Norm when T=0)
template<class _Ty>
struct greater_unary : public unary_function<_Ty, unsigned int>
{
	_Ty value;
	greater_unary(const _Ty& _Right) : value(_Right){}
	unsigned int operator()(const _Ty& _Left) const
	{
		return (_Left > value) ? 1 : 0;
	}
};

class HistL0NormFunc : public HistUnaryFunction<BinCountFunction<greater_unary<unsigned int>>>
{
public:
	static string Name() {	return "L0Norm";	}
	// default T=1 for robustness
	HistL0NormFunc(unsigned int hist_length, unsigned int threshold=1) : 
	HistUnaryFunction(hist_length, BinCountFunction<greater_unary<unsigned int>>(greater_unary<unsigned int>(threshold))){}
};

// histogram entropy : f = -\sum p_i/N * ln(p_i/N)
template<class RealType>
struct bin_entropy_fun : public unary_entropy<RealType>
{
	typedef unary_entropy<RealType> Base;

	RealType normalizer;
	bin_entropy_fun(unsigned int TotalCount) {	SetTotalCount(TotalCount);	}

	template<class _Ty>
	RealType operator()(const _Ty& _Left) const	{	return Base::operator()(_Left * normalizer);	}
	void SetTotalCount(unsigned int TotalCount)	{	normalizer = static_cast<RealType>(1.0) / TotalCount;	}
};

struct entropy_normalizing_fun : public unary_function<float, float>
{
	unsigned int N;
	entropy_normalizing_fun(unsigned int TotalCount) : N(TotalCount) {}
	void SetTotalCount(unsigned int TotalCount)	{	N = TotalCount;	}
	float operator()(const float& _Left) const	{	return _Left / logf(N);	}
};

template<class RealType>
class HistEntropyFunc : public HistUnaryFunction<BinCountFunction<bin_entropy_fun<RealType>>, entropy_normalizing_fun>
{
public:
	static string Name() {	return "Entropy";	}
	HistEntropyFunc(unsigned int hist_length, unsigned int TotalCount) 
		: HistUnaryFunction(hist_length, bin_fun_type(bin_entropy_fun<RealType>(TotalCount)), entropy_normalizing_fun(TotalCount)){}

	void SetTotalCount(unsigned int TotalCount)
	{
		bin_fun.op.SetTotalCount(TotalCount);
		sum_op.SetTotalCount(TotalCount);
	}
};

// entropy = -\sum p_i/N * ln(p_i/N) = ln N - (\sum p_i*ln p_i) / N
template<class bin_count_type>
struct bin_entropy_fun_fast : public unary_entropy_int_fast<bin_count_type, float>
{
	typedef unary_entropy_int_fast<bin_count_type, float> Base;

	bin_entropy_fun_fast(const bin_count_type& TotalCount) : Base(TotalCount) {}
	void SetTotalCount(const bin_count_type& TotalCount)
	{
		entropy_result_table.resize(TotalCount+1);
		pre_compute();
	}
};

struct entropy_fast_normalizing_fun : public unary_function<float, float>
{
	unsigned int N;
	entropy_fast_normalizing_fun(unsigned int TotalCount) : N(TotalCount) {}
	void SetTotalCount(unsigned int TotalCount)	{	N = TotalCount;	}

	float operator()(const float& _Left) const
	{
		//return logf(N) - _Left / N;
		return 1.0f - _Left / N / logf(N);
	}
};

class HistFastEntropyFunc : public HistUnaryFunction<BinCountFunction<bin_entropy_fun_fast<unsigned int>>, entropy_fast_normalizing_fun>
{
public:
	static string Name() {	return "EntropyFast";	}
	HistFastEntropyFunc(unsigned int hist_length, unsigned int TotalCount) 
		: HistUnaryFunction(hist_length, bin_fun_type(bin_entropy_fun_fast<unsigned int>(TotalCount)), entropy_fast_normalizing_fun(TotalCount)){}

	void SetTotalCount(unsigned int TotalCount)
	{
		bin_fun.op.SetTotalCount(TotalCount);
		sum_op.SetTotalCount(TotalCount);
	}
};

// a simple and exemplar histogram function: return the bin with most elements
/*
class MaxBinHistFunc
{
unsigned int m_nHistLength;	
public:
// <max bin index, max value>
typedef pair<unsigned short, unsigned int> value_type;

MaxBinHistFunc(unsigned int hist_length) : m_nHistLength(hist_length){}
unsigned int HistLength() const {	return m_nHistLength;	}

// direct computation
value_type Evaluate(const HistArray& hist)
{	
assert(m_nHistLength == hist.Length());		
value_type max_value(0, hist[0]);

for(unsigned int b = 1; b < hist.Length(); b++)
if (hist[b] > max_value.second)
{
max_value.first = b;
max_value.second = hist[b];
}

if (max_value.second < m_nMinMaxCount)
m_nMinMaxCount = max_value.second;

return max_value;
}

value_type Evaluate(const HistSparse& hist)
{	
assert(m_nHistLength == hist.Length());
HistSparse::const_iterator it = hist.begin();
value_type max_value(it->first, it->second);

for(it++; it != hist.end();it++)
if (it->second > max_value.second)
{
max_value.first = it->first;
max_value.second = it->second;
}

if (max_value.second < m_nMinMaxCount)
m_nMinMaxCount = max_value.second;

return max_value;
}

unsigned short m_nMinMaxCount;				// the max count during all evaluations()
void Reset() { m_nMinMaxCount = 65535; }	// max ushort value
};
//*/