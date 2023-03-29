#include <stdio.h>

#include "permute.h"
#include "turbodecoder.h"

/* UMTS Interleaver */
unsigned short table_p[52] = {7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257};
unsigned char table_v[52] = {3,2,2,3,2,5,2,3,2,6,3,5,2,2,2,2,7,5,3,2,3,5,2,5,2,6,3,3,2,3,2,2,6,5,2,5,2,2,2,19,5,2,3,2,3,2,6,3,7,7,6,3};

unsigned char v;
unsigned short p;
unsigned short s[MAX_COLS],q[MAX_ROWS],r[MAX_ROWS],T[MAX_ROWS];
unsigned short U[MAX_COLS*MAX_ROWS];

//Tper PER[MAX_LONG_CB],DESPER[MAX_LONG_CB];
Tper PER[MAX_LONG_CODED],DESPER[MAX_LONG_CODED];

int M_Rows,M_Cols,M_long;

/* LTE interleaver */
const int k_list[NUM_LTE_SIZES] = {40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120, 128, 136, 144, 152, 160, 168, 176, 184, 192, 200, 208, 216, 224, 232, 240, 248, 256, 264, 272, 280, 288, 296, 304, 312, 320, 328, 336, 344, 352, 360, 368, 376, 384, 392, 400, 408, 416, 424, 432, 440, 448, 456, 464, 472, 480, 488, 496, 504, 512, 528, 544, 560, 576, 592, 608, 624, 640, 656, 672, 688, 704, 720, 736, 752, 768, 784, 800, 816, 832, 848, 864, 880, 896, 912, 928, 944, 960, 976, 992, 1008, 1024, 1056, 1088, 1120, 1152, 1184, 1216, 1248, 1280, 1312, 1344, 1376, 1408, 1440, 1472, 1504, 1536, 1568, 1600, 1632, 1664, 1696, 1728, 1760, 1792, 1824, 1856, 1888, 1920, 1952, 1984, 2016, 2048, 2112, 2176, 2240, 2304, 2368, 2432, 2496, 2560, 2624, 2688, 2752, 2816, 2880, 2944, 3008, 3072, 3136, 3200, 3264, 3328, 3392, 3456, 3520, 3584, 3648, 3712, 3776, 3840, 3904, 3968, 4032, 4096, 4160, 4224, 4288, 4352, 4416, 4480, 4544, 4608, 4672, 4736, 4800, 4864, 4928, 4992, 5056, 5120, 5184, 5248, 5312, 5376, 5440, 5504, 5568, 5632, 5696, 5760, 5824, 5888, 5952, 6016, 6080, 6144};

const int f1_list[NUM_LTE_SIZES] = {3, 7, 19, 7, 7, 11, 5, 11, 7, 41, 103, 15, 9, 17, 9, 21, 101, 21, 57, 23, 13, 27, 11, 27, 85, 29, 33, 15, 17, 33, 103, 19, 19, 37, 19, 21, 21, 115, 193, 21, 133, 81, 45, 23, 243, 151, 155, 25, 51, 47, 91, 29, 29, 247, 29, 89, 91, 157, 55, 31, 17, 35, 227, 65, 19, 37, 41, 39, 185, 43, 21, 155, 79, 139, 23, 217, 25, 17, 127, 25, 239, 17, 137, 215, 29, 15, 147, 29, 59, 65, 55, 31, 17, 171, 67, 35, 19, 39, 19, 199, 21, 211, 21, 43, 149, 45, 49, 71, 13, 17, 25, 183, 55, 127, 27, 29, 29, 57, 45, 31, 59, 185, 113, 31, 17, 171, 209, 253, 367, 265, 181, 39, 27, 127, 143, 43, 29, 45, 157, 47, 13, 111, 443, 51, 51, 451, 257, 57, 313, 271, 179, 331, 363, 375, 127, 31, 33, 43, 33, 477, 35, 233, 357, 337, 37, 71, 71, 37, 39, 127, 39, 39, 31, 113, 41, 251, 43, 21, 43, 45, 45, 161, 89, 323, 47, 23, 47, 263};

const int f2_list[NUM_LTE_SIZES] = {10, 12, 42, 16, 18, 20, 22, 24, 26, 84, 90, 32, 34, 108, 38, 120, 84, 44, 46, 48, 50, 52, 36, 56, 58, 60, 62, 32, 198, 68, 210, 36, 74, 76, 78, 120, 82, 84, 86, 44, 90, 46, 94, 48, 98, 40, 102, 52, 106, 72, 110, 168, 114, 58, 118, 180, 122, 62, 84, 64, 66, 68, 420, 96, 74, 76, 234, 80, 82, 252, 86, 44, 120, 92, 94, 48, 98, 80, 102, 52, 106, 48, 110, 112, 114, 58, 118, 60, 122, 124, 84, 64, 66, 204, 140, 72, 74, 76, 78, 240, 82, 252, 86, 88, 60, 92, 846, 48, 28, 80, 102, 104, 954, 96, 110, 112, 114, 116, 354, 120, 610, 124, 420, 64, 66, 136, 420, 216, 444, 456, 468, 80, 164, 504, 172, 88, 300, 92, 188, 96, 28, 240, 204, 104, 212, 192, 220, 336, 228, 232, 236, 120, 244, 248, 168, 64, 130, 264, 134, 408, 138, 280, 142, 480, 146, 444, 120, 152, 462, 234, 158, 80, 96, 902, 166, 336, 170, 86, 174, 176, 178, 120, 182, 184, 186, 94, 190, 480};

int ComputePermutation(struct permute_t *permute, int Long_CodeBlock, int type)
{
	if (type == PER_LTE)
		return ComputeLTEPermutation(permute, Long_CodeBlock);
	if (type == PER_UMTS)
		return ComputeUMTSPermutation(permute, Long_CodeBlock);
	return -1;
}

int getLTEcbsize (int numbits)
{
	int j;
	if (numbits<k_list[0])
		return 0;

	for (j=0; j<NUM_LTE_SIZES && k_list[j]<numbits; j++);

	if (j==NUM_LTE_SIZES)
		return (-1);

	return k_list[j];
}

int ComputeLTEPermutation(struct permute_t *permute, int Long_CodeBlock)
{
	int i;
	int found=0;
	Tper f1, f2, index;

	permute->PER=PER;
	permute->DESPER=DESPER;

	i=0;
	while ((i<NUM_LTE_SIZES)&&(found==0)) {
		if (k_list[i]>=Long_CodeBlock) found=k_list[i];
		else i++;
	}

	if (found!=0){
		f1=f1_list[i];
		f2=f2_list[i];
	}
	else
		return (-1);

	PER[0] = 0;
	DESPER[i] = 0;
	index = 0;
	for (i=1; i<Long_CodeBlock;i++)
	{
		index = (index+f1+2*i*f2-f2)%(Long_CodeBlock);
		PER[i]=index;
		DESPER[index]=i;
	}
	//printf("LTE interleaver params loaded (f1: %d, f2: %d)\n", f1, f2);
	return 0;

}
	
int ComputeUMTSPermutation(struct permute_t *permute, int Long_CodeBlock) {

	//int M_Rows,M_Cols,M_long;
	int i,j;
	int res,prim,aux;
	int kp,k;
	Tper *per,*desper;
	
	permute->PER=PER;
	permute->DESPER=DESPER;

	M_long=Long_CodeBlock;

/*Determinar R*/
	if( (40 <= M_long)&&(M_long <= 159) )
		M_Rows = 5;
	else if( ( (160 <= M_long)&&(M_long <=200) ) || ((481 <= M_long) && (M_long <=530)))
		M_Rows = 10;
	else
		M_Rows = 20;

/*Determinar p i v*/
	if( (481 <= M_long)&&(M_long <= 530) ) {
		p = 53;
		v = 2;
		M_Cols = p;
	}
	else {
		i=0;
		do {
			p = table_p[i];
			v = table_v[i];
			i++;
		} while(M_long > (M_Rows*(p+1)) );
		
	}

/*Determinar C*/
	if( (M_long) <= (M_Rows)*((p)-1) )
		M_Cols = (p) - 1;
	else if(( (M_Rows)*(p-1) < M_long)&&(M_long <= (M_Rows)*(p) ))
		M_Cols = p;
	else if( (M_Rows)*(p) < M_long)
		M_Cols = (p) + 1;


	// calculem q
	q[0]=1;
	prim=6;

	for (i=1;i<M_Rows;i++) {
		do {
			prim++;
			res=mcd(prim,p-1);
		} while(res!=1);
		q[i]=prim;
	}

	// calculem s
	s[0]=1;
	for (i=1;i<p-1;i++) {
		s[i]=(v*s[i-1])%p;
	}
	// calculem T
	if (M_long<=159 && M_long>=40) {
		T[0]=4;T[1]=3;T[2]=2;T[3]=1;T[4]=0;
	}
	else if ((M_long<=200 && M_long>=160) || (M_long<=530 && M_long>=481)) {
		T[0]=9;T[1]=8;T[2]=7;T[3]=6;T[4]=5;T[5]=4;T[6]=3;T[7]=2;T[8]=1;T[9]=0;
	}
	else if ((M_long<=2480 && M_long>=2281) || (M_long<=3210 && M_long>=3161)) {
		T[0]=19;T[1]=9;T[2]=14;T[3]=4;T[4]=0;T[5]=2;T[6]=5;T[7]=7;T[8]=12;T[9]=18;T[10]=16;T[11]=13;T[12]=17;T[13]=15;T[14]=3;T[15]=1;T[16]=6;T[17]=11;T[18]=8;T[19]=10;
	}
	else {
		T[0]=19;T[1]=9;T[2]=14;T[3]=4;T[4]=0;T[5]=2;T[6]=5;T[7]=7;T[8]=12;T[9]=18;T[10]=10;T[11]=8;T[12]=13;T[13]=17;T[14]=3;T[15]=1;T[16]=16;T[17]=6;T[18]=15;T[19]=11;
	}

	// calculem r
	for (i=0;i<M_Rows;i++) {
		r[T[i]]=q[i];
	}
	
	// calculem U
	for (i=0;i<M_Rows;i++) {
		for (j=0;j<p-1;j++) {
			U[i*M_Cols+j]=s[(j*r[i])%(p-1)];
			if (M_Cols==(p-1))
				U[i*M_Cols+j]-=1;
		}
	}
		
	if (M_Cols==p) {
		for (i=0;i<M_Rows;i++)
			U[i*M_Cols+p-1]=0;
	}
	else if (M_Cols==p+1) {
		for (i=0;i<M_Rows;i++) {
			U[i*M_Cols+p-1]=0;
			U[i*M_Cols+p]=p;
		}
		if (M_long==M_Cols*M_Rows) {
			aux=U[(M_Rows-1)*M_Cols+p];
			U[(M_Rows-1)*M_Cols+p]=U[(M_Rows-1)*M_Cols+0];
			U[(M_Rows-1)*M_Cols+0]=aux;
		}
	}
	

	// ----- CALCUL DE LES PERMUTACIONS I DESPERMUTACIONS

	per=&permute->PER[0];
	desper=&permute->DESPER[0];
	
	k=0;
	for (j=0;j<M_Cols;j++) {
		for (i=0;i<M_Rows;i++) {
			kp=T[i]*M_Cols+U[i*M_Cols+j];
			if (kp<M_long) {
				desper[kp]=k;
				per[k]=kp;
				k++;
			}
		}
	}
	
	return (int) per[0];

}
	
int mcd(int x, int y) {
	int r=1;
	
	while(r) {
		r=x%y;
		x=y;
		y=r;
	}
	return x;
}
