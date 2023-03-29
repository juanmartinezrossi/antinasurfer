/*
 * statsman_cmds.h
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
/** @defgroup cmds_statsman STATS MANAGER Input Commands
 *
 * STATS MANAGER input commands description.
 * @ingroup commands
 * @{
 */

/** @defgroup cmds_statsman_init Stat/Parameters Initialization Request
 *
 * STATS Sensor issues a request to register a statistic variable value (so it can 
 * be viewed/modified by the user through CMDMAN or to initialize the parameters
 * configuration file (where values are hold, it must be done prior to requesting the value) .
 *
 * @{
 */

#define STATSMAN_STINITOBJECT 20

/** STAT INIT Command Constant
 */
#define STATSMAN_STINIT		1

/** PARAMETERS INIT Command Constant
 */
#define STATSMAN_PMINIT		2

/** LOG INIT Command Constant
 */
#define STATSMAN_LOGINIT	3

#define STATSMAN_STATLS		4

int statsman_incmd_init(cmd_t *cmd);
int statsman_incmd_statls(cmd_t *cmd);

/** @} */


/** @defgroup cmds_statsman_close Stat/Parameters Close Request
 *
 * STATS Sensor issues a command requesting to unregister an statistic variable (it can not be
 * used anymore and its resources are freed) or to close a parameter's configuration file (so any
 * more values will be able to be obtained) 
 * @{
 */

/* STAT CLOSE Command Constant
*/
#define STATSMAN_PMCLOSE	5

/** PARAMETERS CLOSE Command Constant 
 */
#define STATSMAN_STCLOSE	6

/** LOG CLOSE Command Constant 
 */
#define STATSMAN_LOGCLOSE	7

int statsman_incmd_close(cmd_t *cmd);

/** @} */


/** @defgroup cmds_statsman_stact Stat Value Modify/View User Request 
 *
 * User, through the CMDMAN daemon, requests to view or modiffy a variable
 * previously registered by the Object. 
 * @{
 */ 

/** STAT GET Command Request 
 */
#define STATSMAN_STGET		8
/** STAT SET Command Request 
 */
#define STATSMAN_STSET		9
/** Stat report Command request
 */
#define STATSMAN_STREPORT	10
int statsman_incmd_stact(cmd_t *cmd);

/** @} */




/** @defgroup cmds_statsman_stactack Stat Action Acknowledgment
 *
 * STATS Sensor reports the result of an Stat Action Request telling if the 
 * modification or the getting of the variable was successfull or not. In case
 * that the action was GET and the result is success, the value of the variable
 * comes with the command
 *
 * @{
 */
 

/** SET OK Command Constant 
 */
#define STATSMAN_STSETOK	11
/** SET ERROR Command Constant 
 */
#define STATSMAN_STSETERR	12
/** GET OK Command Constant 
 */
#define STATSMAN_STVALOK	13
/** GET ERROR Command Constant 
 */
#define STATSMAN_STVALERR	14

/** REPORT OK Command Constant 
 */
#define STATSMAN_STREPORTOK	15
/** REPORT ERROR Command Constant 
 */
#define STATSMAN_STREPORTERR	16

int statsman_incmd_stactack(cmd_t *cmd);

/** @} */


/** @defgroup cmds_statsman_pmget Parameter Value Request
 *
 * STATS Sensor reports that an object is requesting a parameter value. 
 *
 * @{
 */

/** PARAMETER REQUEST Command Constant
 */
#define STATSMAN_PMGET		17

int statsman_incmd_pmget(cmd_t *cmd);

/**@} */


/** @defgroup cmds_statsman_logwrite Write to log
 *
 * STATS Sensor reports contents to write to the log 
 *
 * @{
 */

/** LOG WRITE Command Constant
 */
#define STATSMAN_LOGWRITE	18

int statsman_incmd_logwrite(cmd_t *cmd);

/** @} */

/** @defgroup cmds_statsman_streportvalue Write stat value to log
 *
 * STATS Sensor reports contents to write to the log
 *
 * @{
 */

/** LOG STAT Command Constant
 */
#define STATSMAN_STREPORTVALUE	19

int statsman_incmd_streportvalue(cmd_t *cmd);

/** @} */

