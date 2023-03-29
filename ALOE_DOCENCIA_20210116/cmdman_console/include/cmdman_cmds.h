/*
 * cmdman_cmds.h
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>. All rights reserved.
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
#ifndef CMDMAN_CMDS_H_
#define CMDMAN_CMDS_H_

#define ERROR_PARAMS	-100
#define PARAM_EXEC	-200

int _loadapp(char *x, int answer, strdata_o strdata, char do_print);
int _runapp(char *x, int answer, strdata_o strdata, char do_print);
int _pauseapp(char *x, int answer, strdata_o strdata, char do_print);
int _initapp(char *x, int answer, strdata_o strdata, char do_print);
int _stepapp(char *x, int answer, strdata_o strdata, char do_print);
int _stopapp(char *x, int answer, strdata_o strdata, char do_print);
int _statlist(char *x, int answer, strdata_o strdata, char do_print);
int _statset(char *x, int answer, strdata_o strdata, char do_print);
int _statget(char *x, int answer, strdata_o strdata, char do_print);
int _statreport(char *x, int answer, strdata_o strdata, char do_print);
int _appinfo(char *x, int answer, strdata_o strdata, char do_print);
int _applist(char *x, int answer, strdata_o strdata, char do_print);
int _pelist(char *x, int answer, strdata_o strdata, char do_print);
int _execreport(char *x, int answer, strdata_o strdata, char do_print);
int _execlog(char *x, int answer, strdata_o strdata, char do_print);
int _execinfo(char *x, int answer, strdata_o strdata, char do_print);


#endif /*CMDMAN_CMDS_H_*/
