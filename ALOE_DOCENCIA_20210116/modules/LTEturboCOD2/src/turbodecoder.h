/*
 *	Turbodecoder control structure
 */

#ifndef _TURBODECODER_H
#define _TURBODECODER_H

#define RATE 3
#define TOTALTAIL 12

#define LOG18 -2.07944

#define NUMSTATES 8
#define NINPUTS 2
#define TAIL 3
#define TOTALTAIL 12

#define ESCALA 80

#define INF 9e4
#define ZERO 9e-4

#define SW 150
#define GW 10

#ifndef MAX_LONG_CB
	#define MAX_LONG_CB     6144
#endif
#define MAX_LONG_CODED  (RATE*MAX_LONG_CB+TOTALTAIL)

#define MAX_CB 4

typedef float Tdec;

#define HALT_METHOD_MIN		1
#define HALT_METHOD_MEAN	2
#define HALT_METHOD_NONE	0

typedef struct {
	int Long_CodeBlock;
	int Turbo_iteracions;
	int Turbo_Dt;
	int haltMethod;
	int halt;
	int type;
} ctrl_turbodecoder_t;

int TurboDecoder(Tdec *input, char *output, ctrl_turbodecoder_t *control);

#endif // _TURBODECODER_H
