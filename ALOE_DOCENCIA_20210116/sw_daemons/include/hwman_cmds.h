/*
 * hwman_cmds.h
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

#ifndef HWMAN_CMDS_H_
#define HWMAN_CMDS_H_

#define CHECK_PROCESSORS_PERIOD		2000
#define CHECK_PROCESSORS_TIMEOUT	10000

#define HWMAN_ADDCPU	10
#define HWMAN_ADDITF	11
#define HWMAN_MAPOBJ	12
#define HWMAN_UNMAPOBJ	13
#define HWMAN_UPDCPU	14
#define HWMAN_LISTCPU	15

int hwman_incmd_listcpu(cmd_t *cmd);
int hwman_incmd_swmapper(cmd_t *cmd);
int hwman_incmd_swunmap(cmd_t *cmd);
int hwman_incmd_addcpu (cmd_t *cmd);
int hwman_incmd_updcpu (cmd_t *cmd);
int hwman_incmd_additf (cmd_t *cmd);


#endif /*HWMAN_CMDS_H_*/
