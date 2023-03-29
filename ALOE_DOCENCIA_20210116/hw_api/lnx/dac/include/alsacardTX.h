
#ifndef ALSACARD_H
#define ALSACARD_H

//#include <alsa/asoundlib.h>


int alsacardTX_readcfg(char *filename, int *NsamplesIn, int *NsamplesOut);

int alsacardTX_init(int *timeSlot, float *rxBuffer, float *txBuffer, void (*sync)(void));

void alsacardTX_close();

//static void async_callback(snd_async_handler_t *ahandler);


#endif
