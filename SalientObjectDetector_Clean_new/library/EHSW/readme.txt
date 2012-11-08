structure of histogram based functions

BinAdditiveFunction
    HistUnaryFunction (HistUnaryNullFunction)
        HistUnarySimpleFunction (HistL0NormFunc, HistEntropyFunc, HistFastEntropyFunc)
        HistUnaryOp2ArrayFunction (HistLinearSVMFunc, HistDotProductFastFunc, HistBhattachSimilarityFastFunc, HistUnaryUniquenessFunction)
        HistUnarySimilarityFunction (HistIntersectionFunc, HistBhattachSimilarityFunc, HistDotProductFunc, HistL1NormSimilarityFunc, HistL2NormSimilarityFunc, HistChiSquareSimilarityFunc)
        HistUnaryResidualSimilarityFunc (HistIntersectionResidualFunc)
        HistSVMFunction
    HistBinaryFunction (HistBinaryNullFunction)
        HistBinarySimilarityFunction (HistCenterSurrIntersectionFunc, HistBinaryIntersectionFunc)