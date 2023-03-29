/*
 * cmdman_prints.h
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
#ifndef CMDMAN_PRINTS_H_
#define CMDMAN_PRINTS_H_


int proc_print(int fd, struct proc_info *procs, int nof_procs);
int apps_print(int fd, struct app_info *apps, int nof_apps);
int pe_print(int fd, struct pe_info *pe, int nof_pe);
int stats_print(int fd, struct stat_info *stats, int nof_stats);



#endif /*CMDMAN_PRINTS_H_*/
