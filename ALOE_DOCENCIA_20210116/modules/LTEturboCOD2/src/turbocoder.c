#include "turbocoder.h"
#include "permute.h"

#define NUMREGS 3

struct permute_t permute;


void TurboCoder(char *input, char *output, ctrl_turbocoder_t * control)
{

	int long_cb = control->blocklen;
	int type = control->type;
	
	char reg1_0,reg1_1,reg1_2, reg2_0,reg2_1,reg2_2;
	int i,k=0,j;
	char bit;
	char in,out;
	Tper *per;

	i=ComputePermutation(&permute, long_cb, type);

	per=permute.PER;
	
	reg1_0=0;
	reg1_1=0;
	reg1_2=0;
	
	reg2_0=0;
	reg2_1=0;
	reg2_2=0;
	
	k=0;
	for (i=0;i<long_cb;i++) {
		bit=input[i];
		
		output[k]=bit;
		k++;
		
		in=bit^(reg1_2^reg1_1);
		out=reg1_2^(reg1_0^in);

		reg1_2=reg1_1;
		reg1_1=reg1_0;
		reg1_0=in;
		
		output[k]=out;
		k++;
		
		bit=input[per[i]];
		
		in=bit^(reg2_2^reg2_1);
		out=reg2_2^(reg2_0^in);

		reg2_2=reg2_1;
		reg2_1=reg2_0;
		reg2_0=in;
		
		output[k]=out;
		k++;
	}
	
	k=3*long_cb;
	
	/* TAILING DEL CODER #1 */
	for (j=0;j<NUMREGS;j++) {
		bit=reg1_2^reg1_1;
		
		output[k]=bit;
		k++;
		
		in=bit^(reg1_2^reg1_1);
		out=reg1_2^(reg1_0^in);

		reg1_2=reg1_1;
		reg1_1=reg1_0;
		reg1_0=in;
		
		output[k]=out;
		k++;
	}
	/* TAILING DEL CODER #2 */
	
	for (j=0;j<NUMREGS;j++) {
		bit=reg2_2^reg2_1;
		
		output[k]=bit;
		k++;
			
		in=bit^(reg2_2^reg2_1);
		out=reg2_2^(reg2_0^in);

		reg2_2=reg2_1;
		reg2_1=reg2_0;
		reg2_0=in;
		
		output[k]=out;
		k++;
	}	
}

