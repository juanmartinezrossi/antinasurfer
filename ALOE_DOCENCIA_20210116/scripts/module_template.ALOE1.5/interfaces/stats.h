
int datatype;
int stat_outputsignal;


int input_i[INPUT_MAX_DATA],output_i[OUTPUT_MAX_DATA];

struct utils_param params[] = {
			 /** allowed types:
			 * bitstream 0
			 * char 1
			 * short 2
			 * int 3
			 * float 4
			 */
			{"datatype",STAT_TYPE_INT,1,&datatype},

			/** set here your parameters */
			{NULL, 0, 0, 0}};


struct utils_stat stats[] = {   
		 {"output_signal",
			STAT_TYPE_INT,
			512,
			&stat_outputsignal,
			(void*)output_i,
			WRITE},

			/** place here other statistics */

			{NULL, 0, 0, 0, 0, 0}};

