#include "pch.h"
#include "CppUnitTest.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "CoinMP.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestMSVS
{

	int SOLVCALL MsgLogCallback(const char* messageStr, void *userParam)
	{
		fprintf(stdout, "*** MsgCB: %s\n", messageStr);
		return 0;
	}


	int SOLVCALL IterCallback(int     iterCount, 
							  double  objectValue,
							  int     isFeasible, 
							  double  infeasValue,
							  void   *userParam)
	{
		fprintf(stdout, "*** IterCB: iter=%d, obj=%.12g, feas=%d, infeas=%.12g\n",
						iterCount, objectValue, isFeasible, infeasValue);
		return 0;
	}


	int SOLVCALL MipNodeCallback(int     iterCount, 
								 int	 nodeCount,
								 double	 bestBound,
								 double  bestInteger,
								 int     isImproved,
								 void   *userParam)
	{
		fprintf(stdout, "*** NodeCB: iter=%d, node=%d, bound=%.12g, best=%.12g %s\n",
						iterCount, nodeCount, bestBound, bestInteger, isImproved ? "*" : "");
		return 0;
	}


	void GetAndCheckSolution(double optimalValue, HPROB hProb)
	{
		int solutionStatus;
		const char* solutionText;
		double objectValue;
		int i;
		int colCount;
		double* xValues;
		const char* ColName;
		const char* problemName;

		problemName = CoinGetProblemName(hProb);
		solutionStatus = CoinGetSolutionStatus(hProb);
		solutionText = CoinGetSolutionText(hProb);
		objectValue =  CoinGetObjectValue(hProb);
      
		fprintf(stdout, "\n---------------------------------------\n");
		fprintf(stdout, "Problem Name:    %s\n", problemName);
		fprintf(stdout, "Solution Result: %s\n", solutionText );
		fprintf(stdout, "Solution Status: %d\n", solutionStatus);
		fprintf(stdout, "Optimal Value:   %.12g\n", objectValue);
		fprintf(stdout, "---------------------------------------\n");

		colCount = CoinGetColCount(hProb);
		xValues = (double* )malloc(colCount * sizeof(double));
		CoinGetSolutionValues(hProb, xValues, NULL, NULL, NULL);
		for (i = 0; i < colCount; i++) {
			if (xValues[i] != 0.0) {
				ColName = CoinGetColName(hProb, i);
				fprintf(stdout, "%s = %.12g\n", ColName, xValues[i]);
			}
		}
		fprintf(stdout, "---------------------------------------\n\n");
		Assert::AreEqual(solutionStatus,0);
		Assert::AreEqual(solutionText, "Optimal solution found");
		if (optimalValue != 0.0) {
			Assert::IsTrue( fabs(objectValue-optimalValue) < 0.001 );
		}
	}



	void RunTestProblem(const char* problemName, double optimalValue, int colCount, int rowCount,
		  int nonZeroCount, int rangeCount, int objectSense, double objectConst, double* objectCoeffs, 
		  double* lowerBounds, double* upperBounds, const char* rowType, double* rhsValues, double* rangeValues, 
		  int* matrixBegin, int* matrixCount, int* matrixIndex, double* matrixValues, char** colNames,
		  char** rowNames, const char* objectName, double* initValues, const char* columnType)
	{
		HPROB hProb;
		int result;
		char filename[260];
		const char *userParam = "ProblemData";
    
		fprintf(stdout, "Solve Problem: %s (obj=%.12g)\n", problemName, optimalValue);
		hProb = CoinCreateProblem(problemName);  
		result = CoinLoadMatrix(hProb, colCount, rowCount, nonZeroCount, rangeCount,
						objectSense, objectConst, objectCoeffs, lowerBounds, upperBounds, 
						rowType, rhsValues, rangeValues, matrixBegin, matrixCount, 
						matrixIndex, matrixValues);
		result = CoinLoadNames(hProb, colNames, rowNames, objectName);
		if (columnType) {
			result = CoinLoadInteger(hProb, columnType);
		}
		result = CoinCheckProblem(hProb);
		if (result != SOLV_CALL_SUCCESS) {
			fprintf(stdout, "Check Problem failed (result = %d)\n", result);
		}
		result = CoinRegisterMsgLogCallback(hProb, &MsgLogCallback, (void*)userParam);
		if (columnType == NULL)
			result = CoinRegisterLPIterCallback(hProb, &IterCallback, (void*)userParam);
		else {
			result = CoinRegisterMipNodeCallback(hProb, &MipNodeCallback, (void*)userParam);
		}
		result = CoinOptimizeProblem(hProb, 0);
		strcpy_s(filename, problemName);
		strcat_s(filename, ".mps");
		result = CoinWriteFile(hProb, SOLV_FILE_MPS, filename);
		GetAndCheckSolution(optimalValue, hProb);
		CoinUnloadProblem(hProb);
	}


	void RunTestProblemBuf(const char* problemName, double optimalValue, int colCount, int rowCount, 
		  int nonZeroCount, int rangeCount, int objectSense, double objectConst, double* objectCoeffs, 
		  double* lowerBounds, double* upperBounds, const char* rowType, double* rhsValues, double* rangeValues, 
		  int* matrixBegin, int* matrixCount, int* matrixIndex, double* matrixValues, const char* colNamesBuf, 
		  const char* rowNamesBuf, const char* objectName, double* initValues, const char* columnType)
	{
		HPROB hProb;
		int result;
		char filename[260];
		const char *userParam = "ProblemData";
    
		fprintf(stdout, "Solve Problem: %s (obj=%.12g)\n", problemName, optimalValue);
		hProb = CoinCreateProblem(problemName);  
		result = CoinLoadMatrix(hProb, colCount, rowCount, nonZeroCount, rangeCount,
						objectSense, objectConst, objectCoeffs, lowerBounds, upperBounds, 
						rowType, rhsValues, rangeValues, matrixBegin, matrixCount, 
						matrixIndex, matrixValues);
		result = CoinLoadNamesBuf(hProb, colNamesBuf, rowNamesBuf, objectName);
		if (columnType) {
			result = CoinLoadInteger(hProb, columnType);
		}
		result = CoinCheckProblem(hProb);
		if (result != SOLV_CALL_SUCCESS) {
			fprintf(stdout, "Check Problem failed (result = %d)\n", result);
		}
		result = CoinRegisterMsgLogCallback(hProb, &MsgLogCallback, (void*)userParam);
		if (columnType == NULL)
			result = CoinRegisterLPIterCallback(hProb, &IterCallback, (void*)userParam);
		else {
			result = CoinRegisterMipNodeCallback(hProb, &MipNodeCallback, (void*)userParam);
		}
		result = CoinOptimizeProblem(hProb, 0);
		strcpy_s(filename, problemName);
		strcat_s(filename, ".mps");
		result = CoinWriteFile(hProb, SOLV_FILE_MPS, filename);
		GetAndCheckSolution(optimalValue, hProb);
		CoinUnloadProblem(hProb);
	}


	void RunTestProblemMip(const char* problemName, double optimalValue, int colCount, int rowCount, 
		  int nonZeroCount, int rangeCount, int objectSense, double objectConst, double* objectCoeffs, 
		  double* lowerBounds, double* upperBounds, const char* rowType, double* rhsValues, double* rangeValues, 
		  int* matrixBegin, int* matrixCount, int* matrixIndex, double* matrixValues, char** colNames, 
		  char** rowNames, const char* objectName, double* initValues, const char* columnType, int sosCount, 
		  int sosNZCount, int* sosType, int* sosPrior, int* sosBegin, int* sosIndex, double* sosRef,
		  int semiCount, int* semiIndex, int priorCount, int* priorIndex, int* priorValues, int* priorBranch)
	{
		HPROB hProb;
		int result;
		char filename[260];
   		const char *userParam = "ProblemData";
 
		fprintf(stdout, "Solve Problem: %s (obj=%.12g)\n", problemName, optimalValue);
		hProb = CoinCreateProblem(problemName);
		result = CoinLoadMatrix(hProb, colCount, rowCount, nonZeroCount, rangeCount,
						objectSense, objectConst, objectCoeffs, lowerBounds, upperBounds, 
						rowType, rhsValues, rangeValues, matrixBegin, matrixCount, 
						matrixIndex, matrixValues);
		result = CoinLoadNames(hProb, colNames, rowNames, objectName);
		if (columnType) {
			result = CoinLoadInteger(hProb, columnType);
		}
		if (priorCount > 0) {
			result = CoinLoadPriority(hProb, priorCount, priorIndex, priorValues, priorBranch);
		}
		if (sosCount > 0) {
			result = CoinLoadSos(hProb, sosCount, sosNZCount, sosType, sosPrior, sosBegin, sosIndex, sosRef);
		}
		if (semiCount > 0) {
			result = CoinLoadSemiCont(hProb, semiCount, semiIndex);
		}
		result = CoinCheckProblem(hProb);
		if (result != SOLV_CALL_SUCCESS) {
			fprintf(stdout, "Check Problem failed (result = %d)\n", result);
		}
		result = CoinRegisterMsgLogCallback(hProb, &MsgLogCallback, (void*)userParam);
		if ((columnType == NULL) && (sosCount == 0) && (semiCount == 0))
			result = CoinRegisterLPIterCallback(hProb, &IterCallback, (void*)userParam);
		else {
			result = CoinRegisterMipNodeCallback(hProb, &MipNodeCallback, (void*)userParam);
		}
		strcpy_s(filename, problemName);
		strcat_s(filename, ".mps");
		result = CoinWriteFile(hProb, SOLV_FILE_MPS, filename);
		result = CoinOptimizeProblem(hProb, 0);
		GetAndCheckSolution(optimalValue, hProb);
		CoinUnloadProblem(hProb);
	}


	void SaveTestProblemToFile(const char* problemName, double optimalValue, int colCount, int rowCount,
		  int nonZeroCount, int rangeCount, int objectSense, double objectConst, double* objectCoeffs, 
		  double* lowerBounds, double* upperBounds, const char* rowType, double* rhsValues, double* rangeValues, 
		  int* matrixBegin, int* matrixCount, int* matrixIndex, double* matrixValues, char** colNames,
		  char** rowNames, const char* objectName, double* initValues, const char* columnType)
	{
		HPROB hProb;
		int result;
		char filename[260];
		const char *userParam = "ProblemData";
    
		fprintf(stdout, "Save Problem: %s (obj=%.12g)\n", problemName, optimalValue);
		hProb = CoinCreateProblem(problemName);  
		result = CoinLoadMatrix(hProb, colCount, rowCount, nonZeroCount, rangeCount,
						objectSense, objectConst, objectCoeffs, lowerBounds, upperBounds, 
						rowType, rhsValues, rangeValues, matrixBegin, matrixCount, 
						matrixIndex, matrixValues);
		result = CoinLoadNames(hProb, colNames, rowNames, objectName);
		if (columnType) {
			result = CoinLoadInteger(hProb, columnType);
		}
		result = CoinCheckProblem(hProb);
		if (result != SOLV_CALL_SUCCESS) {
			fprintf(stdout, "Check Problem failed (result = %d)\n", result);
		}
		result = CoinRegisterMsgLogCallback(hProb, &MsgLogCallback, (void*)userParam);
		if (columnType == NULL)
			result = CoinRegisterLPIterCallback(hProb, &IterCallback, (void*)userParam);
		else {
			result = CoinRegisterMipNodeCallback(hProb, &MipNodeCallback, (void*)userParam);
		}
		strcpy_s(filename, problemName);
		strcat_s(filename, ".mps");
		result = CoinWriteFile(hProb, SOLV_FILE_MPS, filename);
		Assert::AreEqual(result, 0);
		CoinUnloadProblem(hProb);
	}

	TEST_CLASS(UnitTestMSVS)
	{
	public:
		
		TEST_METHOD(TestProblemBakery)
		{
			const char* problemName = "Bakery";
			int colCount = 2;
			int rowCount = 3;
			int nonZeroCount = 4 ;
			int rangeCount = 0;

			const char* objectName = "Profit";
			int objectSense = SOLV_OBJSENS_MAX;
			double objectConst = - 4000.0 / 30.0;
			double objectCoeffs[2] = { 0.05 , 0.08 };

			double lowerBounds[2] = { 0, 0 };
			double upperBounds[2] = { 1000000, 1000000 };

			char rowType[3] = { 'L', 'L' , 'L' };
			double rhsValues[3] = { 1400 , 8000 , 5000 };

			int matrixBegin[2+1] = { 0 , 2, 4 };
			int matrixCount[2] = { 2 , 2 };
			int matrixIndex[4] = { 0, 1, 0, 2};
			double matrixValues[4] = { 0.1, 1, 0.2, 1};

			const char* colNames[2] = {"Sun", "Moon"};
			const char* rowNames[3] = {"c1", "c2", "c3"};

			double optimalValue = 506.66666667;

			RunTestProblem(problemName, optimalValue, colCount, rowCount,
			  nonZeroCount, rangeCount, objectSense, objectConst, objectCoeffs, 
			  lowerBounds, upperBounds, rowType, rhsValues, NULL, 
			  matrixBegin, matrixCount, matrixIndex, matrixValues, 
			  const_cast<char**>(colNames), const_cast<char**>(rowNames), objectName, NULL, NULL);
		}

		TEST_METHOD(WriteBakeryToLp)
		{
			const char* problemName = "Bakery";
			int colCount = 2;
			int rowCount = 3;
			int nonZeroCount = 4 ;
			int rangeCount = 0;

			const char* objectName = "Profit";
			int objectSense = SOLV_OBJSENS_MAX;
			double objectConst = - 4000.0 / 30.0;
			double objectCoeffs[2] = { 0.05 , 0.08 };

			double lowerBounds[2] = { 0, 0 };
			double upperBounds[2] = { 1000000, 1000000 };

			char rowType[3] = { 'L', 'L' , 'L' };
			double rhsValues[3] = { 1400 , 8000 , 5000 };

			int matrixBegin[2+1] = { 0 , 2, 4 };
			int matrixCount[2] = { 2 , 2 };
			int matrixIndex[4] = { 0, 1, 0, 2};
			double matrixValues[4] = { 0.1, 1, 0.2, 1};

			const char* colNames[2] = {"Sun", "Moon"};
			const char* rowNames[3] = {"c1", "c2", "c3"};

			double optimalValue = 506.66666667;

			SaveTestProblemToFile(problemName, optimalValue, colCount, rowCount,
			  nonZeroCount, rangeCount, objectSense, objectConst, objectCoeffs, 
			  lowerBounds, upperBounds, rowType, rhsValues, NULL, 
			  matrixBegin, matrixCount, matrixIndex, matrixValues, 
			  const_cast<char**>(colNames), const_cast<char**>(rowNames), objectName, NULL, NULL);
		}
	};
}
