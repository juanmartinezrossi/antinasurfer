#ifndef _INPUTS
#define	_INPUTS

/** Sample input interfaces configuration file.
 *
 * This file configures static input interfaces. In general, data types are
 * configurable (with an initialization parameter). Therefore, here we will
 * declare the input_data buffer as a simple byte. In the processing function,
 * typetools library will convert from a variable type to an internal type.
 *
 * If the component only allows certain type, e.g. BITSTREAM in a CRC, you
 * MUST INDICATE it in this file as a comment.
 *
 *
 * Read sw_api/include/swapi_utils.h and   
 * http://flexnets.upc.edu/trac/wiki/ObjectDeveloperGuide
 * for more documentation on how to use this file
 *
 */

/** INTERFACE 1: CONTROL
 *  maximum input buffer for control interface (in bytes)
 */
#define INPUT_MAX_CONTROL	1

/** This is the structure for the control interface. It is declared in the
 *  itf_types.h file. Other components sending control parameters to the module
 *  will include the itf_types.h file
 */
ControlMODULENAME_h ctrlpkt;

/** declare processing function for the control interface */
int process_inputcontrol(int len);
/** declare control in length function. returns number of control wordtypes to be readed */
int controlin_length();

/** INTERFACE 2: DATA
 *  maximum input buffer for data interface (in bytes)
 */
#define INPUT_MAX_DATA	100*1024

/** input buffer is declared as an array of bytes */
typedef char input1_t;
input1_t input_data[INPUT_MAX_DATA];

/** declare processing function for the input interface */
int process_inputdata(int len);

/** declare data in length function
 *  returns the number od bytes to be readed
 */
int datain_length();

/** configure input interfaces */
struct utils_itf input_itfs[] = {
        {
	"control_flow_in",    	/** Name of the interface (used in the .app) */
        sizeof(ctrlpkt),        /** Size of the sample in bytes              */
        INPUT_MAX_CONTROL,      /** Maximum number of samples (of the buffer)*/
        &ctrlpkt,               /** Buffer to store input data               */
        controlin_length,       /** A pointer to a function that returns the
                                *   number of samples to receive.
				*   If set to NULL all pending bytes in the
                 		*   input FIFO will be readed and stored in
                                *   the buffer 	*/
        process_inputcontrol	/** Pointer to function process control info*/
	},       	
        {
	"data_flow_in",
        sizeof(input1_t),  	/** here sample size is 1-byte 		*/
        INPUT_MAX_DATA,    	/** and number of samples is buffer size */
        input_data,
        datain_length,        /** This function return the number of bytes to*/
                                /*  be processed by the data processing funct */
//	NULL,                   /** again, we want to read all pending data   */
        process_inputdata	/** Pointer to function to process data info  */
	},
	{NULL, 0, 0, 0, 0, 0}};
#endif
/** ============================== End ========================================================  */


