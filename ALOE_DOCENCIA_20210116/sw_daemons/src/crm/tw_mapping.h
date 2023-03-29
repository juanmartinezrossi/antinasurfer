/*
 * tw_mapping.h
 *
 * Copyright (c) 2009 Vuk Marojevic, UPC <marojevic at tsc.upc.edu>. All rights reserved.
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

/** tw-mapping's principal function prototypes **/

// FUNCTION PROTOTYPES
// ordering functions
void sort_d(float m[]);
void sort_b(float m[]);

// mapping algorithm, which calls the cost function
float tw_mapping(int (*ptr_nodes)[Mmax], int w);
//float tw_mapping(int **ptr_nodes, int w);
void resource_trans(void);

