
extern char inputbufferProc[NUM_INPUT_ITFS * BUFFER_INPUT_KBYTES * 1024];
extern char outputbufferProc[NUM_OUTPUT_ITFS * BUFFER_OUTPUT_KBYTES * 1024];

#define IN(b) (input_t) &inputbufferProc[b*BUFFER_INPUT_KBYTES*1024]
#define OUT(b) (output_t) &outputbufferProc[b*BUFFER_OUTPUT_KBYTES*1024]

#define INS(b) &inputbufferProc[b*BUFFER_INPUT_KBYTES*1024]
#define OUTS(b) &outputbufferProc[b*BUFFER_OUTPUT_KBYTES*1024]

#if PROCESSING_TYPE_IN==TYPE_INT
	typedef int* input_t;
#elif PROCESSING_TYPE_IN==TYPE_SHORT
	typedef short* input_t;
#elif PROCESSING_TYPE_IN==TYPE_FLOAT
	typedef float* input_t;
#else
	typedef char* input_t;
#endif

#if PROCESSING_TYPE_OUT==TYPE_INT
	typedef int* output_t;
#elif PROCESSING_TYPE_OUT==TYPE_SHORT
	typedef short* output_t;
#elif PROCESSING_TYPE_OUT==TYPE_FLOAT
	typedef float* output_t;
#else
	typedef char* output_t;
#endif
