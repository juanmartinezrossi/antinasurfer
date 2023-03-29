/*
 * phal_hw_api_man.h
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


#ifdef __cplusplus
extern "C" {
#endif


int hwapi_res_parseall_(char *res_name, char **buffer,int absolute);
int hwapi_res_parseall (char *res_name, char **buffer);
int hwapi_res_open(char *res_name, int mode, int *file_len);
int hwapi_res_close(int fd);
int hwapi_res_read(int fd, char *buffer, int blen);
int hwapi_res_write(int fd, char *buffer, int blen);

#define TI_LINKER_PATH "/usr/local/bin/lnk6x.exe"
#define WINE_PATH "/usr/bin/wine"

int hwapi_comp_relocate(char *res_name, char *output_res_name,
        unsigned int new_address, unsigned int new_len);

#ifdef __cplusplus
}
#endif

