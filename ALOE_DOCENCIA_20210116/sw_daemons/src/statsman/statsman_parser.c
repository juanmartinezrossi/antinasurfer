/*
 * statsman_parser.c
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

#include "phid.h"
#include "phal_daemons.h"
#include "stats.h"

#include "phal_hw_api.h"
#include "phal_hw_api_man.h"

#include "set.h"
#include "str.h"
#include "var.h"
#include "rtcobj.h"
#include "phobj.h"
#include "pkt.h"
#include "daemon.h"

#include "statsman_parser.h"

#include "cfg_parser.h"

/*#define DEB
*/

int var_readvalue(var_o var, key_o value, int type);

vartype_t
type_tophal (int type)
{
	switch (type) {
	case PARAM_STRING:
		return STAT_TYPE_TEXT;
	case PARAM_CHAR:
		return STAT_TYPE_CHAR;
	case PARAM_INT:
		return STAT_TYPE_INT;
	case PARAM_FLOAT:
		return STAT_TYPE_FLOAT;
	default:
		xprintf ("STATSMAN: ERROR Unsuported type %d\n", type);
		return 0;
	}
}

char filename[128];

int statsman_parser_params(phobj_o obj)
{
	Set_o varset;
	var_o var;
	char *buffer;
	int i;
	cfg_o cfg;
	sect_o sect;
	key_o name, value;
	int offset;
	int size, type;


    /*AGBJuly15    sprintf(filename, "%s/%s/%s.%s", PARAMS_DIR,str_str(obj->appname),
			str_str(obj->objname),PARAMS_EXT);*/
/*	printf("statsman_parser_params(): PARAMS_DIR=%s, str_str(obj->appname=%s, str_str(obj->objname)=%s\n", PARAMS_DIR, str_str(obj->appname), str_str(obj->objname));
*/
	printf("APP_Name:\033[1;31m%s\033[0m, Module:\033[1;32m%s\033[0m, Params_Folder:\033[1;34m%s\033[0m\n", str_str(obj->appname), str_str(obj->objname), PARAMS_DIR);

	sprintf(filename, "%s/%s.%s", PARAMS_DIR, str_str(obj->objname),PARAMS_EXT);



	/*AGBJuly15 offset = hwapi_res_parseall(filename, &buffer);*/
	offset = hwapi_res_parseall_(filename, &buffer, 1);
	if (offset < 0) {
            #ifdef DEB
            xprintf ("STATSMAN: Error opening file %s\n", filename);
			printf ("STATSMAN: Error opening file %s\n", filename);
            #endif
            return 0;
	}
        
	cfg = cfg_new(buffer, offset);
	if (!cfg) {
            #ifdef DEB
            xprintf ("STATSMAN: Error parsing file %s", filename);
            #endif
            free(buffer);
            return 0;
	}

	varset = obj->params;
	if (!varset) {
            xprintf ("STATSMAN: Error creating variables");
            free(buffer);
            return 0;
	}
        
	for (i = 0; i < Set_length(cfg_sections(cfg)); i++) {
		sect = Set_get(cfg_sections(cfg), i);

		if (strcmp(sect_title(sect), "parameter")) {
			xprintf ("STATSMAN: Error unknown section %s\n", sect_title (sect));
		}

		name = Set_find(sect_keys(sect), "name", key_findname);
		value = Set_find(sect_keys(sect), "value", key_findname);

		type = key_type(value);
		if (type==-1) {
			xprintf ("STATSMAN: Caution type not guessed, trying with integer...\n");
			type = PARAM_INT;
		}

		size = key_nofelems(value, type);
		if (size == -1) {
			xprintf ("STATSMAN: Error trying to guess size, aborting...\n");
			break;
		}

		if (name && value) {
			var = var_new((varsz_t) size,
					(vartype_t) type_tophal(type));
			if (!var) {
				xprintf ("STATSMAN: Error creating variable");
				break;
			}
			str_delete(&var->name);
			var->name=str_dup(name->pvalue);

			var_readvalue(var, value, type);

			Set_put(varset, var);
		} else {
			xprintf ("STATSMAN: Not enougth params\n");
			break;
		}
	}
        
	free(buffer);

	if (i < Set_length(cfg_sections(cfg))) {
		xprintf ("STATSMAN: Error some params were not parsed.\n");
	}

	cfg_delete(&cfg);

        return 1;
}




