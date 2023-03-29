#ifndef _CTRL_RATEMATCHING
#define _CTRL_RATEMATCHING

/* Module modes */
#define CHAR_RM	1
#define FLOAT_UNRM 2

typedef struct {
	int mode;
	int insize;
	int outsize;
	int rvidx;
} ctrl_ratematching_t;

int ratematching (void * input, void * output, ctrl_ratematching_t * control);

#endif // _CTRL_RATEMATCHING
