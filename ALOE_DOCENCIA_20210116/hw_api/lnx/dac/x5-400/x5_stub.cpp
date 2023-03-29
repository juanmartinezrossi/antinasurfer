#include <iostream>
#include "BoardIo.h"

#include "x5400.h"

using namespace std;


BoardIo Board;

int ad_enable,da_enable,SamplingFreq,ClockInternal,SyncAD,Direct;
int ADsamples,ADdecimate,ADprint,ADscale;
int DAsamples,DAdecimate,DAinterpolate,DAdivider,DAdosin,DAsendad,DAprint,DAscale,DAsinfreq;

int x5_setcfg(int _ad_enable,int _da_enable,int _SamplingFreq,int _ClockInternal,int _SyncAD,
		int _ADsamples, int _ADdecimate, int _ADprint, int _ADscale,
		int _DAsamples,int _DAdecimate,int _DAinterpolate,int _DAdivider,int _DAdosin, int _DAsendad, int _DAprint, int _direct, int _DAsinfreq, int _DAscale)
{
	ad_enable=_ad_enable;
	da_enable=_da_enable;
	SamplingFreq=_SamplingFreq;
	ClockInternal=_ClockInternal;
	SyncAD=_SyncAD;
	ADsamples=_ADsamples;
	ADdecimate=_ADdecimate;
	ADprint=_ADprint;
	DAsamples=_DAsamples;
	DAdecimate=_DAdecimate;
	DAinterpolate=_DAinterpolate;
	DAdivider=_DAdivider;
	DAdosin=_DAdosin;
	DAsendad = _DAsendad;
	DAprint=_DAprint;
	Direct=_direct;
	DAsinfreq=_DAsinfreq;
	DAscale=_DAscale;
	ADscale=_ADscale;
}
int x5_init(int *ts_len, float *rxBuffer, float *txBuffer, void (*sync)(void))
{
	double x;
	Board.Open();
	Board.CfgMain(SamplingFreq,ClockInternal,SyncAD,rxBuffer,txBuffer, sync, Direct);
	if (ad_enable)
		Board.CfgAD(ADsamples,ADdecimate,ADprint,ADscale);
	if (da_enable)
		Board.CfgDA(DAsamples,DAdecimate,DAinterpolate,DAdivider,DAdosin,DAsendad,DAprint,DAsinfreq,DAscale);
	Board.StartStreaming();
	
	if (SyncAD) {
		x=(double) ADsamples*(ADdecimate+1)/SamplingFreq;
		*ts_len=(int) 1000000*x;
	} else {
		x=(double) DAsamples*(DAdecimate+1)/SamplingFreq;
		*ts_len=(int) 1000000*x;	
	}
	return 0;
}
void x5_close()
{
	Board.StopStreaming();
	Board.Close();
}
