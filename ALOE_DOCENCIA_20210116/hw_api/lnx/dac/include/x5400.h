#ifdef __cplusplus

extern "C" int x5_readcfg(char *name, int *NsamplesIn, int *NsamplesOut);

extern "C" int 	x5_setcfg(int ad_enable,int da_enable,int SamplingFreq,int ClockInternal,int SyncAD,
		int ADsamples, int ADdecimate, int ADprint,int ADscale,
		int DAsamples,int DAdecimate,int DAinterpolate,int DAdivider,int DAdosin,int DAsendad,int DAprint, int direct,int DAscale,int DAsinfreq);

extern "C" int x5_init(int *timeSlot, float *rxBuffer, float *txBuffer,
		void (*sync)(void));
extern "C" void x5_close();

#else
int x5_readcfg(char name, int *NsamplesIn, int *NsamplesOut);

int x5_init(int *timeSlot, float *rxBuffer, float *txBuffer,
		void (*sync)(void));
void x5_close();
int 	x5_setcfg(int ad_enable,int da_enable,int SamplingFreq,int ClockInternal,int SyncAD,
		int ADsamples, int ADdecimate, int ADprint,
		int DAsamples,int DAdecimate,int DAinterpolate,int DAdivider,int DAdosin,int DAsendad,int DAprint);
#endif
