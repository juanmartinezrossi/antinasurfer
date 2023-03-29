/*
 * swman_cmds.h
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
/** @defgroup cmds_swman SW MANAGER Input Commands
 *
 * SW MANAGER Input commands description.
 * @ingroup commands
 * @{
 */

/** @defgroup cmds_swman_loadapp Load an Application
 *
 * This command is received by CMDMAN Daemon after User orders to load an application.
 * 
 * The application description (list of objects, associated executables, interfaces and their interconnection)
 * is described in the application configuration file (see swman_parser.c).
 *
 * This command, thus, will parse this file and build the list of objects that will be send downwards
 * in the hierarchy.
 *
 * @{
 */

/** LOAD APP Command Constant 
 */
#define SWMAN_LOADAPP		1

int swman_incmd_loadapp(cmd_t *cmd);


/** @} */

/** @defgroup cmds_swman_appstatus Change Application Status
 *
 * Change the Execution Status (INIT, RUN, PAUSE, STOP) of a previously loaded application.
 * 
 * @{
 */
 
/** APP STATUS Command Constant
 */
#define SWMAN_APPSTATUS		2

int swman_incmd_appstatus(cmd_t *cmd);


int swman_incmd_rtfault(cmd_t *cmd);

#define SWMAN_RTFAULT           16

/** @} */


/** @defgroup cmds_swman_execreq Executable Binary Request 
 *
 * SWLOAD Requests to download the executable binary contents.
 *
 * @{
 */

/** EXEC REQUEST Command Constant
 */
#define SWMAN_EXECREQ		3

int swman_incmd_execreq(cmd_t *cmd);

/** EXEC REQUEST INFO Command Constant
 */
#define SWMAN_EXECINFO		15

int swman_incmd_execinfo(cmd_t *cmd);

/** @} */


/** @defgroup cmds_swman_statusrep Object Status Report 
 *
 * EXEC Sensor reports of a successfully change in an object execution status within certain limits (see exec.h). In case
 * the object did not had time to change its status, or it crashed during that phase, STATUSERR command will be reported
 *
 * @{
 */

/** STATUS CHANGE OK Command Constant
 */
#define SWMAN_STATUSOK		4

/** STATUS ERROR Command Constant 
 */
#define SWMAN_STATUSERR		5

int swman_incmd_statusrep(cmd_t *cmd);


/** @} */

/** @} */

#define SWMAN_EXECERR		6
#define SWMAN_MAPAPPERROR	13
int swman_incmd_apperror(cmd_t *cmd);


/** @defgroup cmds_swman_appinfo Application information request 
 *
 * CMDMAN request to get application information.
 *
 * @{
 */

/** Application Info Command
 */
#define SWMAN_APPINFO		7

int swman_incmd_appinfo(cmd_t *cmd);

/** Application list 
 */
#define SWMAN_APPLS		14

int swman_incmd_applist(cmd_t *cmd);

#define SWMAN_REPORTSTART		8
#define SWMAN_REPORTSTOP		9
#define SWMAN_LOGSSTART			10
#define SWMAN_LOGSSTOP			11

int swman_incmd_appinfocmd(cmd_t *cmd);

/** @} */

/** @defgroup cmds_swman_appinfoack Application information Answer 
 *
 * EXEC answers with process information
 *
 * @{
 */

/** Application Info Command
 */
#define SWMAN_APPINFOREPORT	12
#define REPORT_INTERVAL	100
int swman_incmd_appinforeport(cmd_t *cmd);

/** @} */


