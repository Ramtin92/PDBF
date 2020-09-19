#pragma once


#include "BALogix.h" // Required to use the BA Logix DLL API
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <matrix.h>
#include <mex.h>


#define MEX_FUNCTION_EXPORTS
#ifdef MEX_FUNCTION_EXPORTS
#define MEX_FUNCTION_API_declspec(dllexport)
#else 
#define MEX_FUNCTION_API_declspec(dllimport)
#endif MEX_FUNCTION_API void mexFunction(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);