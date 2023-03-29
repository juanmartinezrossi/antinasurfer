/* 
 * Copyright (c) 2012
 * This file is part of ALOE (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Functions that generate the test data fed into the DSP modules being developed */
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>

#include "UPLINK_DEMAPPING_functions.h"


/**
 * @brief Defines the function activity.
 * @param .
 *
 * @param lengths Save on n-th position the number of samples generated for the n-th interface
 * @return -1 if error, the number of output data if OK

 */
int getDATAfromLTEspectrum(_Complex float *LTEspect, int FFTlength, int datalength, _Complex float *QAMsymb){
	int i, j;

/*	j=FFTlength-CARRIERS1RB-datalength/2;
	for(i=0; i<datalength; i++){
		*(QAMsymb+i)=*(LTEspect+j);
		j++;
		if(j==FFTlength-CARRIERS1RB)j=CARRIERS1RB;
	}
*/
	j=FFTlength-datalength/2;
	for(i=0; i<datalength; i++){
		*(QAMsymb+i)=*(LTEspect+j);
		j++;
		if(j==FFTlength)j=0;
	}
	return(1);
}


