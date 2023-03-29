#include <stdio.h>
#include <stdlib.h>

#include "set.h"
#include "str.h"
#include "cfg_parser.h"
#include "phal_hw_api_man.h"
#include <string.h>

#define DEFAULT_DASCALE 32768
#define DEFAULT_ADSCALE	32768


char filepath[200];

int x5_readcfg(char *name, int *NsamplesIn, int *NsamplesOut) {
	int ad_enable,da_enable,SamplingFreq,ClockInternal,SyncAD,direct;
	int ADsamples,ADdecimate,ADprint,ADscale;
	int DAsamples,DAdecimate,DAinterpolate,DAdivider,DAdosin,DAsendad,DAprint,DAscale,DAsinfreq;

	int i, j, offset;
	char *buffer;
	cfg_o cfg;
	sect_o sect;
	key_o key;
	int error=0;
	
	hwapi_mem_silent(1);

	sprintf(filepath, "/usr/local/etc/%s", name);

	offset = hwapi_res_parseall_(filepath, &buffer,1);
	if (offset < 0) {
		printf ("Error reading file %s.\n", filepath);
		return 0;
	}

	cfg = cfg_new(buffer, offset);
	if (!cfg) {
		printf ("Error parsing file %s\n", filepath);
		free(buffer);
		return 0;
	}
	sect=Set_find(cfg_sections(cfg),"main",sect_findtitle);
	if (!sect) {
		printf("Error can't find section main\n");
		free(buffer);
		cfg_delete(&cfg);
		return 0;
	} else {
		key = Set_find(sect_keys(sect),"dma",key_findname);
		if (!key) {
			direct=0;
		} else {
			direct=!str_scmp(key->pvalue,"yes");
		}
		key = Set_find(sect_keys(sect),"sampling_freq",key_findname);
		if (!key) {
			printf("Error missing key sampling_freq\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			key_value(key,0,PARAM_INT,&SamplingFreq);
		}
		key = Set_find(sect_keys(sect),"clock_source",key_findname);
		if (!key) {
			printf("Error missing key clock_source\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			if (!str_scmp(key->pvalue,"internal")) {
				ClockInternal=1;
			} else if (!str_scmp(key->pvalue,"external")) {
				ClockInternal=0;
			} else {
				printf("Unkown clock_source %s\n",str_str(key->pvalue));
			}
		}
		key = Set_find(sect_keys(sect),"sync_ts",key_findname);
		if (!key) {
			printf("Error missing key sync_ts\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			if (!str_scmp(key->pvalue,"ad")) {
				SyncAD=1;
			} else if (!str_scmp(key->pvalue,"da")) {
				SyncAD=0;
			} else {
				printf("Unkown sync_ts %s\n",str_str(key->pvalue));
			}
		}
	}
	sect=Set_find(cfg_sections(cfg),"dac",sect_findtitle);
	if (!sect) {
		printf("DAC is disabled\n");
		da_enable=0;
	} else {
		da_enable=1;
		key = Set_find(sect_keys(sect),"scale",key_findname);
		if (!key) {
			DAscale=DEFAULT_DASCALE;
		} else {
			key_value(key,0,PARAM_INT,&DAscale);
		}
		key = Set_find(sect_keys(sect),"interpolation",key_findname);
		if (!key) {
			printf("Error missing key interpolation\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {

			key_value(key,0,PARAM_INT,&DAinterpolate);
		}
		key = Set_find(sect_keys(sect),"decimation",key_findname);
		if (!key) {
			printf("Error missing key decimation\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			key_value(key,0,PARAM_INT,&DAdecimate);
		}
		key = Set_find(sect_keys(sect),"clock_divider",key_findname);
		if (!key) {
			printf("Error missing key clock_divider\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			key_value(key,0,PARAM_INT,&DAdivider);
		}
		key = Set_find(sect_keys(sect),"packet_size",key_findname);
		if (!key) {
			printf("Error missing key packet_size\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			key_value(key,0,PARAM_INT,&DAsamples);
		}
		key = Set_find(sect_keys(sect),"debug_sin",key_findname);
		if (!key) {
			DAdosin=0;
		} else {
			if (!str_scmp(key->pvalue,"yes")) {
				DAdosin=1;
			} else if (!str_scmp(key->pvalue,"no")) {
				DAdosin=0;
			} else {
				printf("Unknown debug_sin option %s\n",str_str(key->pvalue));
				DAdosin=0;
			}
		}
		key = Set_find(sect_keys(sect),"debug_sin_freq",key_findname);
		if (!key) {
			DAsinfreq=0;
		} else {
			key_value(key,0,PARAM_FLOAT,&DAsinfreq);
		}
		key = Set_find(sect_keys(sect),"debug_print",key_findname);
		if (!key) {
			DAprint=0;
		} else {
			if (!str_scmp(key->pvalue,"yes")) {
				DAprint=1;
			} else if (!str_scmp(key->pvalue,"no")) {
				DAprint=0;
			} else {
				printf("Unknown debug_print option %s\n",str_str(key->pvalue));
				DAprint=0;
			}
		}
		key = Set_find(sect_keys(sect),"send_onad",key_findname);
		if (!key) {
                        DAsendad=0;
		} else {
			if (!str_scmp(key->pvalue,"yes")) {
				DAsendad=1;
			} else if (!str_scmp(key->pvalue,"no")) {
				DAsendad=0;
			} else {
				printf("Unknown send_onad option %s\n",str_str(key->pvalue));
				DAsendad=0;
			}
		}
	}
	sect=Set_find(cfg_sections(cfg),"adc",sect_findtitle);
	if (!sect) {
		printf("ADC is disabled\n");
		ad_enable=0;
	} else {
		ad_enable=1;
		key = Set_find(sect_keys(sect),"scale",key_findname);
		if (!key) {
			ADscale=DEFAULT_ADSCALE;
		} else {
			key_value(key,0,PARAM_INT,&ADscale);
		}
		key = Set_find(sect_keys(sect),"decimation",key_findname);
		if (!key) {
			printf("Error missing key decimation\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			key_value(key,0,PARAM_INT,&ADdecimate);
		}
		key = Set_find(sect_keys(sect),"packet_size",key_findname);
		if (!key) {
			printf("Error missing key packet_size\n");
			free(buffer);
			cfg_delete(&cfg);
			return 0;
		} else {
			key_value(key,0,PARAM_INT,&ADsamples);
		}
		key = Set_find(sect_keys(sect),"debug_print",key_findname);
		if (!key) {
                        ADprint=0;
		} else {
			if (!str_scmp(key->pvalue,"yes")) {
				ADprint=1;
			} else if (!str_scmp(key->pvalue,"no")) {
				ADprint=0;
			} else {
				printf("Unknown debug_print option %s\n",str_str(key->pvalue));
				ADprint=0;
			}
		}
	}

	free(buffer);

	cfg_delete(&cfg);

	*NsamplesIn=ADsamples;
	*NsamplesOut=DAsamples;
	
	x5_setcfg(ad_enable, da_enable, SamplingFreq, ClockInternal, SyncAD,
	 ADsamples,  ADdecimate, ADprint,
	 DAsamples, DAdecimate, DAinterpolate, DAdivider, DAdosin,DAsendad,DAprint,direct);
	return 1;

}
