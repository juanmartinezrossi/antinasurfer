/*
 * swload_cmds.h
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
/** @defgroup cmds_swload SWLOAD Input Commands
 *
 * SWLOAD Sensor Input commands description.
 * @ingroup commands
 * @{
 */

/** @defgroup cmds_swload_loadobj Load a list of Objects 
 *
 * This command is received by SWLOAD when an order to load a list of objects
 * is received in the processor and no SWMAP Daemon is available. Thus, the processor 
 * must load all objects by itself. 
 *
 * At this point, SWLOAD will match internal interfaces so objects can exchange data
 * and request the executables may require.
 *
 * @{
 */

/** LOAD OBJECTS Command Constant
 */
#define SWLOAD_LOADOBJ		1

int swload_incmd_loadobj(cmd_t *cmd);

/** @} */


/** @defgroup cmds_swload_loading Downloading of an Object
 *
 * This group of commands involve the procedure of downloading an executable
 * and launching it (one instance for each object sharing the same exe_name).
 *
 * This procedure is divided in three steps corresponding to three different commands
 *	- LOADSTART (swload_incmd_loadstart): Begins the transmission of an executable
 *	- LOADING (swload_incmd_loading): Binary data of the executable divided in packets
 *	- LOADEND (swload_incmd_loadend): Packet indicating the end of the transmission (and the checksum) 
 *
 * @{
 */
 
/** LOADSTART Command Constant
 */
#define SWLOAD_LOADSTART	2
/** LOADING Command Constant
 */
#define SWLOAD_LOADING		3
/** LOADEND Command Constant
 */
#define SWLOAD_LOADEND		4

#define SWLOAD_EXECERR		5

int swload_incmd_loadstart(cmd_t *cmd);
int swload_incmd_loading(cmd_t *cmd);
int swload_incmd_loadend(cmd_t *cmd);
int swload_incmd_execerr(cmd_t *cmd);

#define SWLOAD_EXECINFO         6
int swload_incmd_execinfo(cmd_t *cmd);

/* @} */

/* @} */


