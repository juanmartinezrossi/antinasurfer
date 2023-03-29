/*
 * cmdman_cmds.h
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

/** Constants for commands receiving 
 *
 */

/* from statsmanager */
#define CMDMAN_STSETOK		1
#define CMDMAN_STSETERR		2
#define CMDMAN_STVALOK		3
#define CMDMAN_STVALERR		4

#define CMDMAN_STREPORTOK	19
#define CMDMAN_STREPORTERR	20

/* from swmanager */
#define CMDMAN_SWLOADAPPOK	11
#define CMDMAN_SWLOADAPPERR	12
#define CMDMAN_SWSTATUSOK	13
#define CMDMAN_SWSTATUSERR	14

#define CMDMAN_APPLSOK		15
#define CMDMAN_APPLSERR		16

#define CMDMAN_STATSLSOK	17
#define CMDMAN_STATSLSERR	18

#define CMDMAN_APPINFOOK	5
#define CMDMAN_APPINFOERR	6

#define CMDMAN_REPORTOK		7
#define CMDMAN_REPORTERR	8
#define CMDMAN_LOGSOK		7
#define CMDMAN_LOGSERR		8

#define CMDMAN_LISTCPUOK	21

