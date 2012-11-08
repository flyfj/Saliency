#pragma once

// sometimes this head file causes compilation confliction and should be included at last

#include <assert.h>
#include <emmintrin.h>

// fast array performs arithmetic operations using SIMD instructions
// the array memory must be generally 16-byte aligned
template <class _Ty>
class fast_array_base
{
public:
	typedef _Ty value_type;

	explicit fast_array_base(unsigned int size = 0) : _num_op(16 / sizeof(value_type)), _ptr(0), _size(0), _padded_size(0)	{	allocate(size);	}
	fast_array_base(const _Ty& _Val, unsigned int size) : _num_op(16 / sizeof(value_type)), _ptr(0), _size(0), _padded_size(0)	{	allocate(size, &_Val);	}
	fast_array_base(const _Ty *_Ptr, unsigned int size) : _num_op(16 / sizeof(value_type)), _ptr(0), _size(0), _padded_size(0)	{	allocate(size, _Ptr, 1);	}
	fast_array_base(const fast_array_base<_Ty>& a) : _num_op(16 / sizeof(value_type)), _ptr(0), _size(0), _padded_size(0)	{	allocate(a.size(), a._ptr, 1);	}

	//template<class ArrayType>	
	//explicit fast_array_base(ArrayType& a) : _num_op(128 / sizeof(value_type)), _ptr(0), _size(0), _padded_size(0)
	//{	allocate(a.size(), &a[0], 1);	}

	~fast_array_base()	{	clear();	}

	fast_array_base<_Ty>& operator=(const fast_array_base<_Ty>& a)
	{	
		if (this == &a)	;	// do nothing
		else 
			if (size() == a.size())
				for (unsigned int _Idx = 0; _Idx < size(); ++_Idx) _ptr[_Idx] = a._ptr[_Idx];
				// memcpy(_ptr, a._ptr, sizeof(value_type) * size());	// will this be faster?
			else 
				allocate(a.size(), a._ptr, 1);
		return (*this);
	}

	unsigned int size() const	{	return _size;	}
	value_type operator[](unsigned int idx) const	{	return _ptr[idx];	}
	value_type& operator[](unsigned int idx)		{	return _ptr[idx];	}

	void resize(unsigned int _Newsize)	{	resize(_Newsize, _Ty());	}

	void resize(unsigned int _Newsize, const _Ty _Val)	{	allocate(_Newsize, &_Val, 0);	}

	// MACROS from <valarray>
#pragma push_macro("_VALOP")
#pragma push_macro("_VALGOP")
#undef _VALOP
#undef _VALGOP
#define _VALOP(TYPE, LENGTH, RHS)	/* assign RHS(_Idx) to new valarray */ \
	fast_array_base<TYPE> _Ans(LENGTH); \
	for (size_t _Idx = 0; _Idx < _Ans.size(); ++_Idx) \
	_Ans[_Idx] = RHS; \
	return _Ans

#define _VALGOP(RHS)	/* apply RHS(_Idx) to valarray */ \
	for (size_t _Idx = 0; _Idx < size(); ++_Idx) \
	_ptr[_Idx] RHS; \
	return *this

	// assign _Val to each element
	fast_array_base<_Ty>& operator=(const _Ty& _Val)	{	_VALGOP(= _Val);	}

	// subtract _Right from valarray elements
	fast_array_base<_Ty>& operator-=(const _Ty& _Right)
	{
		_VALGOP(-= _Right);
	}

	// add valarray _Right elements to valarray elements
	fast_array_base<_Ty>& operator+=(const fast_array_base<_Ty>& _Right)
	{	
		_VALGOP(+= _Right[_Idx]);
	}

#pragma pop_macro("_VALOP")
#pragma pop_macro("_VALGOP")

	_Ty sum() const	// return sum all elements
	{	
		_Ty _Sum = _ptr[0];
		for (unsigned int _Idx = 0; ++_Idx < size(); )
			_Sum += _ptr[_Idx];
		return (_Sum);
	}

// macro max and min is usually already defined in other places but redefined here
#pragma push_macro("min")
#undef min
	_Ty min() const	// return smallest of all elements
	{	
		_Ty _Min = _ptr[0];
		for (unsigned int _Idx = 0; ++_Idx < size(); )
			if (_ptr[_Idx] < _Min)
				_Min = _ptr[_Idx];
		return (_Min);
	}
#pragma pop_macro("min")

#pragma push_macro("max")
#undef max
	_Ty max() const	// return largest of all elements	
	{	
		_Ty _Max = _ptr[0];
		for (unsigned int _Idx = 0; ++_Idx < size(); )
			if (_Max < _ptr[_Idx])
				_Max = _ptr[_Idx];
		return (_Max);
	}
#pragma pop_macro("max")

protected:
	// resize to _new_size elements and fill values if _p_value != 0
	void allocate(unsigned int _new_size, const _Ty *_p_value = 0, unsigned int _Inc = 0)
	{	
		clear();
		if (_new_size == 0)	return;

		// allocate new array
		_size = _new_size;
		_padded_size = (_size-1) / _num_op * _num_op + _num_op;
		_ptr = (value_type*) _mm_malloc(_padded_size * sizeof(value_type), 16);
		assert("fast_array_malloc" && (0 !=_ptr) );
		if (0 == _ptr) throw 1;

		// pad new values
		if (_p_value != 0)
			for (unsigned int _idx = 0; _idx < _size; ++_idx, _p_value += _Inc) 
				_ptr[_idx] = *_p_value;
	}

	// clear the object, freeing any allocated storage
	void clear()
	{			
		if (_ptr != 0) 
		{
			_mm_free(_ptr);
			_ptr = 0;
			_size = 0;
			_padded_size = 0;
		}
	}

	// number of operands for each SIMD instruction, e.g., 8 for UINT16, 4 for float and 2 for double, determined by value_type in construction
	const unsigned int _num_op;	
	value_type* _ptr;		// array data
	unsigned int _size;		// true array size
	// actually allocated array size, it is no less than true size and multiples of _num_op
	// for convenient SIMD operations  _padded_size = (_size-1) / _num_op * _num_op + _num_op;
	unsigned int _padded_size;
};

#define _VALG_SIMDOP(RHS, OP, TYPE)	/* apply RHS(_Idx) to valarray */ \
	for (unsigned int _idx = 0; _idx < size(); _idx += _num_op)\
		*(TYPE*)(_ptr+_idx) = OP(*(TYPE*)(_ptr+_idx), *(TYPE*)(&RHS));\
		return *this;

#define _DEFINE_SIMD_ADD_SUB(fast_array_class, op_type, op_add, op_sub)	\
fast_array_class& operator+=(const fast_array_class& a){\
fast_array_class& right = const_cast<fast_array_class&>(a);\
_VALG_SIMDOP(right[_idx], op_add, op_type);}\
\
fast_array_class& operator-=(const fast_array_class& a){\
fast_array_class& right = const_cast<fast_array_class&>(a);\
_VALG_SIMDOP(right[_idx], op_sub, op_type);}

#define _DEFINE_SIMD_MUL_DIV(fast_array_class, op_type, op_mul, op_div)	\
fast_array_class& operator*=(const fast_array_class& a){\
fast_array_class& right = const_cast<fast_array_class&>(a);\
_VALG_SIMDOP(right[_idx], op_mul, op_type);}\
\
fast_array_class& operator/=(const fast_array_class& a){\
fast_array_class& right = const_cast<fast_array_class&>(a);\
_VALG_SIMDOP(right[_idx], op_div, op_type);}

class fast_array_float : public fast_array_base<float>
{
	typedef fast_array_base<float> BaseClass;
public:	
	explicit fast_array_float(unsigned int size = 0) : BaseClass(size){}
	fast_array_float(const fast_array_float& a) : BaseClass(static_cast<BaseClass>(a)){}
	fast_array_float(const value_type& _Val, unsigned int size) : BaseClass(_Val, size){}
	fast_array_float(const value_type *_Ptr, unsigned int size) : BaseClass(_Ptr, size){}

	_DEFINE_SIMD_ADD_SUB(fast_array_float, __m128, _mm_add_ps, _mm_sub_ps)
	_DEFINE_SIMD_MUL_DIV(fast_array_float, __m128, _mm_mul_ps, _mm_div_ps)

	value_type min_of_sum(const fast_array_float& a) const
	{		
		fast_array_float& right = const_cast<fast_array_float&>(a);
		value_type min_value =  _ptr[0] + right._ptr[0];

		__m128 sum;
		unsigned int _idx = 0;
		// don't process last unit
		for (; _idx+_num_op < size(); _idx += _num_op)	// shouldn't use _idx<size()-_num_op since RHS could be negative!
		{
			sum = _mm_add_ps(*(__m128*)(_ptr+_idx), *(__m128*)(right._ptr+_idx));
			if (sum.m128_f32[0] < min_value) min_value = sum.m128_f32[0];
			if (sum.m128_f32[1] < min_value) min_value = sum.m128_f32[1];
			if (sum.m128_f32[2] < min_value) min_value = sum.m128_f32[2];
			if (sum.m128_f32[3] < min_value) min_value = sum.m128_f32[3];
		}

		// process last unit
		sum = _mm_add_ps(*(__m128*)(_ptr+_idx), *(__m128*)(right._ptr+_idx));
		for(int offset = 0; _idx + offset < size(); offset++)
			if (sum.m128_f32[offset] < min_value) min_value = sum.m128_f32[offset];
		
		return min_value;
	}

	using BaseClass::operator =;
	using BaseClass::operator -=;
};

class fast_array_uint : public fast_array_base<unsigned int>
{
	typedef fast_array_base<unsigned int> BaseClass;
public:	
	explicit fast_array_uint(unsigned int size = 0) : BaseClass(size){}
	fast_array_uint(const fast_array_uint& a) : BaseClass(static_cast<BaseClass>(a)){}
	fast_array_uint(const value_type& _Val, unsigned int size) : BaseClass(_Val, size){}
	fast_array_uint(const value_type *_Ptr, unsigned int size) : BaseClass(_Ptr, size){}

	_DEFINE_SIMD_ADD_SUB(fast_array_uint, __m128i, _mm_add_epi32, _mm_sub_epi32)
};

class fast_array_ushort : public fast_array_base<unsigned short>
{
	typedef fast_array_base<unsigned short> BaseClass;
public:	
	explicit fast_array_ushort(unsigned int size = 0) : BaseClass(size){}
	fast_array_ushort(const fast_array_ushort& a) : BaseClass(static_cast<BaseClass>(a)){}
	fast_array_ushort(const value_type& _Val, unsigned int size) : BaseClass(_Val, size){}
	fast_array_ushort(const value_type *_Ptr, unsigned int size) : BaseClass(_Ptr, size){}

	_DEFINE_SIMD_ADD_SUB(fast_array_ushort, __m128i, _mm_add_epi16, _mm_sub_epi16)
};

// testing routines
void test_float_fast_array();
void test_int_fast_array();