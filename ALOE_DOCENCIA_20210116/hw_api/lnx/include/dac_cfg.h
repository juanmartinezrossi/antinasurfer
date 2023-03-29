#ifndef DAC_CFG_H
#define DAC_CFG_H


#define DAC_NOF_CHANNELS	2
#define DAC_BUFFER_SZ		(128*1024)
#define DAC_FILENAME_LEN		128

/* Real signals */
#define DAC_SAMPLE_INT16			1
#define DAC_SAMPLE_INT32			2
#define DAC_SAMPLE_FLOAT			3

/** Complex signals, in C99, use _Complex type */
#define DAC_SAMPLE_COMPLEXINT16		4
#define DAC_SAMPLE_COMPLEXINT32		5
#define DAC_SAMPLE_COMPLEXFLOAT		6

#define DAC_NO_ACTIVE				0
#define DAC_ACTIVE					1


#ifdef __cplusplus

extern "C" struct dac_cfg {
	char filename[DAC_FILENAME_LEN];
	double inputFreq;	//Input sampling rate
	double outputFreq;	//Output sampling rate
	double inputRFFreq;
	double outputRFFreq;
	int sampleType;
	int nof_channels;
	float tx_gain;
	float rx_gain;
	float tx_bw;
	float rx_bw;
	int NsamplesIn;
	int NsamplesOut;
	_Complex float dacinbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
	_Complex float dacoutbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
	int DAC_Active;
};

#else
#include <math.h>
#include <complex.h>

struct dac_cfg {
	char filename[DAC_FILENAME_LEN];
	double inputFreq;
	double outputFreq;
	double inputRFFreq;
	double outputRFFreq;
	int sampleType;
	int nof_channels;
	float tx_gain;
	float rx_gain;
	float tx_bw;
	float rx_bw;
	int NsamplesIn;
	int NsamplesOut;
	_Complex float dacinbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
	_Complex float dacoutbuff[DAC_NOF_CHANNELS][DAC_BUFFER_SZ];
	int DAC_Active;
};

#endif
#endif
