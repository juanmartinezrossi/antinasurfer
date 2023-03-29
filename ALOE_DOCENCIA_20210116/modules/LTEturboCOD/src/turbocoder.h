/*
 *	Turbocoder control structure
 */

#ifndef _TURBOCODER_H
#define _TURBOCODER_H

#define NUMREGS     3
#define MAX_LONG_CB 6114

#define RATE 3
#define TOTALTAIL 12


typedef struct {
	int type;
	int blocklen;
} ctrl_turbocoder_t;


void TurboCoder(char *input, char *output, ctrl_turbocoder_t * control);


#endif // _TURBOCODER_H

