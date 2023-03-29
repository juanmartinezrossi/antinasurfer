
#include <stdlib.h>
#include "ctrl_source.h"

ctrl_source_t ctrl_source;

void init_source(ctrl_source_t * control, int type, int length){

	control->typesource = type;
	control->datalength = length; 
}

void bitrand (int len, char * output)
{
	int i;
	for (i=0; i<len; i++)
	{
		output[i] = rand()&0x01;
	}
}

void charrand (int len, char * output)
{
	int i;
	for (i=0; i<len; i++)
	{
		output[i] = rand()&0xFF;
	}
}


void allzeros (int len, char * output)
{
	int i;
	for (i=0; i<len; i++)
	{
		output[i] = 0;
	}
}

void allones (int len, char * output)
{
	int i;
	for (i=0; i<len; i++)
	{
		output[i] = 1;
	}
}

void pseudorandom (int len, char * output)
{
	int i;
	for (i=0; i<len; i++)
	{
		output[i] = ((i+i)%11)&0x01;
	}
}

void source (ctrl_source_t * control, char * data)
{
	if (control->typesource==RANDOM)
		bitrand(control->datalength, data);
	if (control->typesource==ALLONES)
		allones(control->datalength, data);
	if (control->typesource==ALLZEROS)
		allzeros(control->datalength, data);
	if (control->typesource==PSEUDORANDOM)
		pseudorandom(control->datalength, data);
	if (control->typesource==CHARRANDOM)
		charrand(control->datalength, data);
}
