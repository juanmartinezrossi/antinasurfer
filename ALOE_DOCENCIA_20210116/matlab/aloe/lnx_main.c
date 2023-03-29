/* $Revision: 1.10 $
 * Author: Dan Bhanderi, 2005
 *
 *
 * File:  lnx_main.c
 *
 * Abstract:
 *      Linux Soft "Real-Time (single tasking or pseudo-multitasking,
 *      statically allocated data)". *
 *
 * Compiler specified defines:
 *	RT              - Required.
 *      MODEL=modelname - Required.
 *	NUMST=#         - Required. Number of sample times.
 *	NCSTATES=#      - Required. Number of continuous states.
 *      TID01EQ=1 or 0  - Optional. Only define to 1 if sample time task
 *                        id's 0 and 1 have equal rates.
 *      MULTITASKING    - Optional. (use MT for a synonym).
 *	SAVEFILE        - Optional (non-quoted) name of .mat file to create.
 *			  Default is <MODEL>.mat
 *      BORLAND         - Required if using Borland C/C++
 */

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "capi_BIOSignalLog.h"
#include "phal_sw_api.h"

#include "rtwtypes.h"
#include "rtmodel.h"
#include "rt_sim.h"
#include "rt_logging.h"
#ifdef UseMMIDataLogging
#include "rt_logging_mmi.h"
#endif
#include "rt_nonfinite.h"

/* Signal Handler header */
#ifdef BORLAND
/* #include <signal.h> Already included in Soft Real-Time below */
#include <float.h>
#endif

#include "ext_work.h"

/* LNX includes */
#include <sys/mman.h>
#include <sys/time.h>
#include <sched.h>
#include <signal.h>
#include <unistd.h>

/*=========*
 * Defines *
 *=========*/

#ifndef TRUE
#define FALSE (0)
#define TRUE  (1)
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif

#define QUOTE1(name) #name
#define QUOTE(name) QUOTE1(name)    /* need to expand name    */

#ifndef RT
# error "must define RT"
#endif

#ifndef MODEL
# error "must define MODEL"
#endif

#ifndef NUMST
# error "must define number of sample times, NUMST"
#endif

#ifndef NCSTATES
# error "must define NCSTATES"
#endif

#ifndef SAVEFILE
# define MATFILE2(file) #file ".mat"
# define MATFILE1(file) MATFILE2(file)
# define MATFILE MATFILE1(MODEL)
#else
# define MATFILE QUOTE(SAVEFILE)
#endif

#define RUN_FOREVER -1.0

#define EXPAND_CONCAT(name1,name2) name1 ## name2
#define CONCAT(name1,name2) EXPAND_CONCAT(name1,name2)
#define RT_MODEL            CONCAT(MODEL,_rtModel)

/*====================*
 * External functions *
 *====================*/
extern RT_MODEL *MODEL(void);

extern void MdlInitializeSizes(void);
extern void MdlInitializeSampleTimes(void);
extern void MdlStart(void);
extern void MdlOutputs(int_T tid);
extern void MdlUpdate(int_T tid);
extern void MdlTerminate(void);

#if NCSTATES > 0
  extern void rt_ODECreateIntegrationData(RTWSolverInfo *si);
  extern void rt_ODEUpdateContinuousStates(RTWSolverInfo *si);

# define rt_CreateIntegrationData(S) \
    rt_ODECreateIntegrationData(rtmGetRTWSolverInfo(S));
# define rt_UpdateContinuousStates(S) \
    rt_ODEUpdateContinuousStates(rtmGetRTWSolverInfo(S));
# else
# define rt_CreateIntegrationData(S)  \
      rtsiSetSolverName(rtmGetRTWSolverInfo(S),"FixedStepDiscrete");
# define rt_UpdateContinuousStates(S) \
      rtmSetT(S, rtsiGetSolverStopTime(rtmGetRTWSolverInfo(S)));
#endif


/*==================================*
 * Global data local to this module *
 *==================================*/

static struct {
  int_T    stopExecutionFlag;
  int_T    isrOverrun;
  int_T    overrunFlags[NUMST];
  const char_T *errmsg;
} GBLbuf;




#ifdef EXT_MODE
#  define rtExtModeSingleTaskUpload(S)                          \
   {                                                            \
        int stIdx;                                              \
        rtExtModeUploadCheckTrigger(rtmGetNumSampleTimes(S));   \
        for (stIdx=0; stIdx<NUMST; stIdx++) {                   \
            if (rtmIsSampleHit(S, stIdx, 0 /*unused*/)) {       \
                rtExtModeUpload(stIdx,rtmGetTaskTime(S,stIdx)); \
            }                                                   \
        }                                                       \
   }
#else
#  define rtExtModeSingleTaskUpload(S) /* Do nothing */
#endif

/*=================*
 * Local functions *
 *=================*/

char sigNameRe[100],sigNameIm[100];

SignalBufferInfo *sbInfo;

void capi_CreateSignalBuffer(SignalBuffer   *sigBuf,
                             char_T const   *varName,
                             uint8_T         slDataType, 
                             uint_T         *dims, 
                             uint16_T        dataSize,
                             boolean_T       isComplex,
                             int_T           maxSize,
                             real_T          fSlope,
                             real_T          fBias,
                             int8_T          fExp)
{
    /* Set Signal Name */
    sigBuf->sigName[80] ='\0';
    strncpy(sigBuf->sigName,varName,80);
    sprintf(sigNameRe,"%s_re",varName);
    sprintf(sigNameIm,"%s_im",varName);

    /* Set Signal Data Type */
    sigBuf->slDataType = slDataType;
    /* Allocate memory for the real part */
    sigBuf->re = (void *) calloc(dims[0]*dims[1]*maxSize, dataSize);

    /* Allocate memory for imaginary part */
    if(isComplex)
        sigBuf->im = (void *) calloc(dims[0]*dims[1]*maxSize, dataSize);

    /* Signals slope and Bias */
    sigBuf->slope = (fSlope * pow(2, fExp));
    sigBuf->bias  = fBias;
    sigBuf->buffersize = dataSize*dims[0]*dims[1]*maxSize;
    sigBuf->numelems = maxSize;
    sigBuf->curelem = 0;
    
    if (!strncmp(varName,"in_",3)) {
	sigBuf->direction = INPUT;
	sigBuf->fd_re = CreateItf(sigNameRe, FLOW_READ_ONLY); 
	if (sigBuf->fd_re<0) {
		printf("Error creating input flow %s\n",sigNameRe);
		ClosePHAL();
	}
	if (isComplex) {
		sigBuf->fd_im = CreateItf(sigNameIm, FLOW_READ_ONLY); 
		if (sigBuf->fd_im<0) {
			printf("Error creating input flow %s\n",sigNameIm);
			ClosePHAL();
		}
	}
    } else {	
	sigBuf->direction = OUTPUT;
	sigBuf->fd_re = CreateItf(sigNameRe, FLOW_WRITE_ONLY); 
	printf("created output itf %s %d\n",sigNameRe,sigBuf->fd_re);
	if (sigBuf->fd_re<0) {
		printf("Error creatin output flow %s\n",sigNameRe);
		ClosePHAL();
	}
	if (isComplex) {
		sigBuf->fd_im = CreateItf(sigNameIm, FLOW_WRITE_ONLY); 
		if (sigBuf->fd_im<0) {
			printf("Error creatin input flow %s\n",sigNameIm);
			ClosePHAL();
		}
	}
    }

}

void capi_StartBlockIOLogging(rtwCAPI_ModelMappingInfo*  mmi, 
                              int_T                      maxSize)
{
    /* Get the relevant signal information from rtwCAPI_ModelMappingInfo */
    /* Get Maps */
    rtwCAPI_DataTypeMap  const *dTypeMap = rtwCAPI_GetDataTypeMap(mmi);  
    rtwCAPI_DimensionMap const *dimMap   = rtwCAPI_GetDimensionMap(mmi);  
    void *                  *dataAddrMap = rtwCAPI_GetDataAddressMap(mmi); 
    uint_T               const *dimArray = rtwCAPI_GetDimensionArray(mmi);
    rtwCAPI_ElementMap   const *elemMap  = rtwCAPI_GetElementMap(mmi); 
    rtwCAPI_FixPtMap     const *fxpMap   = rtwCAPI_GetFixPtMap(mmi);
    
    /* Get number of BlockIOSignals in the map */
    uint_T numBIOSig = rtwCAPI_GetNumSignals(mmi);
 
    /* Local variables */
    uint_T sigIdx;

    /* Get the Block Signal structure from the modelmap */
    rtwCAPI_Signals const *bioSig =  rtwCAPI_GetSignals(mmi);

    /* Assign memory to the global variable sbInfo */
    sbInfo = (SignalBufferInfo *) malloc(sizeof(SignalBufferInfo));

    /* Allocate memory for logging time */
    sbInfo->time = (time_T *) calloc(maxSize, sizeof(time_T));

    /* Initialize the number of signals Buffers to 0 */
    sbInfo->numSigBufs = 0;
    
    /* In this case, hard code max number of signals that can be logged = 10 */
    /* Assign memory for those 10 signals */
    sbInfo->sigBufs = (SignalBuffer *) calloc(10, sizeof(SignalBuffer));

    /* Create Signal Buffer structures to log the signals */
    for(sigIdx = 0; sigIdx < numBIOSig; sigIdx++) {

        /* Get Signal name from the BlockIOSignal structure */
        char_T const *sigName   =  rtwCAPI_GetSignalName(bioSig,sigIdx);
         
        /* Get Signal specific indices into the maps */
        uint_T   addrIdx  =  rtwCAPI_GetSignalAddrIdx(bioSig,sigIdx);
        uint16_T dTypeIdx =  rtwCAPI_GetSignalDataTypeIdx(bioSig,sigIdx);
        uint16_T dimIdx   =  rtwCAPI_GetSignalDimensionIdx(bioSig,sigIdx);
        uint16_T fxpIdx   =  rtwCAPI_GetSignalFixPtIdx(bioSig,sigIdx); 
         
        /* Get Data type attributes of the signal from DataType Map*/
        uint8_T   slDataType  = rtwCAPI_GetDataTypeSLId(dTypeMap,dTypeIdx);
        boolean_T isComplex   = rtwCAPI_GetDataIsComplex(dTypeMap,dTypeIdx);
        uint16_T  dataSize    = rtwCAPI_GetDataTypeSize(dTypeMap,dTypeIdx);

        /* Get the actual dimensions of the signal from the Dimension Map */
        uint8_T             nDims  = rtwCAPI_GetNumDims(dimMap, dimIdx);
        uint_T              dIndex = rtwCAPI_GetDimArrayIndex(dimMap, dimIdx);
        rtwCAPI_Orientation orient = rtwCAPI_GetOrientation(dimMap,dimIdx);
        uint_T              *dims;
        uint_T              idx;

        /* Signal's Fixed Point information */
        real_T fSlope = 1.0;
        real_T fBias  = 0.0;
        int8_T fExp   = 0;
        if(rtwCAPI_GetFxpFracSlopePtr(fxpMap,fxpIdx) != NULL) {
            fSlope = rtwCAPI_GetFxpFracSlope(fxpMap,fxpIdx);       
            fBias  = rtwCAPI_GetFxpBias(fxpMap,fxpIdx); 
            fExp   = rtwCAPI_GetFxpExponent(fxpMap,fxpIdx); 
        }
        
        dims = (uint_T *) calloc(nDims, sizeof(uint_T));
        for(idx=0; idx<nDims; idx++) 
            {
                dims[idx] = dimArray[dIndex + idx];
            }  

        /* Presently, only scalar named signals can be logged */
        if((orient == rtwCAPI_SCALAR) && (strcmp(sigName,"NULL") && 
                                          strcmp(sigName,""))) {
            /* Check if the memory is valid and create a Signal Buffer*/
            if (&sbInfo->sigBufs[sbInfo->numSigBufs] == NULL) {
                printf("Error allocating memory for sigBufs[%d]", 
                       sbInfo->numSigBufs);
                break;
            }
            capi_CreateSignalBuffer(&(sbInfo->sigBufs[sbInfo->numSigBufs]),
                                    sigName, 
                                    slDataType,
                                    dims,
                                    dataSize, 
                                    isComplex,
                                    maxSize,
                                    fSlope,
                                    fBias,
                                    fExp);
            sbInfo->numSigBufs = sbInfo->numSigBufs + 1;       
        }
        free(dims);
    }
}

void capi_UpdateOutputSignalBuffer(SignalBuffer *sigBuf, 
                             const void   *data,
                             uint_T        numDataPoints,
                             boolean_T     isComplex)
{

    /* Local variables for calculating real world values from fixed point */
    real_T currRealVal = 0.0;
    real_T currImagVal = 0.0;

    /* If the data is non-complex, update just the real buffer, *
     * else update real and imaginary buffers                   */
    if(!isComplex){
        /* The value of the data put in the buffer is decided by the      *
         * data-type of the signal. Each data-type is handled differently */
        switch (sigBuf->slDataType) {
          case SS_DOUBLE: {
              /* Pointer to the current position of the real part */
              real_T *currReal = ((real_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to real_T */
              real_T *_cData   = (real_T *) data;
              /* Assign the casted value to the current position */
              *currReal        = *_cData;
          }
            break;
          case SS_SINGLE: {
              /* Pointer to the current position of the real part */
              real32_T *currReal = ((real32_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to real_T */
              real32_T *_cData   = (real32_T *) data;
              /* Assign the casted value to the current position */
              *currReal          = *_cData;
          }
            break;
          case SS_UINT8: {
              /* Pointers to the current position of the real part */
              uint8_T *currReal = ((uint8_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to real32_T */
              uint8_T *_cData   = ((uint8_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData;
          }
            break;
          case SS_INT8: {
              /* Pointers to the current position of the real part */
              int8_T *currReal = ((int8_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to int8_T */
              int8_T *_cData = ((int8_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData; 
          }
            break;
          case SS_UINT16: {
              /* Pointers to the current position of the real part */
              uint16_T *currReal = ((uint16_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to uint16_T */
              uint16_T *_cData = ((uint16_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData;
          }
            break;
          case SS_INT16: {
              /* Pointers to the current position of the real part */
              int16_T *currReal = ((int16_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to int16_T */
              int16_T *_cData = ((int16_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData;
          }
            break;
          case SS_UINT32: {
              /* Pointers to the current position of the real part */
              uint32_T *currReal = ((uint32_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to uint32_T */
              uint32_T *_cData = ((uint32_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData;
          }
            break;
          case SS_INT32: {
              /* Pointers to the current position of the real part */
              int32_T *currReal = ((int32_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to int32_T */
              int32_T *_cData = ((int32_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData;
          }
            break;
        } /* end switch statement */
    } /* end if(!isComplex) */
    else {
        /* Signal is complex */
        switch (sigBuf->slDataType) {
          case SS_DOUBLE: {
              /* Pointers to the current position of the logged variable */
              real_T *currReal = ((real_T *) sigBuf->re) + numDataPoints;
              real_T *currImag = ((real_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex real_T */
              creal_T *_cData  = (( creal_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_SINGLE: {
              /* Pointers to the current position of the logged variable */
              real32_T *currReal = ((real32_T *) sigBuf->re) + numDataPoints;
              real32_T *currImag = ((real32_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex real32_T */
              creal32_T *_cData = ((creal32_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_UINT8: {
              /* Pointers to the current position of the logged variable */
              uint8_T *currReal = ((uint8_T *) sigBuf->re) + numDataPoints;
              uint8_T *currImag = ((uint8_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint8_T */
              cuint8_T *_cData = ((cuint8_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_INT8: {
              /* Pointers to the current position of the logged variable */
              int8_T *currReal = ((int8_T *) sigBuf->re) + numDataPoints;
              int8_T *currImag = ((int8_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex int8_T */
              cint8_T *_cData = ((cint8_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_UINT16: {
              /* Pointers to the current position of the logged variable */
              uint16_T *currReal = ((uint16_T *) sigBuf->re) + numDataPoints;
              uint16_T *currImag = ((uint16_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint16_T */
              const cuint16_T *_cData = ((const cuint16_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_INT16: {
              /* Pointers to the current position of the logged variable */
              int16_T *currReal = ((int16_T *) sigBuf->re) + numDataPoints;
              int16_T *currImag = ((int16_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex int16_T */
              cint16_T *_cData = ((cint16_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_UINT32: {
              /* Pointers to the current position of the logged variable */
              uint32_T *currReal = ((uint32_T *) sigBuf->re) + numDataPoints;
              uint32_T *currImag = ((uint32_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint32_T */
              cuint32_T *_cData = ((cuint32_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
          case SS_INT32: {
              /* Pointers to the current position of the logged variable */
              int32_T *currReal = ((int32_T *) sigBuf->re) + numDataPoints;
              int32_T *currImag = ((int32_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint32_T */
              cint32_T *_cData = ((cint32_T *)data);
              /* Assign the casted value to the current position */
              *(currReal) = _cData->re;
              *(currImag) = _cData->im;
          }
            break;
        }
    }
}



void capi_UpdateInputSignalBuffer(SignalBuffer *sigBuf, 
                             const void   *data,
                             uint_T        numDataPoints,
                             boolean_T     isComplex)
{

    /* Local variables for calculating real world values from fixed point */
    real_T currRealVal = 0.0;
    real_T currImagVal = 0.0;

    /* If the data is non-complex, update just the real buffer, *
     * else update real and imaginary buffers                   */
    if(!isComplex){
        /* The value of the data put in the buffer is decided by the      *
         * data-type of the signal. Each data-type is handled differently */
        switch (sigBuf->slDataType) {
          case SS_DOUBLE: {
              /* Pointer to the current position of the real part */
              real_T *currReal = ((real_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to real_T */
              real_T *_cData   = (real_T *) data;
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
          case SS_SINGLE: {
              /* Pointer to the current position of the real part */
              real32_T *currReal = ((real32_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to real_T */
              real32_T *_cData   = (real32_T *) data;
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
          case SS_UINT8: {
              /* Pointers to the current position of the real part */
              uint8_T *currReal = ((uint8_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to real32_T */
              uint8_T *_cData   = ((uint8_T *)data);
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
          case SS_INT8: {
              /* Pointers to the current position of the real part */
              int8_T *currReal = ((int8_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to int8_T */
              int8_T *_cData = ((int8_T *)data);
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
          case SS_UINT16: {
              /* Pointers to the current position of the real part */
              uint16_T *currReal = ((uint16_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to uint16_T */
              uint16_T *_cData = ((uint16_T *)data);
              /* Assign the real-world value to the current position */
              *(currReal)       = *_cData;
          }
            break;
          case SS_INT16: {
              /* Pointers to the current position of the real part */
              int16_T *currReal = ((int16_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to int16_T */
              int16_T *_cData = ((int16_T *)data);
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
          case SS_UINT32: {
              /* Pointers to the current position of the real part */
              uint32_T *currReal = ((uint32_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to uint32_T */
              uint32_T *_cData = ((uint32_T *)data);
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
          case SS_INT32: {
              /* Pointers to the current position of the real part */
              int32_T *currReal = ((int32_T *) sigBuf->re) + numDataPoints;
              /* Cast the data value to int32_T */
              int32_T *_cData = ((int32_T *)data);
              /* Set signal value to the current buffer position */
              *_cData        = *currReal;
          }
            break;
        } /* end switch statement */
    } /* end if(!isComplex) */
    else {
        /* Signal is complex */
        switch (sigBuf->slDataType) {
          case SS_DOUBLE: {
              /* Pointers to the current position of the logged variable */
              real_T *currReal = ((real_T *) sigBuf->re) + numDataPoints;
              real_T *currImag = ((real_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex real_T */
              creal_T *_cData  = (( creal_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_SINGLE: {
              /* Pointers to the current position of the logged variable */
              real32_T *currReal = ((real32_T *) sigBuf->re) + numDataPoints;
              real32_T *currImag = ((real32_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex real32_T */
              creal32_T *_cData = ((creal32_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_UINT8: {
              /* Pointers to the current position of the logged variable */
              uint8_T *currReal = ((uint8_T *) sigBuf->re) + numDataPoints;
              uint8_T *currImag = ((uint8_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint8_T */
              cuint8_T *_cData = ((cuint8_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_INT8: {
              /* Pointers to the current position of the logged variable */
              int8_T *currReal = ((int8_T *) sigBuf->re) + numDataPoints;
              int8_T *currImag = ((int8_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex int8_T */
              cint8_T *_cData = ((cint8_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_UINT16: {
              /* Pointers to the current position of the logged variable */
              uint16_T *currReal = ((uint16_T *) sigBuf->re) + numDataPoints;
              uint16_T *currImag = ((uint16_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint16_T */
              cuint16_T *_cData = ((cuint16_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_INT16: {
              /* Pointers to the current position of the logged variable */
              int16_T *currReal = ((int16_T *) sigBuf->re) + numDataPoints;
              int16_T *currImag = ((int16_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex int16_T */
              cint16_T *_cData = ((cint16_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_UINT32: {
              /* Pointers to the current position of the logged variable */
              uint32_T *currReal = ((uint32_T *) sigBuf->re) + numDataPoints;
              uint32_T *currImag = ((uint32_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint32_T */
              cuint32_T *_cData = ((cuint32_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
          case SS_INT32: {
              /* Pointers to the current position of the logged variable */
              int32_T *currReal = ((int32_T *) sigBuf->re) + numDataPoints;
              int32_T *currImag = ((int32_T *) sigBuf->im) + numDataPoints;
              /* Cast the data value to complex uint32_T */
              cint32_T *_cData = ((cint32_T *)data);
              /* Set signal value to the current buffer position */
              _cData->re = *(currReal);
              _cData->im = *(currImag);
          }
            break;
        }
    }
}

int capi_UpdateBlockIOLogging(rtwCAPI_ModelMappingInfo* mmi, signal_direction direction)
{

    /* Get pointers to the various Maps containing information about Signals */
    rtwCAPI_DataTypeMap  const *dTypeMap = rtwCAPI_GetDataTypeMap(mmi);   
    rtwCAPI_DimensionMap const *dimMap   = rtwCAPI_GetDimensionMap(mmi);  
    rtwCAPI_ElementMap   const *elemMap  = rtwCAPI_GetElementMap(mmi);     
    void *                  *dataAddrMap = rtwCAPI_GetDataAddressMap(mmi); 
    uint_T               const *dimArray = rtwCAPI_GetDimensionArray(mmi);
    int nr,ni;
  
    /* Get BlockIOSignals structure array from the Model map */
    rtwCAPI_Signals const *bioSig =  rtwCAPI_GetSignals(mmi);

    /* Get number of Block IO Signals in the structure array */
    uint_T numBIOSig = rtwCAPI_GetNumSignals(mmi);    

    /* Local variables */
    uint_T sigIdx, logsigIdx;

    /*************************
     * update Signal Buffers *
     *************************/

    /* Loop through the BlockIOSignals structure array */
    for(sigIdx = 0, logsigIdx = 0; sigIdx < numBIOSig; sigIdx++) {

        /* Get signal specific indices into the maps */
        uint_T   addrIdx  =  rtwCAPI_GetSignalAddrIdx(bioSig,sigIdx);
        uint16_T dTypeIdx =  rtwCAPI_GetSignalDataTypeIdx(bioSig,sigIdx);
        uint16_T dimIdx   =  rtwCAPI_GetSignalDimensionIdx(bioSig,sigIdx);

        /* From the above indices and maps get information specific 
           to this signal. This information will be used to update 
           signal buffer */
        
        /* signal address */
        void *sigAddr = dataAddrMap[addrIdx];
                
        /* Signal Orientation */
        rtwCAPI_Orientation  orient = rtwCAPI_GetOrientation(dimMap,dimIdx);

        /* Signal Complexity */
        boolean_T isComplex = rtwCAPI_GetDataIsComplex(dTypeMap,dTypeIdx);

        /* Signal data type size */
        uint16_T   dataSize = rtwCAPI_GetDataTypeSize(dTypeMap,dTypeIdx);
        
	SignalBuffer *sigBuf = &sbInfo->sigBufs[logsigIdx];

        /* Update the Signal buffer. 
           Presently, only scalar signals can be logged */
        char_T const *sName = sbInfo->sigBufs[logsigIdx].sigName;
        if(strcmp(sName,"NULL") && strcmp(sName,"")) {
		if (sigBuf->direction == direction) {
			switch(direction) {
				case INPUT: 
					if (sigBuf->curelem == 0) {
						nr = ReadItf(sigBuf->fd_re, sigBuf->re, sigBuf->buffersize);
						if (nr<0) {
							printf("Error reading real flow\n");
							ClosePHAL();
						}
						if (isComplex) {
							ni = ReadItf(sigBuf->fd_im, sigBuf->im, sigBuf->buffersize);
							if (ni<0) {
								printf("Error reading imag flow\n");
								ClosePHAL();
							}
							if (nr!=ni) {
								printf("Caution received different real-imag lengths %d!=%d\n",nr,ni);
							}							
						}
					}			
					if (nr>0) {
						capi_UpdateInputSignalBuffer(sigBuf, 
				                    sigAddr, sigBuf->curelem, 
				                    isComplex);
						sigBuf->curelem++;
						if (sigBuf->numelems == sigBuf->curelem) {
							sigBuf->curelem=0;
						}
					} 
				break;
				case OUTPUT: 
					capi_UpdateOutputSignalBuffer(sigBuf, 
		                            sigAddr, sigBuf->curelem, 
		                            isComplex);
					sigBuf->curelem++;
					if (sigBuf->numelems == sigBuf->curelem) {
						sigBuf->curelem=0;
						printf("sending %d thru %d\n",sigBuf->buffersize,sigBuf->fd_re);
						nr = WriteItf(sigBuf->fd_re, sigBuf->re, sigBuf->buffersize);
						if (nr<0) {
							printf("Error writting real flow\n");
							ClosePHAL();
						}
						if (!nr) {
							printf("Caution missed samples, buffer full\n");
						}
						if (isComplex) {
							ni = WriteItf(sigBuf->fd_im, sigBuf->im, sigBuf->buffersize);
							if (ni<0) {
								printf("Error writting imag flow\n");
								ClosePHAL();
							}
							if (nr!=ni) {
								printf("Caution writted different real-imag lengths %d!=%d\n",nr,ni);
							}							
						}
					}
				break;
			}
		}
            logsigIdx++;
            }
    }

    return 1;
}

void initialize_signals(rtwCAPI_ModelMappingInfo* mmi) {
  int i=0;
  
 capi_StartBlockIOLogging(mmi,1);
}

/* LNX nullhandler for SIGALRM handler */
void nullhandler(int signo) {
  return;
}

/* LNX SIGALRM handler for RT deadline violations */
void deadlineviolation(int signo)
{
  printf("[LNX] * * Real-Time deadline violation ! * *\n");
  return;
}

/* LNX stop handler for SIGINT and SIGTERM handler */
void stophandler(int signo) {
  GBLbuf.stopExecutionFlag = 1;
  return;
}

#ifdef BORLAND
/* Implemented for BC++ only*/

typedef void (*fptr)(int, int);

/* Function: divideByZero =====================================================
 *
 * Abstract: Traps the error Division by zero and prints a warning
 *           Also catches other FP errors, but does not identify them
 *           specifically.
 */
void divideByZero(int sigName, int sigType)
{
    signal(SIGFPE, (fptr)divideByZero);
    if ((sigType == FPE_ZERODIVIDE)||(sigType == FPE_INTDIV0)){
        printf("[LNX] *** Warning: Division by zero\n\n");
        return;
    }
    else{
        printf("[LNX] *** Warning: Floating Point error\n\n");
        return;
    }
} /* end divideByZero */

#endif /* BORLAND */

#if !defined(MULTITASKING)  /* SINGLETASKING */

/* Function: rtOneStep ========================================================
 *
 * Abstract:
 *      Perform one step of the model. This function is modeled such that
 *      it could be called from an interrupt service routine (ISR) with minor
 *      modifications.
 */
static void rt_OneStep(RT_MODEL *S)
{
    real_T tnext;

    /***********************************************
     * Check and see if base step time is too fast *
     ***********************************************/

    if (GBLbuf.isrOverrun++) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    /***********************************************
     * Check and see if error status has been set  *
     ***********************************************/

    if (rtmGetErrorStatus(S) != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    /* enable interrupts here */

    /*
     * In a multi-tasking environment, this would be removed from the base rate
     * and called as a "background" task.
     */
    rtExtModeOneStep(rtmGetRTWExtModeInfo(S),
                     rtmGetNumSampleTimes(S),
                     (boolean_T *)&rtmGetStopRequested(S));

    tnext = rt_SimGetNextSampleHit();
    rtsiSetSolverStopTime(rtmGetRTWSolverInfo(S),tnext);

    MdlOutputs(0);

    rtExtModeSingleTaskUpload(S);

    GBLbuf.errmsg = rt_UpdateTXYLogVars(rtmGetRTWLogInfo(S),
                                        rtmGetTPtr(S));
    if (GBLbuf.errmsg != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    rt_UpdateSigLogVars(rtmGetRTWLogInfo(S), rtmGetTPtr(S));

    MdlUpdate(0);
    rt_SimUpdateDiscreteTaskSampleHits(rtmGetNumSampleTimes(S),
                                       rtmGetTimingData(S),
                                       rtmGetSampleHitPtr(S),
                                       rtmGetTPtr(S));

    if (rtmGetSampleTime(S,0) == CONTINUOUS_SAMPLE_TIME) {
        rt_UpdateContinuousStates(S);
    }

    GBLbuf.isrOverrun--;

    rtExtModeCheckEndTrigger();

} /* end rtOneStep */

#else /* MULTITASKING */

# if TID01EQ == 1
#  define FIRST_TID 1
# else
#  define FIRST_TID 0
# endif

/* Function: rtOneStep ========================================================
 *
 * Abstract:
 *      Perform one step of the model. This function is modeled such that
 *      it could be called from an interrupt service routine (ISR) with minor
 *      modifications.
 *
 *      This routine is modeled for use in a multitasking environment and
 *	therefore needs to be fully re-entrant when it is called from an
 *	interrupt service routine.
 *
 * Note:
 *      Error checking is provided which will only be used if this routine
 *      is attached to an interrupt.
 *
 */
static void rt_OneStep(RT_MODEL *S)
{
    int_T  eventFlags[NUMST];
    int_T  i;
    real_T tnext;
    int_T  *sampleHit = rtmGetSampleHitPtr(S);

    /***********************************************
     * Check and see if base step time is too fast *
     ***********************************************/

    if (GBLbuf.isrOverrun++) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    /***********************************************
     * Check and see if error status has been set  *
     ***********************************************/

    if (rtmGetErrorStatus(S) != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }
    /* enable interrupts here */

    /*
     * In a multi-tasking environment, this would be removed from the base rate
     * and called as a "background" task.
     */
    rtExtModeOneStep(rtmGetRTWExtModeInfo(S),
                     rtmGetNumSampleTimes(S),
                     (boolean_T *)&rtmGetStopRequested(S));

    /************************************************************************
     * Update discrete events and buffer event flags locally so that ISR is *
     * re-entrant.                                                          *
     ************************************************************************/

    tnext = rt_SimUpdateDiscreteEvents(rtmGetNumSampleTimes(S),
                                       rtmGetTimingData(S),
                                       rtmGetSampleHitPtr(S),
                                       rtmGetPerTaskSampleHitsPtr(S));
    rtsiSetSolverStopTime(rtmGetRTWSolverInfo(S),tnext);
    for (i=FIRST_TID+1; i < NUMST; i++) {
        eventFlags[i] = sampleHit[i];
    }

    /*******************************************
     * Step the model for the base sample time *
     *******************************************/

    MdlOutputs(FIRST_TID);

    rtExtModeUploadCheckTrigger(rtmGetNumSampleTimes(S));
    rtExtModeUpload(FIRST_TID,rtmGetTaskTime(S, FIRST_TID));

    GBLbuf.errmsg = rt_UpdateTXYLogVars(rtmGetRTWLogInfo(S),
                                        rtmGetTPtr(S));
    if (GBLbuf.errmsg != NULL) {
        GBLbuf.stopExecutionFlag = 1;
        return;
    }

    rt_UpdateSigLogVars(rtmGetRTWLogInfo(S), rtmGetTPtr(S));

    MdlUpdate(FIRST_TID);

    if (rtmGetSampleTime(S,0) == CONTINUOUS_SAMPLE_TIME) {
        rt_UpdateContinuousStates(S);
    }
     else {
        rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(S), 
                                     rtmGetTimingData(S), 0);
    }

#if FIRST_TID == 1
    rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(S), 
                                 rtmGetTimingData(S),1);
#endif


    /************************************************************************
     * Model step complete for base sample time, now it is okay to          *
     * re-interrupt this ISR.                                               *
     ************************************************************************/

    GBLbuf.isrOverrun--;


    /*********************************************
     * Step the model for any other sample times *
     *********************************************/

    for (i=FIRST_TID+1; i<NUMST; i++) {
        if (eventFlags[i]) {
            if (GBLbuf.overrunFlags[i]++) {  /* Are we sampling too fast for */
                GBLbuf.stopExecutionFlag=1;  /*   sample time "i"?           */
                return;
            }

            MdlOutputs(i);

            rtExtModeUpload(i, rtmGetTaskTime(S,i));

            MdlUpdate(i);

            rt_SimUpdateDiscreteTaskTime(rtmGetTPtr(S), 
                                         rtmGetTimingData(S),i);

            /* Indicate task complete for sample time "i" */
            GBLbuf.overrunFlags[i]--;
        }
    }

    rtExtModeCheckEndTrigger();

} /* end rtOneStep */

#endif /* MULTITASKING */


static void displayUsage (void)
{
    (void) printf("usage: %s -tf <finaltime> -w -port <TCPport>\n",QUOTE(MODEL));
    (void) printf("arguments:\n");
    (void) printf("  -tf <finaltime> - overrides final time specified in "
                  "Simulink (inf for no limit).\n");
    (void) printf("  -w              - waits for Simulink to start model "
                  "in External Mode.\n");
    (void) printf("  -port <TCPport> - overrides 17725 default port in "
                  "External Mode, valid range 256 to 65535.\n");
}

/*===================*
 * Visible functions *
 *===================*/


/* Function: main =============================================================
 *
 * Abstract:
 *      Execute model on a generic target such as a workstation.
 */
int_T main(int_T argc, const char_T *argv[])
{
    RT_MODEL  *S;
    const char *status;
    real_T     finaltime = -2.0;

    int_T  oldStyle_argc;
    const char_T *oldStyle_argv[5];

    /* LNX Timer and priority variables */
    struct sched_param my_sched_params;
    struct itimerval interval;
    sigset_t wait_mask;
    int old_usec = 0;

    rtwCAPI_ModelMappingInfo* mmi;


    /******************************
     * MathError Handling for BC++ *
     ******************************/
#ifdef BORLAND
//    signal(SIGFPE, (fptr)divideByZero);
#endif

    InitPHAL();

    /*******************
     * Parse arguments *
     *******************/

    if ((argc > 1) && (argv[1][0] != '-')) {
        /* old style */
        if ( argc > 3 ) {
            displayUsage();
            exit(EXIT_FAILURE);
        }

        oldStyle_argc    = 1;
        oldStyle_argv[0] = argv[0];
    
        if (argc >= 2) {
            oldStyle_argc = 3;

            oldStyle_argv[1] = "-tf";
            oldStyle_argv[2] = argv[1];
        }

        if (argc == 3) {
            oldStyle_argc = 5;

            oldStyle_argv[3] = "-port";
            oldStyle_argv[4] = argv[2];

        }

        argc = oldStyle_argc;
        argv = oldStyle_argv;

    }

    {
        /* new style: */
        double    tmpDouble;
        char_T tmpStr2[200];
        int_T  count      = 1;
        int_T  parseError = FALSE;

        /*
         * Parse the standard RTW parameters.  Let all unrecognized parameters
         * pass through to external mode for parsing.  NULL out all args handled
         * so that the external mode parsing can ignore them.
         */
        while(count < argc) {
            const char_T *option = argv[count++];
            
            /* final time */
            if ((strcmp(option, "-tf") == 0) && (count != argc)) {
                const char_T *tfStr = argv[count++];
                
                sscanf(tfStr, "%200s", tmpStr2);
                if (strcmp(tmpStr2, "inf") == 0) {
                    tmpDouble = RUN_FOREVER;
                } else {
                    char_T tmpstr[2];

                    if ( (sscanf(tmpStr2,"%lf%1s", &tmpDouble, tmpstr) != 1) ||
                         (tmpDouble < 0.0) ) {
                        (void)printf("finaltime must be a positive, real value or inf\n");
                        parseError = TRUE;
                        break;
                    }
                }
                finaltime = (real_T) tmpDouble;

                argv[count-2] = NULL;
                argv[count-1] = NULL;
            }
        }

        if (parseError) {
            (void)printf("\nUsage: %s -option1 val1 -option2 val2 -option3 "
                         "...\n\n", QUOTE(MODEL));
            (void)printf("\t-tf 20 - sets final time to 20 seconds\n");

            exit(EXIT_FAILURE);
        }

        rtExtModeParseArgs(argc, argv, NULL);

        /*
         * Check for unprocessed ("unhandled") args.
         */
        {
/*            int i;
            for (i=1; i<argc; i++) {
                if (argv[i] != NULL) {
                    printf("Unexpected command line argument: %s\n",argv[i]);
                    exit(EXIT_FAILURE);
                }
            }
*/        }
    }

    /****************************
     * Initialize global memory *
     ****************************/
    (void)memset(&GBLbuf, 0, sizeof(GBLbuf));


    
    /************************
     * Initialize the model *
     ************************/
    rt_InitInfAndNaN(sizeof(real_T));

    S = MODEL();
    if (rtmGetErrorStatus(S) != NULL) {
        (void)fprintf(stderr,"[LNX] Error during model registration: %s\n",
                      rtmGetErrorStatus(S));
        exit(EXIT_FAILURE);
    }
    if (finaltime >= 0.0 || finaltime == RUN_FOREVER) rtmSetTFinal(S,finaltime);

    MdlInitializeSizes();
    MdlInitializeSampleTimes();
    
    status = rt_SimInitTimingEngine(rtmGetNumSampleTimes(S),
                                    rtmGetStepSize(S),
                                    rtmGetSampleTimePtr(S),
                                    rtmGetOffsetTimePtr(S),
                                    rtmGetSampleHitPtr(S),
                                    rtmGetSampleTimeTaskIDPtr(S),
                                    rtmGetTStart(S),
                                    &rtmGetSimTimeStep(S),
                                    &rtmGetTimingData(S));

    if (status != NULL) {
        (void)fprintf(stderr,
                "[LNX] Failed to initialize sample time engine: %s\n", status);
        exit(EXIT_FAILURE);
    }
    rt_CreateIntegrationData(S);

#ifdef UseMMIDataLogging
    rt_FillStateSigInfoFromMMI(rtmGetRTWLogInfo(S),&rtmGetErrorStatus(S));
    rt_FillSigLogInfoFromMMI(rtmGetRTWLogInfo(S),&rtmGetErrorStatus(S));
#endif
    GBLbuf.errmsg = rt_StartDataLogging(rtmGetRTWLogInfo(S),
                                        rtmGetTFinal(S),
                                        rtmGetStepSize(S),
                                        &rtmGetErrorStatus(S));
    if (GBLbuf.errmsg != NULL) {
        (void)fprintf(stderr,"[LNX] Error starting data logging: %s\n",GBLbuf.errmsg);
        return(EXIT_FAILURE);
    }

    rtExtModeCheckInit(rtmGetNumSampleTimes(S));
    rtExtModeWaitForStartPkt(rtmGetRTWExtModeInfo(S),
                             rtmGetNumSampleTimes(S),
                             (boolean_T *)&rtmGetStopRequested(S));

    (void)printf("\n[LNX] ** starting the model **\n");

    MdlStart();
    if (rtmGetErrorStatus(S) != NULL) {
      GBLbuf.stopExecutionFlag = 1;
    }

    
    /*************************************************************************
     * Execute the model.  You may attach rtOneStep to an ISR, if so replace *
     * the call to rtOneStep (below) with a call to a background task        *
     * application.                                                          *
     *************************************************************************/

    /* LNX setup high priority task */
/*IGM
    my_sched_params.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler( 0, SCHED_FIFO, &my_sched_params)) {
      perror("scheduling");
      exit(1);
    }
*/
    /* LNX lock all pages in memory */
    mlockall(MCL_CURRENT | MCL_FUTURE);

    /* LNX: setup timer for periodic signal (SIGALRM) */

    sigemptyset(&wait_mask);
    
    printf("[LNX] PID: %d\n", getpid());
    fflush(stdout);

/**IGM    while (!GBLbuf.stopExecutionFlag &&
           (rtmGetTFinal(S) == RUN_FOREVER ||
            rtmGetTFinal(S)-rtmGetT(S) > rtmGetT(S)*DBL_EPSILON)) {
*/

    

    
    mmi = &(rtmGetDataMapInfo(S).mmi);
	printf("Model %s initiated\n",S->modelName); 
    while(1) {
      switch(Status()) {
      case PHAL_STATUS_STOP:
        ClosePHAL();
        break;
      case PHAL_STATUS_INIT:
	initialize_signals(mmi);
	break;  
      case PHAL_STATUS_RUN:
      
      rtExtModePauseIfNeeded(rtmGetRTWExtModeInfo(S),
			     rtmGetNumSampleTimes(S),
			     (boolean_T *)&rtmGetStopRequested(S));
      
      if (rtmGetStopRequested(S)) break;

      capi_UpdateBlockIOLogging(mmi,INPUT);
      rt_OneStep(S);
      capi_UpdateBlockIOLogging(mmi,OUTPUT);
    break;
    }
    Relinquish(1);
    
    }
    
    /********************
     * Cleanup and exit *
     ********************/

    /* LNX unlock pages from memory */
    munlockall();

#ifdef UseMMIDataLogging
    rt_CleanUpForStateLogWithMMI(rtmGetRTWLogInfo(S));
    rt_CleanUpForSigLogWithMMI(rtmGetRTWLogInfo(S));
#endif
    rt_StopDataLogging(MATFILE,rtmGetRTWLogInfo(S));

    rtExtModeShutdown(rtmGetNumSampleTimes(S));

    if (GBLbuf.errmsg) {
        (void)fprintf(stderr,"%s\n",GBLbuf.errmsg);
        exit(EXIT_FAILURE);
    }

    if (GBLbuf.isrOverrun) {
        (void)fprintf(stderr,
                      "%s: ISR overrun - base sampling rate is too fast\n",
                      QUOTE(MODEL));
        exit(EXIT_FAILURE);
    }

    if (rtmGetErrorStatus(S) != NULL) {
        (void)fprintf(stderr,"%s\n", rtmGetErrorStatus(S));
        exit(EXIT_FAILURE);
    }
#ifdef MULTITASKING
    else {
        int_T i;
        for (i=1; i<NUMST; i++) {
            if (GBLbuf.overrunFlags[i]) {
                (void)fprintf(stderr,
                        "%s ISR overrun - sampling rate is too fast for "
                        "sample time index %d\n", QUOTE(MODEL), i);
                exit(EXIT_FAILURE);
            }
        }
    }
#endif

    MdlTerminate();
    return(EXIT_SUCCESS);

} /* end main */


/* EOF: lnx_main.c */

