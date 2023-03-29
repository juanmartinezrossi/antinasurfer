/*
 * sync.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>,
 *                    Xavier Reves, UPC <xavier.reves at tsc.upc.edu>
 * All rights reserved.
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
#define MAX_CHILD 5

#define SYNC_KEY 			0xAFBFCFDF
#define SYNC_CONTINUOUS 	0xCFDF0000
#define DEFAULT_PERIOD 1000
#define MIN_PERIOD 500
#define MIN_ROUND_TRIP 100

#define DEFAULT_ROUND_TRIP 1000

#define OFFSET_TH 100

struct syncdata_tx {
	unsigned int key;
};
struct syncdata_rx	 {
	int new_tstamp;
	int next_ts;
	time_t ref_time;
};

struct syncdata_cont {
	int tstamp;
	int tslot_len;
};

struct sync_register {
	int cmd;
	hwitf_t itf_id;
	peid_t pe_id;
	int plat_family;
	int tslen_usec;
	int period;
	int round_trip_limit;
};

#define SYNC_REGISTER_MAGIC	0x10203040
#define SYNC_REGISTER_OK	0x0000FFFF
#define SYNC_REGISTER_ERROR	0xFFFF0000

#define EXT_ITF_TYPE_MASK 			0xF0
#define EXT_ITF_TYPE_SYNC_CLIENT	0x20

#define EXT_ITF_TYPE_SYNC_MASTER	0x2
