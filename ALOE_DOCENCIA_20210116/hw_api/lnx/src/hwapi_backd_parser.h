/*
 * hwapi_backd_parser.h
 *
 *  Created on: Jun 4, 2012
 *      Author: ismael
 */



#include "set.h"
#include "str.h"
#include "cfg_parser.h"

int parse_section_platform(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_dac(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_other(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_interfaces(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_objects(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_kernel(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_network(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_priorities(Set_o keys, struct hw_api_db *hwapi, int idx);
int parse_section_affinity(Set_o keys, struct hw_api_db *hwapi, int idx);

int missing_section_objects(struct hw_api_db *hwapi);
int missing_section_network(struct hw_api_db *hwapi);
int missing_section_priorities(struct hw_api_db *hwapi);
int missing_section_affinity(struct hw_api_db *hwapi);

