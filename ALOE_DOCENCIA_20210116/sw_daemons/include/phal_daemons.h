/*
 * phal_daemons.h
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

/** @defgroup constants ALOE Common Constants 
 *
 * @ingroup common
 *
 * Common constants used by several Daemons 
 * @{
 */ 
  
/** @defgroup fields Packet Fields
 *
 * Use this constants to define fields that other daemons can understand
 * @{
 */ 
#define FIELD_OBJNAME	1
#define FIELD_APPNAME	2
#define FIELD_OBJID	3
  
#define FIELD_STAT	4
#define FIELD_STNAME	5
#define FIELD_STID	6
#define FIELD_STTYPE	7
#define FIELD_STSIZE	8
#define FIELD_STVALUE	9
  
#define FIELD_PARAM	10
#define FIELD_PMNAME	11
#define FIELD_PMID	12
#define FIELD_PMTYPE	13
#define FIELD_PMSIZE	14
#define FIELD_PMVALUE	15
  
#define FIELD_NOFOBJECTS 16

#define FIELD_STATUS	17
#define FIELD_APPID	18
  
#define FIELD_EXENAME	19
#define FIELD_PLATID	10
  
#define FIELD_PINFO	21
#define FIELD_MEMINFO	22
#define FIELD_BINDATA	23
#define FIELD_CHKSUM	24

#define FIELD_APP	25

#define FIELD_EXEOBJ	26
#define FIELD_EXEINFO	27

#define FIELD_TSTAMP	28

#define FIELD_XITF	31
#define FIELD_PEID	32
#define FIELD_REQPEID	33
#define FIELD_PE	34

#define FIELD_DATAHEADER	40
#define FIELD_OBJITFID		41
#define FIELD_OBJITFMODE	42
#define FIELD_OBJXITFID		43

#define FIELD_LOGID	44
#define FIELD_LOGNAME	45
#define FIELD_LOGTXT	46

#define FIELD_STATLIST  47

#define FIELD_STREPORTACT 48
#define FIELD_STREPORTPERIOD 49
#define FIELD_STREPORTWINDOW 50
#define FIELD_STREPORTTSVEC 51
#define FIELD_STREPORTVALUE 52
#define FIELD_STREPORTSZ    53

#define FIELD_PELIST		54

#define FIELD_TIME          55

#define FIELD_GCOST			56
#define FIELD_C				57

/** @} */ 
  
/** @defgroup daemons Daemon Code Constants
 * Use this constants when sending a packet to another daemon
 * @{
 */ 
#define DAEMON_FRONTEND		1
#define DAEMON_CMDMAN		2
#define DAEMON_STATS		3
#define DAEMON_STATSMAN		4
#define DAEMON_HWMAN		5
#define DAEMON_SWMAN		6
#define DAEMON_SWLOAD		7
#define DAEMON_EXEC			8
#define DAEMON_BRIDGE		9
#define DAEMON_MAX			10

	 
/** @} */ 
  
  
/** @defgroup exec_status ALOE Execution Status
 *
 * This constants define the objects Execution Status
 * @{
 */ 
#define PHAL_STATUS_0		0
#define PHAL_STATUS_STARTED	1
#define PHAL_STATUS_INIT	2
#define PHAL_STATUS_RUN		3
#define PHAL_STATUS_PAUSE	4
#define PHAL_STATUS_STEP	5
#define PHAL_STATUS_STOP	6
#define PHAL_STATUS_ABNSTOP	7
#define PHAL_STATUS_MAX		8
  
/** @} */ 

/** @defgroup others Other Constants
 *
 * @{
 */
#define FLOW_READ_ONLY		1
#define FLOW_WRITE_ONLY		2
#define FLOW_READ_WRITE		3

#define PARAM_CHAR       10
#define PARAM_UCHAR  11
#define PARAM_SHORT  20
#define PARAM_USHORT 21
#define PARAM_INT        30
#define PARAM_UINT   31
#define PARAM_HEX        32
#define PARAM_FLOAT  4
#define PARAM_DOUBLE 5
#define PARAM_STRING 6
#define PARAM_BOOL   7

/** @} */
  
/** @} */ 
  
/* to erase */ 
#define xprintf printf
#include <stdio.h>
  
