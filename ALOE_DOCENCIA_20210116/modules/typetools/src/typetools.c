#include "typetools.h"

#include "phal_sw_api.h"

/******************************
 *    CONVOLUTION FUNCTION    *
 ******************************/
int convii(int *filter, int *state, int length, int din)
{
	int i;
	int res=0;

	for (i=length-1; i>0; i--)
		state[i]=state[i-1];
	state[0]=din;

	for (i=0; i<length; i++)
		res+=filter[i]*state[i];

	return(res);
}

short convss(short *filter, short *state, int length, short din)
{
	int i;
	short res=0;

	for (i=length-1; i>0; i--)
		state[i]=state[i-1];
	state[0]=din;

	for (i=0; i<length; i++)
		res+=filter[i]*state[i];

	return(res);
}

float convff(float *filter, float *state, int length, float din)
{
	int i;
	float res=0;

	for (i=length-1; i>0; i--)
		state[i]=state[i-1];
	state[0]=din;

	for (i=0; i<length; i++)
		res+=filter[i]*state[i];

	return(res);
}

void bin2char (char *input, char *output, int outputlength)
{
    char x;
    int i;

    for (i=0; i<outputlength; i++)
    {
        if ((i%8)==0) {
            x=input[i/8];
        }
        output[i]=(x & 0x80)>>7;
        x<<=1;
    }
}

void bin2int8 (char *input, int *output, int outputlength)
{
    char x;
    int i;

    for (i=0; i<outputlength; i++)
    {
        if ((i%8)==0) {
            x=input[i/8];
        }
        output[i]=(x & 0x80)>>7;
        x<<=1;
    }
}


void bin2float (char *input, float *output, int length)
{
    char x;
    int i;

    for (i=0; i<length; i++)
    {
        if ((i%8)==0) {
            x=input[i/8];
        }
        output[i]=(x & 0x80)>>7;
        x<<=1;
    }
}

void char2bin (char *input, char *output, int inputlength)
{
    int x;
    int i,c;

    x=0;
    c=0;
    for (i=0; i<inputlength; i++)
    {
        if (input[i]) {
            x|=0x1;
        }
        c++;
        if (c==8 || i==inputlength-1) {
            output[i/8]=((char) (x & 0xff))<<(8-c);
            x=0;
            c=0;
        } else {
            x<<=1;
        }
    }
}


void int82bin (int *input, char *output, int inputlength)
{
    int x;
    int i,c;

    x=0;
    c=0;
    for (i=0; i<inputlength; i++)
    {
        if (input[i]) {
            x|=0x1;
        }
        c++;
        if (c==8 || i==inputlength-1) {
            output[i/8]=((char) (x & 0xff))<<(8-c);
            x=0;
            c=0;
        } else {
            x<<=1;
        }
    }
}



void float2bin (float *input, char *output, int length)
{
    int x;
    int i,c;

    x=0;
    c=0;
    for (i=0; i<length; i++)
    {
        if (input[i]) {
            x|=0x1;
        }
        c++;
        if (c==8 || i==length-1) {
            output[i/8]=((char) (x & 0xff))<<(8-c);
            x=0;
            c=0;
        } else {
            x<<=1;
        }
    }
}

void type2int(void *input, int *output, int len, int datatype)
{
    int i;
    char *inc;
    short *ins;
    int *ini;
    float *inf;

    if (datatype==TYPE_INT) {
    	memcpy(output,input,len*sizeof(int));
    }

    switch(datatype)
    {
        case TYPE_BITSTREAM:
            bin2int8((char*) input, output, len);
            break;
        case TYPE_CHAR:
            inc=input;
            for (i=0;i<len;i++) {
                output[i] = (int) inc[i];
            }
            break;
        case TYPE_SHORT:
            ins=(short*) input;
            for (i=0;i<len;i++) {
                output[i] = (int) ins[i];
            }
            break;
        case TYPE_INT:
            ini=(int*) input;
            for (i=0;i<len;i++) {
                output[i] = (int) ini[i];
            }
            break;
        case TYPE_FLOAT:
            inf=(float*) input;
            for (i=0;i<len;i++) {
                output[i] = (int) inf[i];
            }
            break;
    }

}


void type2intAUTOSCALE(void *input, int *output, int len, int datatype, int maxint)
{
    int i;
    char *inc;
    short *ins;
    int *ini;
    float *inf;

    float *temp, maxIN=0.0, escale;
    int maxINi=0;

/*    if(datatype==TYPE_FLOAT){
        temp=(float*) input;
        for(i=0; i<len; i++){
            if(temp[i]>maxIN)maxIN=temp[i];
            if(-temp[i]>maxIN)maxIN=-temp[i];
        }
        escale=(float)maxint/maxIN;
        for(i=0; i<len; i++)temp[i]=temp[i]*escale;
    }
*/

    switch(datatype)
    {
        case TYPE_BITSTREAM:
            bin2int8((char*) input, output, len);
            printf("TYPETOOLS: NOT READY-BITSTREAM");
            break;
        case TYPE_CHAR:
            printf("TYPETOOLS: NOT READY-CHAR");
            inc=input;
            for (i=0;i<len;i++) {
                output[i] = (int) inc[i];
            }
            break;
        case TYPE_SHORT:
            printf("TYPETOOLS: NOT READY-SHORT");
            ins=(short*) input;
            for (i=0;i<len;i++) {
                output[i] = (int) ins[i];
            }
            break;
        case TYPE_INT:
            ini=(int*) input;
            for(i=0; i<len; i++){
                if(ini[i]>maxINi)maxINi=ini[i];
                if(-ini[i]>maxINi)maxINi=-ini[i];
            }
            escale=(float)maxint/maxINi;
            for(i=0; i<len; i++)ini[i]=ini[i]*escale;
            for (i=0;i<len;i++) {
                output[i] = (int) ini[i];
            }
            break;
        case TYPE_FLOAT:
            inf=(float*) input;
            for(i=0; i<len; i++){
                if(inf[i]>maxIN)maxIN=inf[i];
                if(-inf[i]>maxIN)maxIN=-inf[i];
            }
            escale=(float)maxint/maxIN;
            for(i=0; i<len; i++)inf[i]=inf[i]*escale;

            for (i=0;i<len;i++) {
                output[i] = (int) inf[i];
            }
            break;
    }

}




void int2type(int *input, void *output, int len, int datatype)
{
    int i;
    char *inc;
    short *ins;
    int *ini;
    float *inf;

    if (datatype==TYPE_INT) {
    	memcpy(output,input,len*sizeof(int));
    }

    switch(datatype)
    {
        case TYPE_BITSTREAM:
            int82bin(input, (char*) output, len);
            break;
        case TYPE_CHAR:
            inc=(char*) output;
            for (i=0;i<len;i++) {
                inc[i] = (char) input[i];
            }
            break;
        case TYPE_SHORT:
            ins=(short*) output;
            for (i=0;i<len;i++) {
                ins[i] = (int) input[i];
            }
            break;
        case TYPE_INT:
            ini=(int*) output;
            for (i=0;i<len;i++) {
                ini[i] = (int) input[i];
            }
            break;
        case TYPE_FLOAT:
            inf=(float*) output;
            for (i=0;i<len;i++) {
                inf[i] = (int) input[i];
            }
            break;
    }

}




void float2type(float *input, void *output, int len, int datatype)
{
    int i;
    char *inc;
    short *ins;
    int *ini;
    float *inf;

    switch(datatype)
    {
        case TYPE_BITSTREAM:
            printf("Not allowed\n");
            break;
        case TYPE_CHAR:
            inc=(char*) output;
            for (i=0;i<len;i++) {
                inc[i] = (char) input[i];
            }
            break;
        case TYPE_SHORT:
            ins=(short*) output;
            for (i=0;i<len;i++) {
                ins[i] = (int) input[i];
            }
            break;
        case TYPE_INT:
            ini=(int*) output;
            for (i=0;i<len;i++) {
                ini[i] = (int) input[i];
            }
            break;
        case TYPE_FLOAT:
            inf=(float*) output;
            for (i=len;i--; ) {
               *((float *)output++) = *(input++);
            }
            break;
    }

}

void type2float(void *input, float *output, int len, int datatype)
{
    int i;
    char *inc;
    short *ins;
    int *ini;
    float *inf;

    switch(datatype)
    {
        case TYPE_BITSTREAM:
            bin2float ((char*) input, output, len);
            break;
        case TYPE_CHAR:
            inc=input;
            for (i=0;i<len;i++) {
                output[i] = (float) inc[i];
            }
            break;
        case TYPE_SHORT:
            ins=(short*) input;
            for (i=0;i<len;i++) {
                output[i] = (float) ins[i];
            }
            break;
        case TYPE_INT:
            ini=(int*) input;
            for (i=0;i<len;i++) {
                output[i] = (float) ini[i];
            }
            break;
        case TYPE_FLOAT:
            inf=(float*) input;
            for (i=len;i--; ) {
                *(output++) = (float) *(inf++);
            }
            break;
    }

}




int typeNsamplesArray(int datatype, int size)
{
    if (!size)
        return 0;

    datatype=datatype&0x0F;
    switch(datatype)
    {
        case TYPE_BITSTREAM:
            return size*8;
        default:
            return ((size-1)/typeSize(datatype) + 1);
    }
}

int typeSizeArray(int datatype, int nsamples)
{
    if (!nsamples)
        return 0;

    switch(datatype)
    {
        case TYPE_BITSTREAM:
            return DIV8(nsamples);
        default:
            return nsamples*typeSize(datatype);
    }
}

int typeSize(int datatype)
{
    switch(datatype)
    {
        case TYPE_BITSTREAM:
            return sizeof(char);
        case TYPE_BIT8:
            return sizeof(char);
        case TYPE_INT8:
            return sizeof(char);
        case TYPE_INT16:
            return sizeof(short);
        case TYPE_INT32:
            return sizeof(int);
        case TYPE_FLOAT:
            return sizeof(float);
    }
}

int type2stats(int datatype)
{
    switch(datatype)
    {
        case TYPE_BITSTREAM:
            return STAT_TYPE_UCHAR;
        case TYPE_BIT8:
            return STAT_TYPE_UCHAR;
        case TYPE_INT8:
            return STAT_TYPE_UCHAR;
        case TYPE_INT16:
            return STAT_TYPE_SHORT;
        case TYPE_INT32:
            return STAT_TYPE_INT;
        case TYPE_FLOAT:
            return STAT_TYPE_FLOAT;
    }
}

#ifdef TYPE2TYPE_TONI
/** This function return the number of data out according with the output data type
 *  Return -1 en case of incorrect data type conversion
 */

int type2type(void *input, void *output,\
                int typeIN, int typeOUT,\
                int dataINlength, float MAXvalue)
{
    static int one=0;
    int len, i, a;

    float *input_f, *output_f;
    char *input_c, *output_c;
    int *input_i, *output_i;
    short *input_s, *output_s;
    float escale;


     switch( typeOUT )
            {
            case TYPE_BITSTREAM:
                    switch( typeIN )
                        {
                        case TYPE_BITSTREAM:
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=dataINlength;       //dataINlength = number of incoming bits
                            for(i=0; i<len; i++)*(output_c+i)=*(input_c+i);
                            return(len);
                            break;
                        case TYPE_BIT8:
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=dataINlength;
                            char2bin (input_c, output_c, len);
                            return(len);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;

            case TYPE_BIT8:
                switch( typeIN )
                        {
                        case (TYPE_BITSTREAM):
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=typeSizeArray(typeOUT, dataINlength);
                           // printf("TYPETOOLS.typet2type() \n");
                           // printf("len=%d dataINlength=%d\n", len, dataINlength);
                            bin2char (input_c, output_c, len);
                            return(len);
                            break;
                        case (TYPE_BIT8):
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=dataINlength;
                            for(i=0; i<len; i++)*(output_c+i)=*(input_c+i);
                            return(len);
                            break;
                         default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
            case TYPE_INT8:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_c=(char*) input;
                            output_c=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(int)*(input_c+i);
                            return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_s=(short*) input;
                            output_c=(char*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(char)((*(input_s+i))/256);
                            return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_i=(int*) input;
                            output_c=(char*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(char)((*(input_i+i))/16843009);
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(float*) input;
                            output_c=(char*) output;
                            escale=MAXvalue/128.0;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(char)((*(input_f+i))/escale);
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
            case TYPE_INT16:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            //CHECKED printf("IN %d OUT %d\n", TYPE_INT8, TYPE_INT16);
                            input_c=(char*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=(short)*(input_c+i);
                              return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_s=(short*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=*(input_s+i);
                             return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEIN=INT32 TYPEOUT=INT16\n");
                            input_i=(int*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=(short)(*(input_i+i)/65536);
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(int*) input;
                            output_s=(short*) output;
                            escale=MAXvalue/32768.0;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=(short)((*(input_f+i))/escale);
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
            case TYPE_INT32:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            //printf("TYPETOOLS: TYPEIN %d TYPEOUT %d\n", TYPE_INT8, TYPE_INT32);
                            input_c=(char*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=(int)*(input_c+i);
                            return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            input_s=(short*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=*(input_s+i);
                            return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEIN=INT32 TYPEOUT=INT32\n");
                            input_i=(int*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=(int)*(input_i+i);
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(int*) input;
                            output_i=(int*) output;
                            escale=MAXvalue/2147483648.0;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=(int)((*(input_f+i))/escale);
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
            case TYPE_FLOAT:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            input_c=(char*) input;
                            output_f=(float*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_f+i)=(float)*(input_c+i);
                            return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            input_s=(short*) input;
                            output_f=(float*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_f+i)=*(input_s+i);
                            return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEIN=INT32 TYPEOUT=FLOAT\n");
                            input_i=(int*) input;
                            output_f=(float*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_f+i)=(float)*(input_i+i);
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(float*) input;
                            output_f=(float*) output;
                            if (gain==1.0){
								for(i=dataINlength; i--;)
										*(output_f++)=*(input_f++)*gain;
                            }else{
								for(i=dataINlength; i--;)
										*(output_f++)=*(input_f++);
                            }
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
             default:
                printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                return(-1);
                break;
     }


}
#endif

int type2type(void *input, void *output,\
                int typeIN, int typeOUT,\
                int dataINlength, float gain)
{
    static int one=0;
    int len, i, a;

    float *input_f, *output_f;
    char *input_c, *output_c;
    int *input_i, *output_i;
    short *input_s, *output_s;
    float escale;

     switch( typeOUT )
            {
            case TYPE_BITSTREAM:
                    switch( typeIN )
                        {
                        case TYPE_BITSTREAM:
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=dataINlength;       //dataINlength = number of incoming bits
                            for(i=0; i<len; i++)
                            	*(output_c+i)=*(input_c+i);
                            return(len);
                            break;
                        case TYPE_BIT8:
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=dataINlength;
                            for (i=0;i<len;i++)
															output_c[i]*=gain;
															char2bin (input_c, output_c, len);
                            return(len);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;

            case TYPE_BIT8:
                switch( typeIN )
                        {
                        case (TYPE_BITSTREAM):
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=typeSizeArray(typeOUT, dataINlength);
                           // printf("TYPETOOLS.typet2type() \n");
                           // printf("len=%d dataINlength=%d\n", len, dataINlength);
                            bin2char (input_c, output_c, len);
                            return(len);
                            break;
                        case (TYPE_BIT8):
                            input_c=(char*) input;
                            output_c=(char*) output;
                            len=dataINlength;
                            for(i=0; i<len; i++)
                            	*(output_c+i)=*(input_c+i)*gain;
                            return(len);
                            break;
                         default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
            case TYPE_INT8:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_c=(char*) input;
                            output_c=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(int)*(input_c+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_s=(short*) input;
                            output_c=(char*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(char) (*(input_s+i))*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_i=(int*) input;
                            output_c=(char*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(char)(*(input_i+i))*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(float*) input;
                            output_c=(char*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_c+i)=(char) (*(input_f+i))*gain;
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
            case TYPE_INT16:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            //CHECKED printf("IN %d OUT %d\n", TYPE_INT8, TYPE_INT16);
                            input_c=(char*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=(short)*(input_c+i)*gain;
                              return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            //CHECKED printf("TYPEINN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            input_s=(short*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=*(input_s+i)*gain;
                             return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEIN=INT32 TYPEOUT=INT16\n");
                            input_i=(int*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=(short)*(input_i+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(int*) input;
                            output_s=(short*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_s+i)=(short)(*(input_f+i))*gain;
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
            case TYPE_INT32:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            //printf("TYPETOOLS: TYPEIN %d TYPEOUT %d\n", TYPE_INT8, TYPE_INT32);
                            input_c=(char*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=(int)*(input_c+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            input_s=(short*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=*(input_s+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEIN=INT32 TYPEOUT=INT32\n");
                            input_i=(int*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=(int)*(input_i+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(float*) input;
                            output_i=(int*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_i+i)=(int)(*(input_f+i)*gain);
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
            case TYPE_FLOAT:
                     switch( typeIN )
                        {
                        case TYPE_INT8:
                            input_c=(char*) input;
                            output_f=(float*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_f+i)=(float)*(input_c+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_INT16):
                            input_s=(short*) input;
                            output_f=(float*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_f+i)=*(input_s+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_INT32):
                            //CHECKED printf("TYPEIN=INT32 TYPEOUT=FLOAT\n");
                            input_i=(int*) input;
                            output_f=(float*) output;
                            for(i=0; i<dataINlength; i++)
                                *(output_f+i)=(float)*(input_i+i)*gain;
                            return(dataINlength);
                            break;
                        case (TYPE_FLOAT):
                            input_f=(float*) input;
                            output_f=(float*) output;
                            if (gain==1.0){
								for(i=dataINlength; i--;)
										*(output_f++)=*(input_f++)*gain;
                            }else{
								for(i=dataINlength; i--;)
										*(output_f++)=*(input_f++);
                            }
                            return(dataINlength);
                            break;
                        default:
                            printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                            printf("TYPEIN=%d TYPEOUT=%d\n", typeIN, typeOUT);
                            return(-1);
                            break;
                }
                break;
             default:
                printf("TYPETOOLS.typet2type() TYPE CONVERSION NON VALID\n");
                return(-1);
                break;
     }


}


/*First binari (int) to MSB bit in char*/
int MSBfirstbin2char (int *input, unsigned char *output, int length)
{
    char x=0x00;
    int i, n=0, l=0;

    n=0;
    l=0;
    for (i=0; i<=length; i++)
    {
        if(i%8==0){
		if(i>0){
			output[l]=x;
			x=0x00;
			l++;
		}
	}
	else x<<=1;
        x=(char)(input[n] & 0x01) | x;
	n++;
    }
    return(length/8);
}


/*MSB bit first binari output*/
int MSBfirstchar2bin (unsigned char *input, int *output, int length)
{
    int x;
    int i,c;

    x=0;
    c=0;
    for (i=0; i<length; i++)
    {
	for(c=0; c<8; c++)output[i*8+c]=(int)(((input[i]<<c)&0x80)>>7);
    }
    return(length*8);
}



