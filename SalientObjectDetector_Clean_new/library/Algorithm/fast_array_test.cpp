#include <stdlib.h>
#include <iostream>
#include <valarray>
using namespace std;

#include <time_ex.h>

#include "fast_array.h"

void test_float_fast_array()
{
	time_t timer;
	srand(time(&timer));

	const int size = 10000000 + rand() % 100;
	
	//const int size = 100 + rand() % 10;

	valarray<float> a(size), b(size);
	for(int n = 0; n < size; n++)
	{
		a[n] = (float)(rand()+0.001f) / (rand()+0.001f);
		b[n] = (float)(rand()+0.001f) / (rand()+0.001f);
	}
	fast_array_float fa(&a[0], a.size()), fb(&b[0], b.size());

	printf("test float fast array for %d float\n", size);

	printf("min_of_sum, ");
	tic(); float s0 = (a+b).min(); printf(" normal : %s", toc("").c_str());
	tic(); float s1 = fa.min_of_sum(fb); printf(" fast : %s\n", toc("").c_str());
	if (s0 != s1) printf("wrong result, %f != %f, with diff %f\n", s0, s1, s0-s1);

	return;

	// performance test
	printf("+=, ");
	tic();	a += b;		printf(" normal : %s", toc("").c_str());
	tic();	fa += fb;	printf(", fast : %s\n", toc("").c_str());

	// correctness test
	for(int n = 0; n < size; n++)
		if (a[n] != fa[n]) printf("wrong result at idx %d, %f != %f\n", n, a[n], fa[n]);

	// performance test
	printf("-=, ");
	tic();	a -= b;		printf(" normal : %s", toc("").c_str());
	tic();	fa -= fb;	printf(", fast : %s\n", toc("").c_str());

	// correctness test
	for(int n = 0; n < size; n++)
		if (a[n] != fa[n]) printf("wrong result at idx %d, %f != %f\n", n, a[n], fa[n]);

	// performance test
	printf("*=, ");
	tic();	a *= b;		printf(" normal : %s", toc("").c_str());
	tic();	fa *= fb;	printf(", fast : %s\n", toc("").c_str());

	// correctness test
	for(int n = 0; n < size; n++)
		if (a[n] != fa[n]) printf("wrong result at idx %d, %f != %f\n", n, a[n], fa[n]);

	// performance test
	printf("/=, ");
	tic();	a /= b;		printf(" normal : %s", toc("").c_str());
	tic();	fa /= fb;	printf(", fast : %s\n", toc("").c_str());

	// correctness test
	for(int n = 0; n < size; n++)
		if (a[n] != fa[n]) printf("wrong result at idx %d, %f != %f\n", n, a[n], fa[n]);
}