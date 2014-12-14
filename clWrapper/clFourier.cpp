#include "stdafx.h"

#include "clFourier.h"
#include <math.h>


clFourier::~clFourier(void)
{
	clfftDestroyPlan(&fftplan);
}

void clFourier::Setup(int width, int height)
{
	// Perform setup for FFT's

	clfftInitSetupData(&fftSetupData);
	fftStatus = clfftSetup(&fftSetupData);

	//	Local Data
	size_t buffSizeBytesIn = 0;
	size_t buffSizeBytesOut = 0;
	size_t fftVectorSize= 0;
	size_t fftVectorSizePadded = 0;
	size_t fftBatchSize = 0;
	cl_uint nBuffersOut = 0;
	cl_uint profileCount = 0;

	clfftDim fftdim = CLFFT_2D;
	clfftResultLocation	place = CLFFT_OUTOFPLACE;
	clfftLayout inLayout  = CLFFT_COMPLEX_INTERLEAVED;
	clfftLayout outLayout = CLFFT_COMPLEX_INTERLEAVED;

	size_t clLengths[ 3 ];
	size_t clPadding[ 3 ] = {0, 0, 0 };  // *** TODO
	size_t clStrides[ 4 ];
	size_t batchSize = 1;


	clLengths[0]=width;
	clLengths[1]=height;
	clLengths[2]=1;

	clStrides[ 0 ] = 1;
	clStrides[ 1 ] = clStrides[ 0 ] * (clLengths[ 0 ] + clPadding[ 0 ]);
	clStrides[ 2 ] = clStrides[ 1 ] * (clLengths[ 1 ] + clPadding[ 1 ]);
	clStrides[ 3 ] = clStrides[ 2 ] * (clLengths[ 2 ] + clPadding[ 2 ]);

	fftVectorSize	= clLengths[ 0 ] * clLengths[ 1 ] * clLengths[ 2 ];
	fftVectorSizePadded = clStrides[ 3 ];
	fftBatchSize	= fftVectorSizePadded * batchSize;


	fftStatus = clfftCreateDefaultPlan( &fftplan, Context->GetContext(), fftdim, clLengths );

	//	Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	fftStatus = clfftSetResultLocation( fftplan, place );
	fftStatus = clfftSetPlanPrecision(fftplan,CLFFT_SINGLE);
	fftStatus = clfftSetLayout( fftplan, inLayout, outLayout );
	fftStatus = clfftSetPlanBatchSize( fftplan, batchSize );
	fftStatus = clfftSetPlanScale (fftplan, CLFFT_FORWARD, 1/sqrtf(width * height));
	fftStatus = clfftSetPlanScale (fftplan, CLFFT_BACKWARD, 1/sqrtf(width * height));

	// Not using padding here yet
	if ((clPadding[ 0 ] | clPadding[ 1 ] | clPadding[ 2 ]) != 0) {
		clfftSetPlanInStride  ( fftplan, fftdim, clStrides );
		clfftSetPlanOutStride ( fftplan, fftdim, clStrides );
		clfftSetPlanDistance  ( fftplan, clStrides[ fftdim ], clStrides[ fftdim ]);
	}

	fftStatus = clfftBakePlan( fftplan, 1, &Context->GetQueue(), NULL, NULL );
	
	//get the buffersize
	
	fftStatus = clfftGetTmpBufSize(fftplan, &buffersize );
		
	if (buffersize)
	{
		// because buffersize should be in bytes already...
		clMedBuffer = Context->CreateBuffer<char,Manual>(buffersize);
		//clCreateBuffer ( *context, CL_MEM_READ_WRITE, buffersize, 0, &medstatus);
	}
}

clFourier::clFourier(clContext &Context, int _width, int _height): Context(&Context), width(_width), height(_height)
{ 
	Setup(_width,_height);
	AutoTeardownFFT::GetInstance();
	
};