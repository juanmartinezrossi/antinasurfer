
#ifndef ALSACARDRX_H
#define ALSACARDRX_H

//#include <alsa/asoundlib.h>


int alsacardRX_readcfg(char *filename, int *NsamplesIn, int *NsamplesOut);

int alsacardRX_init(int *timeSlot, float *rxBuffer, float *txBuffer, void (*sync)(void));

void alsacardRX_close();



#endif
