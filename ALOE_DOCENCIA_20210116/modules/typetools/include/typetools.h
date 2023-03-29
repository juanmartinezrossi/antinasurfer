#ifndef _TYPETOOLS_H
#define	_TYPETOOLS_H

#define TYPE_BITSTREAM  0x00    /** 8 bits: default type char*/
#define TYPE_CHAR       0x01
#define TYPE_SHORT      0x02
#define TYPE_INT        0x03
#define TYPE_FLOAT      0x04
#define TYPE_BIT8       0x11   /** A byte containing the info related to 1 bit*/

#define TYPE_BITSTREAM  0x00
#define TYPE_INT8       0x01
#define TYPE_INT16      0x02
#define TYPE_INT32      0x03
#define TYPE_FLOAT      0x04
#define TYPE_BIT8       0x11   /** A byte containing the info related to 1 bit*/
#define TYPE_COMPLEX		0x07
#define TYPE_CUSTOM			0x20


#define DIV32(a) (((a-1)>>5)+1)
#define DIV8(a) (((a-1)>>3)+1)
#define DIV4(a) (((a-1)>>2)+1)
#define DIVINT(a,b) (((a-1)/b)+1)

int convii(int *filter, int *state, int length, int din);
short convss(short *filter, short *state, int length, short din);
float convff(float *filter, float *state, int length, float din);

/** bin2char
 * Converts a char-aligned bitstream to an array of bytes where each
 * bit uses a byte
 */
void bin2char (char *input, char *output, int length);

/** bin2int8
 * Converts a char-aligned bitstream to an array of integers where each
 * bit uses an integer
 */
void bin2int8 (char *input, int *output, int length);

/** bin2float
 * Converts a char-aligned bitstream to an array of floats where each
 * bit uses an float
 */
void bin2float (char *input, float *output, int length);

/** char2bin
 * Converts an array of bytes where each bit uses an byte to a
 * char-algined bitstream
 */
void char2bin (char *input, char *output, int length);

/** int82bin
 * Converts an array of integers where each bit uses an integer to a
 * char-algined bitstream
 */
void int82bin (int *input, char *output, int length);

/** float2bin
 * Converts an array of floats where each bit uses a float to a
 * char-algined bitstream
 */
void float2bin (float *input, char *output, int length);

/** type2int
 * Converts an arbitrary type to an array of integers
 */
void type2int(void *input, int *output, int len, int datatype);

/** type2int
 * Converts an arbitrary type to an array of integers of maximum value maxint
 */

void type2intAUTOSCALE(void *input, int *output, int len, int datatype, int maxint);

/** int2type
 * Converts an array of integers to an array of arbitrary type
 */
void int2type(int *input, void *output, int len, int datatype);

/** float2type
 * Converts an array of floats to an array of arbitrary type
 */
void float2type(float *input, void *output, int len, int datatype);

/** float2type
 * Converts an array of arbitrary tupe to an array of floats
 */
void type2float(void *input, float *output, int len, int datatype);

/** typeSizeArray
 * Returns the amount of bytes needed to store an array of nsamples
 * of arbitrary type
 */
int typeSizeArray(int datatype, int nsamples);

/** convert from typetools type to STATS type */
int type2stats(int datatype);

/** typeSize
 * Returns the amount of bytes needed to store a sample
 * of arbitrary type
 */
int typeSize(int datatype);

/** typeNsamplesArray
 * Returns the amount of samples in an array
*/
int typeNsamplesArray(int datatype, int size);

/** typet2type
 *  Converts an array of arbitrary type to an array of abritrary type
 *  MAXvalue indicate the output max value. A rescaling process is done.
 *  Returns de number of bits/samples
 */
int type2type(void *input, void *output,\
                int typeIN, int typeOUT,\
                int dataINlength, float MAXvalue);
		
/*First binari (int) to MSB bit in char*/
int MSBfirstbin2char (int *input, unsigned char *output, int length);

/*MSB bit first binari output*/
int MSBfirstchar2bin (unsigned char *input, int *output, int length);





#endif
