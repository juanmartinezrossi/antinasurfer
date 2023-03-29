/*
 * net_utils.c
 *
 * Copyright (c) 2009 Xavier Reves, UPC <xavier.reves at tsc.upc.edu>. All rights reserved.
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


#include <stdio.h>
#include <complex.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "time_utils.h"

#define CLOCK_MODE _POSIX_MONOTONIC_CLOCK

int get_time_r(mytime_t *x) {

	/*if (clock_gettime(CLOCK_MONOTONIC,x)) {*/
	if (clock_gettime(CLOCK_MODE,x)) {
		return -1;
	}
	return 0;
}


/*struct timespec tdata[3];	*/
mytime_t tdata[3];	

void get_time_interval_r(mytime_t * tdata) {

	tdata[0].tv_sec = tdata[2].tv_sec - tdata[1].tv_sec;
	tdata[0].tv_nsec = tdata[2].tv_nsec - tdata[1].tv_nsec;
	if (tdata[0].tv_nsec < 0) {
		tdata[0].tv_sec--;
		tdata[0].tv_nsec += 1000000000;
	}
}
