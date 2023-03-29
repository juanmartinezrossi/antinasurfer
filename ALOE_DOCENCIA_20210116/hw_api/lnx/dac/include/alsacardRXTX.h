
#ifndef ALSACARDRXTX_H
#define ALSACARDRXTX_H



int alsacardRXTX_readcfg(char *filename, int *NsamplesIn, int *NsamplesOut);

int alsacardRXTX_init(int *timeSlot, float *rxBuffer, float *txBuffer, void (*sync)(void));

void alsacardRXTX_close();



#endif
