/*
 * swman_parser.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "phal_hw_api.h"
#include "phal_hw_api_man.h"

#include "phid.h"
#include "phal_daemons.h"

#include "swman.h"

#include "set.h"
#include "str.h"
#include "var.h"
#include "app.h"
#include "rtcobj.h"
#include "phobj.h"
#include "phitf.h"
#include "pkt.h"
#include "daemon.h"

#include "exec.h"
#include "swman.h"
#include "swman_parser.h"

#include "cfg_parser.h"
#include "swman_platforms.h"

char apppath[256];

char buffer[1024];

/** Function that opens the file, reads to memory
 * and calls parsing function
 */
int swman_parser_app(char *app_name, Set_o objects)
{
	int i, j, offset;
	char *buffer;
	cfg_o cfg;
	sect_o subsect, sect;
	key_o obj_name, exe_name, itf_name, remote_itf_name,
			remote_obj_name, proc_req, bw_req, force_pe, delay;
	phobj_o obj;
	phitf_o itf;

	assert (app_name && objects);

	/*AGBJuly15	sprintf(apppath, "%s/%s.%s", APPS_CFG_DIR, app_name, APPS_CFG_EXT);*/
	sprintf(apppath, "%s.%s", app_name, APPS_CFG_EXT);
/*	printf("swman_parser_app(): APPS_CFG_DIR=%s, app_name=%s, APPS_CFG_EXT=%s\n", APPS_CFG_DIR, app_name, APPS_CFG_EXT);
*/
	/*AGBJuly15	offset = hwapi_res_parseall(apppath, &buffer);*/
	offset = hwapi_res_parseall_(apppath, &buffer, 1);
	if (offset < 0) {
		xprintf ("SWMAN: Error reading file %s\n", apppath);
		return 0;
	}

	cfg = cfg_new(buffer, offset);
	if (!cfg) {
		xprintf ("SWMAN: Error parsing file %s\n", apppath);
		free(buffer);
		return 0;
	}

	for (i = 0; i < Set_length(cfg_sections(cfg)); i++) {
		sect = Set_get(cfg_sections(cfg), i);
		assert(sect);
		if (strcmp(sect_title(sect), "object")) {
			xprintf ("SWMAN: Error unknown section %s\n", sect_title (sect));
			break;
		}

		obj_name = Set_find(sect_keys(sect), "obj_name", key_findname);
		exe_name = Set_find(sect_keys(sect), "exe_name", key_findname);
		proc_req = Set_find(sect_keys(sect), "kopts", key_findname);
		force_pe = Set_find(sect_keys(sect), "force_pe", key_findname);
		
		if (obj_name && exe_name) {
			obj = phobj_new();
			if (!obj) {
				xprintf ("SWMAN: Error creating object\n");
				break;
			}
			str_delete(&obj->objname);
			obj->objname=str_dup(obj_name->pvalue);
			str_delete(&obj->exename);
			obj->exename=str_dup(exe_name->pvalue);
			if (force_pe) {
				key_value(force_pe,0,PARAM_INT,&obj->force_pe);
			} else {
				obj->force_pe=-1;
			}

			if (proc_req) {
				key_value(proc_req, 0, PARAM_FLOAT, &obj->rtc->mops_req);
			} 
			
			Set_put(objects, obj);
		}

		for (j = 0; j < Set_length(sect_subsects(sect)); j++) {

			subsect = Set_get(sect_subsects(sect), j);
			assert(subsect);

			itf_name = Set_find(sect_keys(subsect), "name",
					key_findname);
			remote_itf_name = Set_find(sect_keys(subsect),
					"remote_itf", key_findname);
			remote_obj_name = Set_find(sect_keys(subsect),
					"remote_obj", key_findname);
				
			bw_req = Set_find(sect_keys(subsect), "kbpts", key_findname);
			delay = Set_find(sect_keys(subsect), "delay", key_findname);

			if (itf_name && remote_itf_name && remote_obj_name) {

				if (!strcmp(sect_title(subsect), "inputs")) {
					itf = phitf_new(FLOW_READ_ONLY);

				} else if (!strcmp(sect_title(subsect),
						"outputs")) {
					itf = phitf_new(FLOW_WRITE_ONLY);
				} else {
					xprintf ("SWMAN: Error unknown sub-section %s\n", sect_title(subsect));
					break;
				}
				
				if (!itf) {
					xprintf ("SWMAN: Error creating itf\n");
					break;
				}

				str_delete(&itf->name);
				itf->name=str_dup(itf_name->pvalue);
				str_delete(&itf->remotename);
				itf->remotename=str_dup(remote_itf_name->pvalue);
				str_delete(&itf->remoteobjname);
				itf->remoteobjname=str_dup(remote_obj_name->pvalue);

				if (bw_req) {
					key_value(bw_req, 0, PARAM_FLOAT, &itf->bw_req);
				}
				if (delay) {
					key_value(delay, 0, PARAM_INT, &itf->delay);
				} else {
					itf->delay=1;
				}

				Set_put(obj->itfs, itf);
			}
		}

		if (j < Set_length(sect_subsects(sect))) {
			xprintf ("SWMAN: Caution some interfaces were not parsed.\n");
		}

	}

	free(buffer);

	if (i < Set_length(cfg_sections(cfg))) {
		xprintf ("SWMAN: Caution some objects were not parsed.\n");
	}

	cfg_delete(&cfg);

	return 1;
}


char platstr[16];

/** parses executable information
 */
int swman_parser_execs(Set_o execs)
{
	int i, j, k, offset;
	char *buffer;
	cfg_o cfg;
	sect_o subsect, sect;
	key_o exe_name, platform, memory;
	execinfo_o exec;
        execimp_o imp;
	
	assert (execs);

	/*AGBJul15	offset = hwapi_res_parseall(EXECINFO_PATH, &buffer);*/
	offset = hwapi_res_parseall_(EXECINFO_PATH, &buffer, 1);
	if (offset < 0) {
		xprintf ("SWMAN: Error reading file %s\n", EXECINFO_PATH);
		return 0;
	}

	cfg = cfg_new(buffer, offset);
	if (!cfg) {
		xprintf ("SWMAN: Error parsing file %s\n", apppath);
		free(buffer);
		return 0;
	}

	for (i = 0; i < Set_length(cfg_sections(cfg)); i++) {
		sect = Set_get(cfg_sections(cfg), i);
		assert(sect);

		if (strcmp(sect_title(sect), "executable")) {
			xprintf ("SWMAN: Error unknown section %s\n", sect_title (sect));
			break;
		}

		exe_name = Set_find(sect_keys(sect), "name", key_findname);
		
		if (exe_name) {
			exec = execinfo_new();
			if (!exec) {
				xprintf ("SWMAN: Error creating object\n");
				break;
			}
			str_delete(&exec->name);
			exec->name=str_dup(exe_name->pvalue);
			
			Set_put(execs, exec);
		}

		for (j = 0; j < Set_length(sect_subsects(sect)); j++) {

			subsect = Set_get(sect_subsects(sect), j);
			if (subsect) {
                        platform = Set_find(sect_keys(subsect), "platform",
					key_findname);
                        memory = Set_find(sect_keys(subsect), "memory",
					key_findname);

                        imp = execimp_new();
                        if (!imp) {
                            printf("SWMAN: Error creating object\n");
                            break;
                        }
                        
                        imp->name = exec->name;

                        if (!strcmp(sect_title(subsect), "binary")) {
                            imp->mode = BINARY;
                        } else if (!strcmp(sect_title(subsect), "source")) {
                            imp->mode = SOURCE;
                        } else {
                            printf("SWMAN: Error parsing exec info. Unknown mode %s\n",sect_title(subsect));
                            break;
                        }

                        if (platform) {
                            key_value(platform, 0, PARAM_STRING, platstr);
                            k=0;
                            while(k<NOF_PLATS && strcmp(platstr, platforms_cfg[k].name)) {
                                k++;
                            }
                            if (k<NOF_PLATS) {
                                imp->platform = platforms_cfg[k].plt_id;
                            } else {
                                printf("SWMAN: Error parsing exec info. Unknown platform %s\n",platstr);
                                imp->platform = -1;
                            }
                        }

                        if (memory) {
                            key_value(memory, 0, PARAM_HEX, &imp->pinfo.mem_sz);
                        }

                        Set_put(exec->versions, imp);
			}
		}
	}

	free(buffer);

	if (i < Set_length(cfg_sections(cfg))) {
		xprintf ("SWMAN: Caution some executables were not parsed.\n");
	}

	cfg_delete(&cfg);

	return 1;
}
