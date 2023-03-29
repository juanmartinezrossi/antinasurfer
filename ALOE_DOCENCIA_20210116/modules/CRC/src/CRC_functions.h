/* 
 * Copyright (c) 2012.
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

#ifndef CRC_
#define CRC_

#define LTE_CRC24A	0x1864CFB
#define LTE_CRC24B	0X1800063
#define LTE_CRC16	0x11021
#define LTE_CRC8	0x19B


#define MAX_LENGTH	1024*16

typedef struct {
	unsigned long table[256];
	unsigned char data0[MAX_LENGTH];
	int polynom;
	int order;
	unsigned long crcinit;
	unsigned long crcxor;
	unsigned long crcmask;
	unsigned long crchighbit;
	unsigned int crc_out;
} crc_t;

unsigned int crc(unsigned int crc, char *bufptr, int len,
		int long_crc, unsigned int poly, int paste_word);

void crc_free(crc_t *crc_p);
int crc_init(crc_t *crc_par, unsigned int crc_poly, int crc_order);
int crc_set_init(crc_t *crc_par, unsigned long crc_init_value);
int crc_set_xor(crc_t *crc_par, unsigned long crc_xor_value);
unsigned int crc_attach(char *bufptr, int len, crc_t *crc_params);
void pack_bits(unsigned int value, char **bits, int nof_bits);
unsigned int unpack_bits(char **bits, int nof_bits);

int add_CRC(char *bufptr, int len, crc_t *crc_params);
int check_CRC(char *bufptr, int datalen, crc_t *crc_params);


#endif
