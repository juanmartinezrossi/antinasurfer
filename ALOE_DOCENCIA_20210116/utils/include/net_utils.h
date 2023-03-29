/*
 * net_utils.h
 *
 * Copyright (c) 2009 Xavier Reves, UPC <xavier.reves at tsc.upc.edu>. All rights reserved.
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
#ifndef NET_UTILS_H_
#define NET_UTILS_H_

int setup_client(unsigned long int ip, int port, int type);
int setup_server(unsigned long int ip, int port, int *sck_fd, int type);


#endif /*NET_UTILS_H_*/
