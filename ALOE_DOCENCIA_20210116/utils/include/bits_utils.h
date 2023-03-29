/*
 * print_utils.h
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
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void bytes2bits(char *in, char *out, int inlength);
int bits2bytes(char *in, char *out, int inlength);
void bytes2bitsfloat(char *in, float *out, int inlength);




