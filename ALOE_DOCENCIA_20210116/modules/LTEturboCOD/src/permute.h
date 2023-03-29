#ifndef _PERMUTE_H
#define _PERMUTE_H

#define TURBO_RATE 3

#define MAX_ROWS 20
#define MAX_COLS 256

#define SIZE_U 10240 	//tamany maxim en bytes que pot ocupar U
#define SIZE_PER 20456  //tamany en bytes del vector permutacions (5114*4bytes)

/* Permutation/Interleaving option*/
#define PER_UMTS 0
#define PER_LTE	1

typedef int Tper; // tipus array de permutacions (TurboCodi)

struct permute_t {
	Tper *PER; 		// array de permutacions
	Tper *DESPER;	// array de despermutacions
};

/* LTE valid Interleaver Sizes */
#define NUM_LTE_SIZES 188

int mcd(int x,int y);
int ComputePermutation(struct permute_t *permute, int Long_CodeBlock, int type);
int ComputeLTEPermutation(struct permute_t *permute, int Long_CodeBlock);
int ComputeUMTSPermutation(struct permute_t *permute, int Long_CodeBlock);
int getLTEcbsize (int numbits);

#endif // _PERMUTE_H
