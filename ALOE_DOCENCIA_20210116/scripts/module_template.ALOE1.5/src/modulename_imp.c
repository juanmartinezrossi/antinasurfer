/** ALOE headers
 */
#include <phal_sw_api.h>
#include <math.h>
#include <swapi_utils.h>

#include "typetools.h"

#include "itf_types.h"
#include "inputs.h"
#include "outputs.h"
#include "stats.h"

#include "modulename.h"


/** The basic signal processing function has one input and one output.
 * The third parameter indicates the number of received samples.
 * The function must return the number of samples to send.
 */
int process(int *input, int *output, int nsamples)
{
	/** your code goes here */


	/** return the number of samples to send */
	return nsamples;
}


/** This function is called during the initialization process, after
 * the parameters (in stats.h) have been retrieved.
 */
int InitCustom()
{
    return 1;
}

/** This function is called after the processing function, each time slot.
 * You can place here other code (e.g. state-update, etc.)
 */
int RunCustom()
{
    return 1;
}

/** this function is called when input data is received. By default,
 * it changes data type to integer, calls the process function, change
 * the data type to the configured datatype and sends the data through
 * the first output interface.
 */
int process_input(int len)
{
    int nsamples_in,nsamples_out;

    nsamples_in=typeNsamplesArray(datatype,len);

    type2int(input_data,input_i,nsamples_in,datatype);

    nsamples_out=process(input_i,output_i,nsamples_in);

    int2type(output_i,output_data,nsamples_out,datatype);

    SendItf(0,typeSizeArray(datatype,nsamples_out));

    return 1;
}


void ConfigInterfaces()
{

}



