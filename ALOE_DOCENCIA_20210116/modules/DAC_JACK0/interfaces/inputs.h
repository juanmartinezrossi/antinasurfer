/* 
 * Define here the inputs for the module
 */

#ifndef _INPUTS_H
#define	_INPUTS_H


/** Interface Name: input (From module point of view)
 *  
 *  Data type: Complex Float
 *  Max input samples: 1024
 *  Min input samples: 1
 *  
 *  Callback function: process_input()
 *
 *  Description: Input stream of data
 */

#define INPUT_MAX_DATA	1024*1024	/*Max Number of data symbols*/

/** Type declaration */
typedef _Complex float input_t;

/** buffer for data */
input_t input_data[INPUT_MAX_DATA];

struct dac_h ctrl_pkt;

/* processing function */
int process_input(int len);
int process_control(int len);

/** configure input interfaces */
struct utils_itf input_itfs[] = {
					{"control",
					sizeof(struct dac_h),
					INPUT_MAX_DATA,
					&ctrl_pkt,
					NULL,
					process_control},

					{"input",
                    sizeof(input_t),
                    INPUT_MAX_DATA,
                    input_data,
                    NULL,      
                    process_input},

                    {NULL, 0, 0, 0, 0, 0}};

/** =============== End ================  */





#endif	/* _INPUTS_H */

