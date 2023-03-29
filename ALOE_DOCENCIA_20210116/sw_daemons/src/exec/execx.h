/*
 * exec.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
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
/** TIME SLOT LIMITS FOR EACH STATUS
*/ 
#define LOAD_LIMIT_TS	10000
#define INIT_LIMIT_TS	10000
#define RUN_LIMIT_TS	1000
#define PAUSE_LIMIT_TS	3
#define STOP_LIMIT_TS	10000
#define STEP_LIMIT_TS 	3

int exec_statusrep (struct hwapi_proc_i *obj, int ok);

