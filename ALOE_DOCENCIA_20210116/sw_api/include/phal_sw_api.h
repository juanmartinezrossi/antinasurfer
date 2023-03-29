/*
 * phal_sw_api.h
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define xprintf printf

/** @defgroup exec_status ALOE Execution Status
 *
 * This constants define the objects Execution Status
 * @{
 */
#define PHAL_STATUS_INIT	2
#define PHAL_STATUS_RUN		3
#define PHAL_STATUS_PAUSE	4
#define PHAL_STATUS_STOP	6

/** @} */

/** @ingroup interfaces
 *
 * @{
 */
#define FLOW_READ_ONLY		1
#define FLOW_WRITE_ONLY		2
#define FLOW_READ_WRITE		3

/** @} */

#define STAT_TYPE_CHAR		1
#define STAT_TYPE_UCHAR		2
#define STAT_TYPE_FLOAT		3
#define STAT_TYPE_INT			4
#define STAT_TYPE_SHORT		5
#define STAT_TYPE_TEXT		6
#define STAT_TYPE_STRING	6
#define STAT_TYPE_COMPLEX 	7



/* ========================================
              ALOE SW-API FUNCTIONS
   ========================================
 */


int InitPHAL();
void ClosePHAL();
void Relinquish(int nslots);
int Status(void);
char *GetObjectName();
char *GetAppName();

int CreateItf(char *name, int mode);
int CloseItf(int fd);
int ReadItf(int fd, void *buffer, int size);
int WriteItf(int fd, void *buffer, int size);
int GetItfStatus(int fd);

int GetTstamp(void);
unsigned int GetTempo(float freq);

int InitParamFile(void);
int GetParam(char *ParamName, void *ParamValue, int ParamType, int ParamLen);
int CloseParamFile(void);

int InitStat(char *StatName, int StatType, int StatSize);
int InitObjectStat(char *ObjectName, char *StatName, int StatType, int StatSize);
int CloseStat(int StatId);
int CloseObjectStat(int StatId);
int SetStatsValue(int StatId, void * Value, int Size);
int GetStatsValue(int StatId, void * Buffer, int BuffByteSize);
int SetObjectStatsValue(int StatId, void * Value, int Size);

int InitCounter(char *name);
void StartCounter(int CounterId);
void StopCounter(int CounterId);

int CreateLog();
int CloseLog(int LogId);
int WriteLog(int LogId, char *str);

/*/////GLOBAL VARS////////////*/
void ListPublicStats(int nof_listedstats);
int GetPublicStatID(char *app_name, char *obj_name, char *StatName);
int CreateLocalStat(char *StatName, int StatType, int StatSize);
int CreateRemotePublicStat(char *app_name, char *ObjectName,char *StatName, int StatType, int datalength);
int SetPublicStatsValue(int StatId, void * Value, int length);
int GetPublicStatsValue(int StatId, void * Buffer, int BuffElemSize);


