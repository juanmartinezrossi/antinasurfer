/*
 *
 * File: capi_BIOSignalLog.h
 *
 * Abstract:
 *   Header file for Block I/O Signal logging routines using buffers 
 *   of fixed size. See capi_BIOSignalLoh.c for function definitions
 *
 */

#ifndef _CAPI_BIOSIGNALLOG_H_
# define _CAPI_BIOSIGNALLOG_H_

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "rtw_modelmap.h"

#define MAX_DATA_POINTS 5000

typedef enum {
	INPUT,
	OUTPUT,
	INOUT
} signal_direction;

/* SignalBuffer structure - Structure to log a Signal */
/* Will be used to store the value of a BlockIO signal as the code executes */
typedef struct SignalBuffer_tag {
    char_T     sigName[81]; /* Signal name                                */
    void      *re;          /* Real Part - Buffers up the real part       */
    void      *im;          /* Imaginary Part - Buffers up imaginary part */
    real_T     slope;       /* slope if stored as an integer              */
    real_T     bias;        /* bias if stored as integer                  */
    uint8_T    slDataType;  /* Data Type of the Signal (Enumerated value) */
    signal_direction direction;
    int32_T	fd_re;
    int32_T	fd_im;
    uint32_T	buffersize;
    uint32_T    numelems;
    uint32_T    curelem;
}SignalBuffer;

/* SignalBufferInfo - Structure to log time and Signal Buffers */
/* Will be used to store a number of Block IO signals and book keeeping */
typedef struct SignalBufferInfo_tag {
    SignalBuffer *sigBufs;      /* Pointer to logged Signal Buffers        */
    time_T       *time;         /* Time buffer                             */
    uint_T        numSigBufs;   /* Number of Signal Buffers                */
}SignalBufferInfo;


extern void capi_CreateSignalBuffer(SignalBuffer   *sigBuf,
                                    char_T const   *varName,
                                    uint8_T         slDataType, 
                                    uint_T         *dims, 
                                    uint16_T        dataSize,
                                    boolean_T       isComplex,
                                    int_T           maxSize,
                                    real_T          fSlope,
                                    real_T          fBias,
                                    int8_T          fExp);


extern void capi_StartBlockIOLogging(rtwCAPI_ModelMappingInfo *mmi, 
                                     int_T                     maxSize);

extern void capi_UpdateSignalBuffer(SignalBuffer *sigBuf, 
                                    const void   *data,
                                    uint_T        numDataPoints,
                                    boolean_T     isComplex);
                                    

extern int capi_UpdateBlockIOLogging(rtwCAPI_ModelMappingInfo *mmi,
				     signal_direction	direction);


extern void capi_StopBlockIOLogging(const char_T *file);


#endif /* _CAPI_BIOSIGNALLOG_H */

/* EOF capi_BIOSignal.h */
