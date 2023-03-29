/*
 * stats_cmds.h
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
/** @defgroup cmds_stats STATS Input Commands
 *
 * STATS Sensor input commands description.
 * @ingroup commands
 * @{
 */

/** @defgroup cmds_stats_stact Stat Value Action
 *
 * Action Request command to STATS Sensor (Get or Modify a variable value).
 *
 * @{
 */

/** SET Command Constant
 */
#define STATS_STSET			1
/** GET Command Constant
 */
#define STATS_STGET			2

#define STATS_STREPORT			11
int stats_incmd_stact(cmd_t *cmd);


/** @} */ /* end cmds_stats_stact */


/** @defgroup cmds_stats_initack Stat/Parameters Initialization Acknowledgement 
 *
 * This command is issued when the initialization of an statistic variable
 * or the parameters file has been successfully realized by the STATS MANAGER.
 *
 * @ingroup cmds_stats
 * @{
 */


/** STAT INIT OK Command Constant
 */
#define STATS_STINITOK		3

/** STAT INIT ERROR Command Constant
 */
#define STATS_STINITERR		4

/** PARAMETERS FILE INIT OK Command Constant 
 */
#define STATS_PMINITOK		5

/** PARAMETERS FILE INIT ERROR Command Constant 
 */
#define STATS_PMINITERR	 	6

/** LOG INIT OK Command Constant 
 */
#define STATS_LOGINITOK		9

/** LOG INIT ERROR Command Constant 
 */
#define STATS_LOGINITERR	10


int stats_incmd_initack(cmd_t *cmd);

/** @} */


/** @defgroup stats_cmds_pmval Parameter Value Request Answer
 *
 * This command carries the value of a requested initialization parameter.
 *
 * @ingroup cmds_stats
 *
 * @{
 */

/** PARAMETER VALUE OK Command Constant
 */
#define STATS_PMVALOK		7

/** PARAMETER VALUE ERROR Command Constant
 */
#define STATS_PMVALERR		8



int stats_incmd_pmval(cmd_t *cmd);

/** @} */

/** @} */







