/*!
\file   BALogixExample.c
\brief  This file contains the implementation for the BA Logix EdgeMax Library C-Interface Example
\par    Detailed Description:
This example demonstrates the BA Logix DLL's programmers API.  This is a simplistic example
and is only intended to demonstrate usage.  The programmer is responsible for allocating and
deallocating any memory passed into or returned by the BA Loigx DLL API.

The BA Logix DLL provides a single user callable routine called BAL_EdgeDetection.
The BAL_EdgeDetection routine takes 3 user parameters (input data,
bitplane mask, and enable/disable morphological operations)

input data                              - The image data to be processed stored in the BAL_sImage data
type. The BAL_sImage data type is a 1D buffer stored in
row-major format.

bitplane mask                           - The bitplane mask is a 32 bit mask (31: MSB to 0: LSB)
A value of 1 at a given bitplane location enables that
bitplane to be processed by the BA Logix's Edge detection
algorithm.  A value of 0 at a given bitplane location
disables that bitplane from being processed by the
BA Logix's Edge detection algorithm.

enable/disable morphological operations - A boolean flag that enables/disables morphological
thinning at the end of the BA Logix's Edge detection
process.  A value of true (1) enables morphological
operations.  A value of false (0) disables morphological
operations.

The BAL_EdgeDetection routine returns the resultant edge map stored in an BAL_sImage data structure
or returns NULL upon failure.

The BA Logix DLL is a licensed library.  As a result, the developer will need to
install the BA Logix DLL first.  During the installation process, a license service is installed
onto your computer system.  The installer will then prompt you to activate your license.
(see the "Using License Manager.doc" for more details)  Once the setup is complete, you will need
to include both the balogix.dll and the filechk.dll in your application folder.
\author BA Logix Inc.
\date   2009-03-13
*/

/*
* Copyright ?2009 BA Logix, Inc. All rights reserved.
* Use of this software is governed by the terms and conditions of the end user license agreement (EULA)
* that accompanies this software. EXCEPT AS WARRANTED IN THE LICENSE AGREEMENT, BA LOGIX HEREBY
* DISCLAIMS ALL WARRANTIES AND CONDITIONS WITH REGARD TO THE SOFTWARE, INCLUDING ALL IMPLIED
* WARRANTIES AND CONDITIONS OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, TITLE AND
* NON-INFRINGEMENT.
*/


//#include "BALogix.h" // Required to use the BA Logix DLL API
//#include <stdio.h>
//#include <stdlib.h>
//#include <malloc.h>

#include "mexFunction.h"
#include "engine.h"
#include <math.h>
/*!
\brief  This is the top-level routine for the BA Logix's DLL usage example
\return 0 if successful, non-zero otherwise
\author BA Logix Inc.
*/


void mexFunction(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[])
{
	BAL_sImage *pData, *pEdge, *pRecon;
	UINT32      bitplaneMask;
	BOOL        enableMorph;
	UINT32      height, width, bpp;
	BYTE        grayValue;
	UINT32      sqrDim;
	UINT32      startRow, endRow, startCol, endCol;
	UINT32      row, col;
	UINT32      nOnes;
	UINT32		nbitplanes;
	UINT32		p_code = 0;
	UINT32		n_code = 8;
	double		beta = 0.0;
	UINT32		winsize = 2;
	double		sigma = 1;
	UINT32		kernelsize = 3;
	UINT32		usegausian = 0;
	
	// start Matlab engine API connection 
	Engine *ep;
	ep = engOpen(NULL);
	engSetVisible(ep, 0);


	//retrieve the input image here 
	//BYTE *matrixIn = mxGetUint8s(prhs[0]);
	int m = mxGetM(prhs[0]);
	int n = mxGetN(prhs[0]);
	////retrieve the input settings for the bitplane mask and initialize the bitplane mask.....
															//UINT32 *nbrBitplane = (UINT32 *)prhs[1];
															//UINT32 *pBitplane = mxGetPr(nbrBitplane);
															//UINT32 nbitplanes = pBitplane[0];//(UINT32)pBitplane;

	nbitplanes = mxGetScalar(prhs[1]);
	if (nrhs == 10) {
		p_code = mxGetScalar(prhs[3]);
		n_code = mxGetScalar(prhs[4]);
		beta = mxGetScalar(prhs[5]);
		winsize = mxGetScalar(prhs[6]);
		sigma = mxGetScalar(prhs[7]);
		kernelsize = mxGetScalar(prhs[8]);
		usegausian = mxGetScalar(prhs[9]);

	}
	else {
		fprintf(stderr, "Invalid number of input arguments\n");
		exit(-1);
	}

	bitplaneMask = (UINT32)pow(2, n_code) - 1;
	UINT32 i;
	for (i = 0; i < n_code - nbitplanes; i++) {
		bitplaneMask -= (UINT32)pow(2, i);
	}
	char lutName[10];
	sprintf(lutName, "%u,%u", n_code, p_code);
	//const char* lutName = NULL;
	//mexPrintf("nbitplanes: %d\np_code: %d\nbitplaneMask: %d\nlutname: %s\nbeta: %f\nheight: %d\nwidth: %d\n", nbitplanes, p_code, bitplaneMask, lutName, beta, m, n);

	
	//now retrieve the user's setting for the decomposition method and initialize the type of decomposition method to be used 
	//for the by the edge detection algorithm.
																//UINT32 *decompoitionMethod= (UINT32 *)prhs[2];
																//UINT32 *pDecompositionMethod = mxGetPr(decompoitionMethod);
																//UINT32 valueDecompositionMethod = pDecompositionMethod[0];

	BAL_DECOMPOSITION_METHOD DECOMPOSITION_METHOD = 0;

	int valueDecompositionMethod;
	valueDecompositionMethod = mxGetScalar(prhs[2]);
	//mexPrintf("%d\n", valueDecompositionMethod);

	switch (valueDecompositionMethod)
	{ 
	case 0: 
		DECOMPOSITION_METHOD= BAL_BITPLANE_DECOMPOSITION;
		break;
	case 1:  
		DECOMPOSITION_METHOD = BAL_FIBONACCI_DECOMPOSITION;

		break;
	default: 
		DECOMPOSITION_METHOD = BAL_BITPLANE_DECOMPOSITION;
		break;
	}

	

	// Initialize the elecments 
	pData = pEdge = pRecon = NULL;
	
	enableMorph = true;
	height = (UINT32)m;       // the height of the image is the numbe of rows 
	width  = (UINT32)n;   // set the pitch of the image equal to the width and equal to the number column 
	bpp = 1;
	grayValue = 255;
	sqrDim = 50;
	startRow = startCol = 0;
	endRow = m; //startRow + sqrDim;
	endCol = n;   // startCol + sqrDim;
		
	//bitplaneMask = 0x0080;

	plhs[0] =  mxDuplicateArray(prhs[0]);
	
	// Allocate image structures
	pData = BAL_NewImage(height, width, bpp);
	if (pData == NULL) {
		fprintf(stderr, "Failed to allocate input image structure\n");
		exit(-1);
	}

	
	// fill in the matrix element of the BAL_struct of the input image created
	BYTE *inputImage;
	inputImage = mxGetUint8s(prhs[0]);
	// copy the imput image matrix into the scan0 element of the BAL_image structure
	/*mexPrintf("size: %u\n", sizeof(mxGetIr(matrixIn))/sizeof(size_t));*/
	for (row = startRow; row < endRow; row++) {
		for (col = startCol; col < endCol; col++) {
			pData->scan0[col *pData->height + row] = (BYTE)inputImage[col*pData->height + row] ; //grayValue;			
		} // end for col
	} // end for row

	
	//scanImage = mxDuplicateArray(matrixIn);	
		
	//Create the input BAL_sImage Structure and return it to matlab.............takes this off later....This is just to verify in matlab that we are converting the image successfully.
	mxArray *thheight, *thwidth, *thbpp, *thpitch, *thscan0;
	double *h, *w, *p, *bp, *sc;
	const char *fieldnames[5]; //This will hold field names.
//


	thheight = mxCreateDoubleMatrix(1, 1, mxREAL);
	thwidth = mxCreateDoubleMatrix(1, 1, mxREAL);
	thpitch = mxCreateDoubleMatrix(1, 1, mxREAL);
	thbpp = mxCreateDoubleMatrix(1, 1, mxREAL);
	thscan0 = mxCreateDoubleMatrix(height, width, mxREAL);
		
	h = mxGetPr(thheight); w = mxGetPr(thwidth);  p = mxGetPr(thpitch);  bp = mxGetPr(thbpp);  sc = mxGetPr(thscan0);
	h[0] =  (double )pData->height;  w[0] = (double)pData->width;  p[0]= (double)pData->width;  bp[0] = (double)pData->bpp;
		
	for (row = 0; row < pData->height; row++) {
		for (col = 0; col < pData->width; col++) {
			sc[col *pData->height + row] = (double)pData->scan0[col*pData->height + row] ;
		} // end for col
	} // end for row
		

//Assign field names
	fieldnames[0] = (char*)mxMalloc(20); fieldnames[2] = (char*)mxMalloc(20); fieldnames[4] = (char*)mxMalloc(20);
	fieldnames[1] = (char*)mxMalloc(20); fieldnames[3] = (char*)mxMalloc(20);
	memcpy(fieldnames[0], "height", sizeof("height"));
	memcpy(fieldnames[1], "width", sizeof("width"));
	memcpy(fieldnames[2], "pitch", sizeof("pitch"));
	memcpy(fieldnames[3], "bpp", sizeof("bpp"));
	memcpy(fieldnames[4], "scan0", sizeof("scan0"));
		
//Allocate memory for the structure
	plhs[1] = mxCreateStructMatrix(1, 1, 5, fieldnames);
	//////Deallocate memory for the fieldnames
	mxFree(fieldnames[0]);  	  mxFree(fieldnames[4]);
	mxFree(fieldnames[1]);
	mxFree(fieldnames[2]);		 
	mxFree(fieldnames[3]);
//Assign the field values
	mxSetFieldByNumber(plhs[1], 0, 0, thheight);
	mxSetFieldByNumber(plhs[1], 0, 1, thwidth);
	mxSetFieldByNumber(plhs[1], 0, 2, thpitch);
	mxSetFieldByNumber(plhs[1], 0, 3, thbpp);
	mxSetFieldByNumber(plhs[1], 0, 4, thscan0);
	//NOTE: mxSetFieldByNumber(..) will automatically take care
	//////      of allocating required memory for the fields.





	//BAL_EdgeDetection(BAL_sImage *pImg, UINT32 planeMask, BAL_DECOMPOSITION_METHOD decompositionMethod, BOOL enableMorph)
	char* msg = NULL;
	pEdge = BAL_EdgeDetection(pData, nbitplanes, DECOMPOSITION_METHOD, enableMorph, p_code, n_code, beta, winsize, sigma, kernelsize, usegausian, &pRecon, &msg);
	
	if (pEdge == NULL) {
		//mexPrintf(msg);
		fprintf(stderr, "Edge Detection failed!\n");
		exit(-1);
	}
	mexPrintf(msg);
	if (msg != NULL) {
		free(msg);
	}
	//////Pass the edge map BAL_sImage structure to the output variable
	mxArray *thheight2, *thwidth2, *thbpp2, *thpitch2, *thscan02;
	double *h2, *w2, *p2, *bp2, *sc2;
	const char *fieldnames2[5]; //This will hold field names.
							   //


	thheight2 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thwidth2 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thpitch2 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thbpp2 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thscan02 = mxCreateDoubleMatrix(height, width, mxREAL);

	h2 = mxGetPr(thheight2); w2 = mxGetPr(thwidth2);  p2 = mxGetPr(thpitch2);  bp2 = mxGetPr(thbpp2);  sc2 = mxGetPr(thscan02);
	h2[0] = (double)pEdge->height;  w2[0] = (double)pEdge->width;  p2[0] = (double)pEdge->width;  bp2[0] = (double)pEdge->bpp;

	for (row = 0; row < pEdge->height; row++) {
		for (col = 0; col < pEdge->width; col++) {
			sc2[col * pEdge->height + row] = (BYTE)pEdge->scan0[col * pEdge->height + row];
		} // end for col
	} // end for row


	  //Assign field names
	fieldnames2[0] = (char*)mxMalloc(20); fieldnames2[2] = (char*)mxMalloc(20); fieldnames2[4] = (char*)mxMalloc(20);
	fieldnames2[1] = (char*)mxMalloc(20); fieldnames2[3] = (char*)mxMalloc(20);
	memcpy(fieldnames2[0], "height2", sizeof("height2"));
	memcpy(fieldnames2[1], "width2", sizeof("width2"));
	memcpy(fieldnames2[2], "pitch2", sizeof("pitch2"));
	memcpy(fieldnames2[3], "bpp2", sizeof("bpp2"));
	memcpy(fieldnames2[4], "scan02", sizeof("scan02"));
	//Allocate memory for the structure
	plhs[2] = mxCreateStructMatrix(1, 1, 5, fieldnames2);
	//////Deallocate memory for the fieldnames
	mxFree(fieldnames2[0]);  	  mxFree(fieldnames2[4]);
	mxFree(fieldnames2[1]);
	mxFree(fieldnames2[2]);
	mxFree(fieldnames2[3]);
	//Assign the field values
	mxSetFieldByNumber(plhs[2], 0, 0, thheight2);
	mxSetFieldByNumber(plhs[2], 0, 1, thwidth2);
	mxSetFieldByNumber(plhs[2], 0, 2, thpitch2);
	mxSetFieldByNumber(plhs[2], 0, 3, thbpp2);
	mxSetFieldByNumber(plhs[2], 0, 4, thscan02);
	//NOTE: mxSetFieldByNumber(..) will automatically take care
	//////      of allocating required memory for the fields.




	mxArray* thheight3, * thwidth3, * thbpp3, * thpitch3, * thscan03;
	double* h3, * w3, * p3, * bp3, * sc3;
	const char* fieldnames3[5]; //This will hold field names.
							   //


	thheight3 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thwidth3 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thpitch3 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thbpp3 = mxCreateDoubleMatrix(1, 1, mxREAL);
	thscan03 = mxCreateDoubleMatrix(height, width, mxREAL);

	h3 = mxGetPr(thheight3); w3 = mxGetPr(thwidth3);  p3 = mxGetPr(thpitch3);  bp3 = mxGetPr(thbpp3);  sc3 = mxGetPr(thscan03);
	h3[0] = (double)pRecon->height;  w3[0] = (double)pRecon->width;  p3[0] = (double)pRecon->width;  bp3[0] = (double)pRecon->bpp;
	for (row = 0; row < pRecon->height; row++) {
		for (col = 0; col < pRecon->width; col++) {
			sc3[col * pRecon->height + row] = (BYTE)pRecon->scan0[col * pRecon->height + row];
		} // end for col
	} // end for row


	  //Assign field names
	fieldnames3[0] = (char*)mxMalloc(20); fieldnames3[2] = (char*)mxMalloc(20); fieldnames3[4] = (char*)mxMalloc(20);
	fieldnames3[1] = (char*)mxMalloc(20); fieldnames3[3] = (char*)mxMalloc(20);
	memcpy(fieldnames3[0], "height3", sizeof("height3"));
	memcpy(fieldnames3[1], "width3", sizeof("width3"));
	memcpy(fieldnames3[2], "pitch3", sizeof("pitch3"));
	memcpy(fieldnames3[3], "bpp3", sizeof("bpp3"));
	memcpy(fieldnames3[4], "scan03", sizeof("scan03"));
	//Allocate memory for the structure
	plhs[3] = mxCreateStructMatrix(1, 1, 5, fieldnames3);
	//////Deallocate memory for the fieldnames
	mxFree(fieldnames3[0]);  	  mxFree(fieldnames3[4]);
	mxFree(fieldnames3[1]);
	mxFree(fieldnames3[2]);
	mxFree(fieldnames3[3]);
	//Assign the field values
	mxSetFieldByNumber(plhs[3], 0, 0, thheight3);
	mxSetFieldByNumber(plhs[3], 0, 1, thwidth3);
	mxSetFieldByNumber(plhs[3], 0, 2, thpitch3);
	mxSetFieldByNumber(plhs[3], 0, 3, thbpp3);
	mxSetFieldByNumber(plhs[3], 0, 4, thscan03);


	int engClose(Engine *ep);
	// Cleanup
	//plhs[0] = pData;
	//plhs[1] = pEdge;
	BAL_DeleteImage(pData);
	BAL_DeleteImage(pEdge);
	//BAL_DeleteImage(pRecon);

}

