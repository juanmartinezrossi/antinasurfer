/*
 * phal_cm_wrapper.c
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

#include "cm_ifc.h"
#include "phal_sw_api.h"

int status = 0;

/*Global control functions*/
int TestBedStatus()
{
    if (status == PHAL_STATUS_INIT) {
        Relinquish(1);
    }
    status = Status();

    return status;
}

int InitCommManager(int module_id)
{
    return InitPHAL();
}

int InitCommManagerN(int module_id, const char *name)
{
    return InitPHAL();
}

int CloseCommManager(int module_id)
{
    ClosePHAL();
    return 1;
}

int GetTimeStamp()
{
    return GetTstamp();
}

/*Message/Packet passing functions*/
int CreateFlow(int module_id, int remote_module, struct flow_config fconf)
{
    /*if (strstr(fconf.name,"_rx")) {
            mode = FLOW_READ_ONLY;
    } else {
            mode = FLOW_WRITE_ONLY;
    }
     */
    return CreateItf((char*) fconf.name, fconf.rw);
}

int WriteFlow(int flow_id, unsigned int nof_bytes, void *buffer)
{
    return WriteItf(flow_id, buffer, nof_bytes);
}

int ReadFlow(int flow_id, unsigned int nof_bytes, void *buffer)
{
    return ReadItf(flow_id, buffer, nof_bytes);
}

int GetFlowStatus(int flow_id)
{
    return 1;
}

int CheckFlow(int flow_id)
{
    return 1;
}

int DeleteFlow(int flow_id)
{
    return CloseItf(flow_id);
}

/*Initialisation files reading*/
int ReadInitFile()
{
    return InitParamFile();
}

int GetParameter(int file_id, char *param_name, void *param)
{
    /** parameter type and size ??? */
    return GetParam(param_name, param, 0, 0);
}

int GetParameterMid(int file_id, char *pname, void *param, int mid)
{
    return 1;
}

int WriteMsg(int log_id, int dest, char *short_msg_desc, char *long_msg_desc)
{
    return 1;
}

int WriteVar(int log_id, char *var_name, char *var_value)
{
    return 1;
}

int LogStat(int log_id, int stat_id, int idx)
{
    return 1;
}

int GetNameMid(int mid, char *name, int buffer_size)
{
    return 1;
}

/*Statistics Control*/
int InitStatistics(int module_id)
{
    return 1;
}

int SetStatValue(int id, void *value, int idx)
{
    return SetStatsValue(id, value, 0);
}

int GetStatValue(int id, void *value, int idx)
{
    return SetStatsValue(id, value, 0);
}

int CreateStat(char *name)
{
    return InitStat(name, STAT_TYPE_INT, 1);
}

int SetStatLabel(int vect_id, char *label, int idx)
{
    return 1;
}

int GetStatIndex(int trow_id, char *element_name)
{
    return 1;
}

int GetStatId(int trow_id, char *element_name)
{
    return 1;
}

int DeleteStat(int id)
{
    return CloseStat(id);
}

int CloseStatistics(int module_id)
{
    return 1;
}

void UnrecoveryError(char *msg)
{
    printf("%s",msg);
    exit(-1);
}

