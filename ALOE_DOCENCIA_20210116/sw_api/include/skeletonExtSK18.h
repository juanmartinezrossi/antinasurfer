#ifndef _SK18EXTSTATS_H
#define	_SK18EXTSTATS_H

#include <phal_sw_api.h>
#include <swapi_utils.h>
#include <assert.h>
#include <stdio.h>

#ifdef _COMPILE_MEX_INCLUDE
#include "mex.h"
#endif

/* Info and error messages print */
#define INFOSTR "[info at "
#define ERRSTR "[error at "
#define WHERESTR  "file %s, line %d]: "
#define WHEREARG  __FILE__, __LINE__
#ifdef _COMPILE_MEX
#define DEBUGPRINT2(out,...)       mexPrintf(__VA_ARGS__)
#elif _COMPILE_ALOE
#define DEBUGPRINT2(out,...)       Log(__VA_ARGS__)
#else
#define DEBUGPRINT2(out,...)       fprintf(out, __VA_ARGS__)
#endif
#define aerror_msg(_fmt, ...)  DEBUGPRINT2(stderr,ERRSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)
#define aerror(a)  DEBUGPRINT2(stderr, ERRSTR WHERESTR a, WHEREARG)

#define ainfo(a) DEBUGPRINT2(stdout, INFOSTR WHERESTR a, WHEREARG)
#define ainfo_msg(_fmt, ...)  DEBUGPRINT2(stdout,INFOSTR WHERESTR _fmt, WHEREARG, __VA_ARGS__)

#define modinfo ainfo
#define modinfo_msg ainfo_msg
#define moderror aerror
#define moderror_msg aerror_msg

int get_input_samples(int idx);
void set_output_samples(int idx, int nsamples);

void * in_p(int idx);
void * out_p(int idx);

void free_buffers_memory();

void cleanstring(char *string);
void printCOLORtext(int position, char *fore, char *ground,  int end, char *data2print);
void printCOLORtextline(int position, char *fore, char *ground,  int end, char *data2print);
void print_params(char *fore, char *ground);
void print_itfs_setup(int mode, int ncolums, int TS2print, int nprintTS, int offset, int maxlen);
#endif
