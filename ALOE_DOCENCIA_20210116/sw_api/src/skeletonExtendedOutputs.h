#ifndef _OUTPUT_H
#define	_OUTPUT_H
/** sends bitstream packed in bytes
 */
/** INTERFACE 1: OUTPUT*/
/** input buffer is declared as an array of bytes */
typedef char output1_t;


/**  maximum output buffer for data interface (in bytes)*/
#define OUTPUT_MAX_DATA	(100*1024)	/*Max Number of data symbols*/
output1_t output_data[OUTPUT_MAX_DATA];

/** declare get_output_length() function
 *  returns the number od bytes to be sent
 */
int get_output_length();

/** declare gen_output() function
 *  generates the data to be sent
 */
int generate_outputdata(int len);

/** configure output interfaces */
struct utils_itf output_itfs[] = {
                {"data_flow_out",
                 sizeof(output1_t),
                 OUTPUT_MAX_DATA,
                 output_data,
                 get_output_length,	/** A pointer to a function that
					 *  returns the  number of samples to
					 *  send.  
					 *  If set to NULL all available bytes
					 *  in the output FIFO will be sent */
                generate_outputdata,	/** Pointer to function to generate
					 *  output data 	*/
                },
                {NULL, 0, 0, 0, 0, 0}};


#endif
