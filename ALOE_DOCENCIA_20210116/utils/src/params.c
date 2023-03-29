/*
 * params.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
 *
 *
 * This file is part of ALOE.
 *
 * ALOE is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ALOE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ALOE.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "params.h"

int string_to_argv (char *string, char **argv, int max_args)
  {
    int i;
    int argc;
    
    argc = 0;
    argv[0] = string;
    argc = 1;
    
    for (i = 0; string[i] != '\0'; i++)
      {
        if (isspace (string[i]) && argc < max_args)
          {
            string[i] = '\0';
            i++;
            argv[argc] = &string[i];
            argc++;
          }
      }
    
    argv[argc] = NULL;
    
    return argc;
  }

/*********************************************/
/****    CONVERT AND RETURN PARAMETERS    ****/
/*********************************************/
int get_param(int argc, char *argv[], char *marker, void *value, int flag)
{

	int j;
	int n;
	int found=0;
		
	if (argc==1)
		return(-1);
	
	if (marker==NULL)
	{
		for (j=1; j<argc; j++)
			if (argv[j][0]!='\0')
			{
				n=string_to_param(argv[j],value,flag);
				if (n<0)
					found=-1;
				else
				{
					/*Remove parameter from list*/
					argv[j][0]='\0';
					found=1;
				}
				break;
			}
	}
	else
	{
		for (j=0; j<argc; j++)
			/*Search for marker*/
			if (strcmp(marker,argv[j])==0)
			{
				/*Remove parameter form list*/
				argv[j][0]='\0';
				
				/*Does it require any value?*/
				if (value!=NULL)
				{
					/*Check availability of associated argument*/
					if ((argc-1)>j)
					{
						n=string_to_param(argv[j+1],value,flag);
						if (n<0)
							found=-1;
						else
						{
							/*Remove parameter from list*/
							argv[j+1][0]='\0';
							found=1;
						}
					}
					else found=-1;
				}
				else found=1;
				break;
			}
	} 
	return(found);
}

/*********************************************/
/****    TRANSFORM A STRING INTO VALUE    ****/
/*********************************************/
int string_to_param(char *svalue, void *pvalue, int flag)
{
	int i=0;
	char *pc;
	unsigned char *puc;
	short *pshort;
	unsigned short *pushort;
	int *pint;
	unsigned int *puint;
	float *pfloat;
	double *pdouble;
	int hex_value;
	
	switch (flag) {
	case PARAM_CHAR:
		pc=(char *)pvalue;
		pc[i]=(char)atoi(svalue);
		break;
	case PARAM_UCHAR:
		puc=(unsigned char *)pvalue;
		puc[i]=(unsigned char)atoi(svalue);
		break;
	case PARAM_SHORT:
		pshort=(short *)pvalue;
		pshort[i]=(short)atoi(svalue);
		break;
	case PARAM_USHORT:
		pushort=(unsigned short *)pvalue;
		pushort[i]=(unsigned short)atoi(svalue);
		break;
	case PARAM_INT:
		pint=(int *)pvalue;
		pint[i]=(int)atoi(svalue);
		break;
	case PARAM_UINT:
		puint=(unsigned int *)pvalue;
		puint[i]=(unsigned int)atoi(svalue);
		break;
	case PARAM_HEX:
		hex_value=(unsigned int)strtoul(svalue,NULL,16);
		puint=(unsigned int *)pvalue;
		*puint=hex_value;
		break;
	case PARAM_FLOAT:
		pfloat=(float *)pvalue;
		pfloat[i]=(float)atof(svalue);
		break;
	case PARAM_DOUBLE:
		pdouble=(double *)pvalue;
		pdouble[i]=(double)atof(svalue);
		break;
	case PARAM_BOOL:
		pc=(char *)pvalue;
		if (strcmp(svalue,"true")==0)
			pc[i]=1;
		else
			pc[i]=0;
	case PARAM_STRING:
		pc=(char *)pvalue;
		strcpy(pc,svalue);
		break;
	default:
		i=-1;
		break;
	}
		
	return(i);
}

char *param_to_string(void *pvalue, int flag) 
{
	char *str;

	switch (flag) {
	case PARAM_CHAR:
		if (asprintf(&str,"%d",*((char*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_UCHAR:
		if (asprintf(&str,"%d",*((unsigned char*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_SHORT:
		if (asprintf(&str,"%d",*((short*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_USHORT:
		if (asprintf(&str,"%d",*((unsigned short*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_INT:
		if (asprintf(&str,"%d",*((int*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_UINT:
		if (asprintf(&str,"%d",*((unsigned int*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_HEX:
		if (asprintf(&str,"0x%x",*((unsigned int*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_FLOAT:
		if (asprintf(&str,"%f",*((float*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_DOUBLE:
		if (asprintf(&str,"%f",*((double*) pvalue))==-1)
			str=NULL;
		break;
	case PARAM_BOOL:
		if (*((char*) pvalue))
			str=strdup("yes");
		else
			str=strdup("no");
		break;
	case PARAM_STRING:
		str=strdup((char*) pvalue);
		break;
	default:
		str=NULL;
		break;
	}
		
	return str;

}

