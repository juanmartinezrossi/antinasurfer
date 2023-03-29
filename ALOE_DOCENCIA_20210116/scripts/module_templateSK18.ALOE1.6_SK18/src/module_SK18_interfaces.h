#ifndef _INTERFACES_H
#define	_INTERFACES_H

#include <swapi_utils.h>
#include <phal_sw_api.h>
//#include <cmdman_backend.h>


/* DATA_TYPES: 
		char: 						TYPE_CHAR, 
		short: 						TYPE_SHORT, 
		int: 							TYPE_INT, 
		float: 						TYPE_FLOAT, 
		_Complex float: 	TYPE_COMPLEX, 
		user_defined:			TYPE_CUSTOM 
*/

/*************** INPUTS ***************/

#define NUM_INPUT_ITFS 					4						/* Define the number of input interfaces */

struct itf_info IN_ITF_INFO[NUM_INPUT_ITFS+1] = {
		
	/*ITF0*/	{TYPE_CHAR, sizeof(char), 1*2048},				/*{DATA_TYPE, SIZEOF(DATA_TYPE), MAX_LENGTH_BUFFER_IN_SAMPLES}*/
	/*ITF1*/	{TYPE_INT, sizeof(int), 2*2048},
	/*ITF2*/	{TYPE_FLOAT, sizeof(float), 3*2048},
	/*ITF3*/	{TYPE_COMPLEX, sizeof(_Complex float), 4*2048},
	/* Add here as many as module input interfaces*/
	/** end */
	{0, 0, 0}};


/*************** OUTPUTS ***************/

#define NUM_OUTPUT_ITFS 				4						/* Define the number of output interfaces */

struct itf_info OUT_ITF_INFO[NUM_OUTPUT_ITFS+1] = {
		
	{TYPE_CHAR, sizeof(char), 1*2048},							/*{DATA_TYPE, SIZEOF(DATA_TYPE), MAX_LENGTH_BUFFER_IN_SAMPLES}*/
	{TYPE_INT, sizeof(int), 2*2048},
	{TYPE_FLOAT, sizeof(float), 3*2048},
	{TYPE_COMPLEX, sizeof(_Complex float), 4*2048},
	/** end */
	{0, 0, 0}};


/** number of samples to capture from input/output signals */
#define VIEW_CAPTURE_SAMPLES		512
/** buffer offset for signals capture */
#define VIEW_CAPTURE_OFFSET		0

#endif
