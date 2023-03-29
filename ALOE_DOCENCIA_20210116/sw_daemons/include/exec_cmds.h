/*
 * exec_cmds.h
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


/** @defgroup cmds_exec EXEC Input Commands
 *
 * @ingroup commands
 *
 * @{
 */

/** @defgroup cmds_exec_setstatus Change Object Status
 * 
 * Change the execution status of an object
 *
 * @{
 */

/** Command Constant
 */
#define EXEC_SETSTATUS	1

/** Processing Function
 *
 */
int exec_incmd_setstatus(cmd_t *cmd);

/** @} */ /* end cmds_exec_setstatus */

/** @defgroup cmds_exec_report Start/Stop exec info reports
 * 
 * @{
 */

/** Command Constant
 */
#define EXEC_REPORTSTART	2
#define EXEC_REPORTSTOP		3

/** Processing Function
 *
 */
int exec_incmd_reportcmd(cmd_t *cmd);

/** @} */ /* end cmds_exec_reports */

/** @} */ /* end cmds_exec */

