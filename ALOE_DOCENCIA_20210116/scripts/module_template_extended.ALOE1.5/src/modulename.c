/**
 * Module Name: _modulename_
 * Description:
 * Files: _modulename_.c
 *
 * Author: _user_
 * Created: _date_
 * Revision: 0.1
 *
 * Comments:
 *
 */

#include <phal_sw_api.h>
#include <math.h>
#include <swapi_utils.h>

#include "typetools.h"
#include "stats.h"

#include "moduleConfig.h"

/** This file must be included AFTER moduleConfig.h */
#include <skeletonExt.h>


/** Initialization function. This code is executed during the INIT phase only.
 * @return 0 fails initialization and stops execution of application.
 */
int myinit()
{
	printf("O    SPECIFIC PARAMETERS SETUP: %s.myinit().\n", GetObjectName());
	printf("O      ProcessingTypeIN=%d, ProcessingTypeOUT=%d, OutputDataLength=%d \n",\
				PROCESSING_TYPE_IN, PROCESSING_TYPE_OUT, outputlength);
	/** Add initial module configuration parameters for printing*/
	/**printf("O      Parameter1=%d, Parameter2=%d\n", parameter1, parameter2);*/
	printf("O------------------------------------------------------------------------------O\n");


	/** during the initialization phase, you can force a special type for an input or output
	 * interface. If you do so, declare the pointer to the data using INS(itf_num)
	 */
	/*set_special_type_IN(0,TYPE_SHORT);*/

	/* or for output interfaces use set_special_type_OUT() */


	return 1;
}


/** Main work function. The input and output buffers can be accessed through the
 * inputN and outputM pointers where N and M are the input and output interface
 * index
 *
 * @returns >0 Number of samples to send through ALL the output interfaces.
 * If some output interface sends a different number of samples, use setLength(M,size)
 * @returns <0 if error, stops execution
 */
int execute()
{
	int i;

	int *x = IN(0);				/** pointer to input interface 0 */
	/*short *y = (short*) INS(1);*/	/** special type pointer to interface 1 */
	int *z = OUT(0);			/** pointer to output interface 0 */

	/** your code goes here */
	for (i=0;i<getLength(0);i++) {
		z[i]=x[i];
	}

	/*setLength(0,i);	*/

	/** return the number of samples to send */
	return i;
}




