/*
 *	Mapper control structure
 */

#ifndef _CTRL_SOURCE
#define _CTRL_SOURCE

/* Block mode */
#define ALLZEROS 				0
#define ALLONES  				1
#define RANDOM	 				2
#define PSEUDORANDOM	 	3
#define CHARRANDOM	 		4

typedef struct {
	int typesource;
	int datalength;
} ctrl_source_t;

#endif // _CTRL_SOURCE
