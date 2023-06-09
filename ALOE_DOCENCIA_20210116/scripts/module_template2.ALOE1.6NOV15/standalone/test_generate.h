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

#ifndef _TEST_GEN_H
#define _TEST_GEN_H

#include "params.h"

//test_gen_t test_ctrl;// = {1, 1024, "DEFAULT", "DEFAULT", 1.0};	//"module_template2",

int generate_input_signal(input_t *input, int *lengths);

//int generate_input_signal(input_t *input, int *lengths, test_gen_t *test_ctrl);

#endif
