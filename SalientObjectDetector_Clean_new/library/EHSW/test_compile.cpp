#include "HistUnaryFunction.h"
#include "HistUnarySimpleFunction.h"
#include "BinUnaryOp2ArrayFunction.h"
#include "HistUnaryOp2ArrayFunction.h"
#include "HistUnarySimilarityFunction.h"

#include "HistBinaryFunction.h"
#include "HistBinarySimilarityFunction.h"

#include <vector>
#include <valarray>
using namespace std;

void test_ehsw_compile()
{
	valarray<float> op2_array;
	int hist_length, total_count, total_count2;

	BinAdditiveFunction<BinFunctionBase<unsigned int>> bin_additive(0, BinFunctionBase<unsigned int>());
	
	HistUnaryNullFunction<float> null(hist_length);
 	HistL0NormFunc l0norm(hist_length);
 
 	HistEntropyFunc<float> entropy(hist_length, total_count);
 	HistFastEntropyFunc entropy_fast(hist_length, total_count);

	BinUnaryOp2ArrayFunction<binary_null<float>> bin_null(op2_array);
 	BinUnaryOp2ArrayNorm1Function<binary_max_of_diff_and_zero<unsigned int>> bin_max_diff_zero(op2_array, 1);

 	HistUnaryUniquenessFunc<float> uniqueness(op2_array, total_count);

	BinUnaryOp2ArrayNorm1Function<binary_min<float>> bin_min(op2_array, 1);
	
 	HistIntersectionFunc intersection(op2_array, total_count);
 	HistIntersectionResidualFunc intersection_residual(op2_array, total_count, total_count2);
	HistBhattachSimilarityFunc bhattach(op2_array, total_count);

	HistCenterSurrIntersectionFunc binary_interection(hist_length, total_count, total_count2);
}