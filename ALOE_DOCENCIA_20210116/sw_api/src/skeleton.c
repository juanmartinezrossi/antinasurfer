/** ALOE headers
 */
#include <stdio.h>
#include <phal_sw_api.h>
#include <swapi_utils.h>
#include <setjmp.h>

extern struct utils_itf input_itfs[];
extern struct utils_itf output_itfs[];

extern struct utils_param params[];
extern struct utils_stat stats[];

#define MAX_IN_ITFS    50
#define CHECK_INPACKETS_TH 2

void (*atclosefnc)(void);

int currentInterfaceIdx;

int debug_mode=0;

struct initf {
    int fd;
    int rcv_len;
    int long_in;
    int counter;
    int statid;
    struct utils_itf *uitf;
};


struct initf initfs[MAX_IN_ITFS];


#define MAX_OUT_ITFS    50

struct outitf {
    int fd;
    int long_out;
    int error;
    int counter;
    int statid;
    struct utils_itf *uitf;
};


struct outitf outitfs[MAX_OUT_ITFS];

int nof_input_itfs, nof_output_itfs, nof_stats;

int proc_counter;

int Init()
{
    int i,n;

    if (!debug_mode) {

    	InitStat("CPU_usec", STAT_TYPE_INT, 1);
        InitStat("SYS_REL", STAT_TYPE_INT, 1);

        if (!CreateLog()) {
            printf("Error creating log\n");
            return 0;
        }

 /*       if (params[0].name != NULL) {
            printf("\nskeleton.c: InitParamFile();\n");
            n = InitParamFile();
            if (!n) {
                Log("Caution could not initiate params file\n");
            } else {
                for (i = 0; params[i].name; i++) {
                    n = GetParam(params[i].name, params[i].value, params[i].type, params[i].size);
					Log("INNIT n=%d\n", n);
                    if (!n) {
                        Log("Caution parameter %s not initated\n", params[i].name);
                    }
                }
            }
        }
*/
 /*       printf("\nskeleton.c: ConfigInterfaces();\n");*/
        ConfigInterfaces();
    }

    for (i = 0; input_itfs[i].name && i < MAX_IN_ITFS; i++) {
        if (!debug_mode) {
            initfs[i].fd = CreateItf(input_itfs[i].name, FLOW_READ_ONLY);

/*			printf("CreateItf(input_itfs[i].name, FLOW_READ_ONLY: initfs[%d].fd=%d\n", i, initfs[i].fd);*/

            if (initfs[i].fd < 0) {
                Log("Caution input interface %s not connected\n", input_itfs[i].name);
            }
        } else {
            initfs[i].fd=1;
        }
        initfs[i].rcv_len = 0;
        initfs[i].uitf = &input_itfs[i];
        if (initfs[i].uitf->get_block_sz == NULL) {
            initfs[i].long_in = initfs[i].uitf->max_buffer_len;
        }
        if (!debug_mode) {
/*            initfs[i].counter = InitCounter(input_itfs[i].name);
            if (initfs[i].counter<0) {
                Log("Error creating proc counter\n");
                return 0;
            }*/
        }
/*		printf("sw_api/skeleton.c: input_itfs[%d].name=%s\n", i, input_itfs[i].name);*/

    }

    nof_input_itfs = i;
    if (i == MAX_IN_ITFS) {
        Log("Caution some input interfaces may have not been initialized\n");
    }

    for (i = 0; output_itfs[i].name && i < MAX_OUT_ITFS; i++) {
        if (!debug_mode) {
            outitfs[i].fd = CreateItf(output_itfs[i].name, FLOW_WRITE_ONLY);

/*			printf("CreateItf(output_itfs[i].name, FLOW_WRITE_ONLY): outitfs[%d].fd=%d\n", i, outitfs[i].fd);*/

            if (outitfs[i].fd < 0) {
                Log("Caution output interface %s not connected\n", output_itfs[i].name);
            }
        } else {
            outitfs[i].fd=1;
        }
        outitfs[i].uitf = &output_itfs[i];
        if (!debug_mode) {
/*            outitfs[i].counter = InitCounter(output_itfs[i].name);
            if (outitfs[i].counter<0) {
                Log("Error creating proc counter\n");
                return 0;
            }
            */
        }
/*		printf("sw_api/skeleton.c: output_itfs[%d].name=%s\n", i, output_itfs[i].name);*/

    }

    nof_output_itfs = i;
    if (i==MAX_OUT_ITFS) {
        Log("Caution some output interfaces may have not been initialized\n");
    }

    if (!debug_mode) {
        for (i = 0; stats[i].name; i++) {
            if (!stats[i].id) {
                Log("Error configuring stat %s: Error missing id pointer for stats\n", stats[i].name);
                return 0;
            }
            if (stats[i].update_mode != OFF && !stats[i].value) {
                Log("Error configuring stat %s: Automatic mode needs a value pointer\n",stats[i].name);
                return 0;
            }
            *stats[i].id = InitStat(stats[i].name, stats[i].type, stats[i].size);
            if (*stats[i].id < 0) {
                Log("Error configuring stat %s: Error initiating with PHAL\n", stats[i].name);
                return 0;
            }
            if (stats[i].value) {
                SetStatsValue(*stats[i].id, stats[i].value, stats[i].size);
            }
        }

        proc_counter = InitCounter("PROC");
        if (proc_counter<0) {
            Log("Error creating proc counter\n");
            return 0;
        }

        nof_stats = i;

        return InitCustom();
    } else {
        return 1;
    }

}

int IsDataIn(int idx) {
	return GetItfStatus(initfs[idx].fd);
}
int IsDataOut(int idx) {
	return GetItfStatus(outitfs[idx].fd);
}

int SendItf(int idx, int len)
{
    int i = idx, n;

    if (idx < 0 || idx > nof_output_itfs) {
        Log("Invalid idx %d while sending\n", idx);
        
        return -1;
    }

    if (!len)
        return 0;

    if (outitfs[i].error) {
        Log("Not sending through itf %s due to an error.\n",outitfs[i].uitf->name);
        return 0;
    }

    if (outitfs[i].fd<=0) {
        #ifdef VERBOSE
        Log("Caution: trying to write to unconnected interface %s\n",outitfs[i].uitf->name);
        #endif
        return 1;
    }

    /* write to itf */
    if (!debug_mode) {

/*		printf("\nWRITE: sw_api/src/skeleton.c: outitfs[%d].fd=%u; outitfs[%d].uitf->buffer=%p, outitfs[%d].uitf->max_buffer_len=%d\n", 
				i, outitfs[i].fd, i, (void *)outitfs[i].uitf->buffer, i, outitfs[i].uitf->max_buffer_len);
*/

        n = WriteItf(outitfs[i].fd, outitfs[i].uitf->buffer, len*outitfs[i].uitf->sample_sz);
        if (n < 0) {
            Log("Error writing to itf %s\n", outitfs[i].uitf->name);
            outitfs[i].error=1;
            return -1;
        } else if (!n) {
            Log("Caution, missing packet due to full buffer for itf %s\n", outitfs[i].uitf->name);
        }
    }


    return n/outitfs[i].uitf->sample_sz;
}

int Run()
{
    int i,n,j;
    int block_sz;
    int nbytes=0;
    int npackets;

    #ifdef LOG_INOUT_DATA
    int j;
    #endif
    if (!debug_mode) {
        for (i=0; i < nof_stats; i++) {
            if (stats[i].update_mode == READ) {
                if (!stats[i].value) {
                    Log("Error in stat %s: Mode is READ but any value was defined\n",stats[i].name);

                } else {
                    GetStatsValue(*stats[i].id, stats[i].value, stats[i].size);
                }
            }
        }
    }
    for (i = 0; i < nof_input_itfs; i++) {
        currentInterfaceIdx=i;
        if (initfs[i].uitf->get_block_sz != NULL) {
            block_sz = initfs[i].uitf->get_block_sz();
            if (block_sz && block_sz != initfs[i].long_in) {
                if (block_sz > initfs[i].uitf->max_buffer_len) {
                	printf("Error invalid block size for itf %s. Bigger than buffer.\n",initfs[i].uitf->name);
                    
                    return 0;
                }
                if (block_sz + initfs[i].rcv_len > initfs[i].uitf->max_buffer_len) {
                    Log("Error changing min sample length for itf %s: too large\n", initfs[i].uitf->name);
                    
                    initfs[i].rcv_len = 0;
                }
                initfs[i].long_in = block_sz;
            }
        }
        /* receive a block of data */
        if (initfs[i].fd>0) {
#ifdef CHECK_INPACKETS_TH
        	npackets=GetItfStatus(initfs[i].fd);
        	if (npackets>CHECK_INPACKETS_TH) {
        		Log("%d packets in interface %s",npackets, initfs[i].uitf->name);
        	}
#endif
            #ifdef READ_ALL_PACKETS
            do {
            #endif
                if (!debug_mode) {
                    /*StartCounter(initfs[i].counter);*/

/*					printf("READ: sw_api/src/skeleton.c: initfs[%d].fd=%u; initfs[%d].uitf->buffer=%u, initfs[i].rcv_len=%d\n", 
								i, initfs[i].fd, i, initfs[i].uitf->buffer, initfs[i].rcv_len);
*/
                    n = ReadItf(initfs[i].fd, (void*) (((char*) initfs[i].uitf->buffer) + initfs[i].rcv_len),
                            initfs[i].long_in*initfs[i].uitf->sample_sz - initfs[i].rcv_len);
                    if (n < 0) {
                    	printf("Error reading from itf %s\n",initfs[i].uitf->name);

                        return 0;
                    }
                } else {
                    /* generate random inputs */
                    for (j=0;j<initfs[i].long_in*initfs[i].uitf->sample_sz;j++) {
                        *((char*) initfs[i].uitf->buffer+i)=(char) rand();
                    }
                    n=initfs[i].long_in*initfs[i].uitf->sample_sz;
                }
                #ifdef READ_ALL_PACKETS
                 else if (!n) {
                    break;
                }
                #endif
                initfs[i].rcv_len += n;
            #ifdef READ_ALL_PACKETS
            } while (n
                && (initfs[i].rcv_len < initfs[i].long_in*initfs[i].uitf->sample_sz
                        || (initfs[i].uitf->get_block_sz==NULL && initfs[i].rcv_len > 0)));
            #endif
            nbytes += initfs[i].rcv_len;

            /* when enough data is received, process and send it */
            if (initfs[i].rcv_len >= initfs[i].long_in*initfs[i].uitf->sample_sz
                    || ((initfs[i].uitf->get_block_sz==NULL || !initfs[i].uitf->get_block_sz())
                            && initfs[i].rcv_len > 0)) {
                if (n>0) {

                    #ifdef LOG_INOUT_DATA
                    sprintf(s,"IN%d: ",i);
                    for (j=0;j<LOG_INOUT_DATA;j++) {
                        sprintf(s,"%s%d,",s,*((char*) initfs[i].uitf->buffer+j));
                    }
                    Log("%s\n",s);
                    #endif


                    /*if (!debug_mode)
                        StopCounter(initfs[i].counter);*/
                    if (initfs[i].uitf->process_fnc) {
						StartCounter(proc_counter);
						n = initfs[i].uitf->process_fnc(initfs[i].rcv_len/initfs[i].uitf->sample_sz);
						StopCounter(proc_counter);
                    }
                }
                if (n < 0) {
                	printf("Error processing input flow itf %s\n", initfs[i].uitf->name);
                    return 0;
                } 
                initfs[i].rcv_len = 0;
            }
        }
    }

    nbytes=0;

    for (i=0;i<nof_output_itfs;i++) {
        currentInterfaceIdx=i;
        if (outitfs[i].uitf->get_block_sz != NULL
                && outitfs[i].uitf->process_fnc != NULL
                && outitfs[i].fd>0) {

            outitfs[i].long_out = outitfs[i].uitf->get_block_sz();

            if (outitfs[i].long_out > outitfs[i].uitf->max_buffer_len) {
                Log("Error requested size exceeds output buffer for itf %s\n", outitfs[i].uitf->name);
                
            }

            if (outitfs[i].long_out) {
                if (outitfs[i].error) {
                	printf("Not sending through itf %s due to an error.\n",outitfs[i].uitf->name);
                    return 0;
                }
                if (!outitfs[i].uitf->process_fnc(outitfs[i].long_out)) {
                    printf( "Error generating data for output itf %s.\n",outitfs[i].uitf->name);
                    
                    return 0;
                }
                if (!debug_mode) {
/*                    StartCounter(outitfs[i].counter);*/
                    n=SendItf(i,outitfs[i].long_out);
/*                    StopCounter(outitfs[i].counter);*/
                    if (n < 0) {
                        Log("Error writing to itf %s\n", outitfs[i].uitf->name);
                        outitfs[i].error=1;
                        return 1;
                    } else if (!n) {
                        Log("Caution, missing packet due to full buffer for itf %s\n", outitfs[i].uitf->name);
                    }
                }
                nbytes+=outitfs[i].long_out;
            }
        }
    }

    if (!debug_mode) {
        for (i=0; i < nof_stats; i++) {
            if (stats[i].update_mode == WRITE) {
                if (!stats[i].value) {
                    Log("Error in stat %s: Mode is WRITE but any value was defined\n",stats[i].name);
                } else {
                    SetStatsValue(*stats[i].id, stats[i].value, stats[i].size);
                }
            }
        }
    }
    StartCounter(proc_counter);
    i=RunCustom();
    StopCounter(proc_counter);
    return i;
}

int interval;

int GetStatSize(int i) {
	return stats[i].size;
}
void SetStatSize(int i, int size) {
	stats[i].size=size;
}

void AtClose(void (*closefnc)(void)) {
	atclosefnc=closefnc;
}

void StatSet(char *name, void *value) {
	int i;
	for(i=0;stats[i].name;i++) {
		if (!strcmp(name,stats[i].name)) {
			SetStatsValue(*stats[i].id,value,stats[i].size);
		}
	}
}

int isItfConnected(int idx)
{
    return (outitfs[idx].fd>0);
}
void IntervalSet(int numslots)
{
    interval=numslots;
}

/** Main
 */
int main(int argc, char **argv)
{

    if (argc==2) {
        if (!strcmp(argv[1],"-d")) {
            debug_mode=1;
        }
    }
    atclosefnc=NULL;

    interval=1;

    if (!debug_mode)
        InitPHAL();

    if (debug_mode) {
        Init();
        sleep_ms(500);
        while(1)
            Run();
    } else {
            while (1) {
            switch (Status()) {
            case PHAL_STATUS_INIT:
                if (!Init()) {
                    Log("Error initiating. Exiting...\n");
                    ClosePHAL();
                    exit(0);
                }
                break;
            case PHAL_STATUS_RUN:
                if (!Run()) {
                    Log("Error running. Exiting...\n");
                    ClosePHAL();
                    exit(0);
                }
                break;
            default:
            	if (atclosefnc)
            		atclosefnc();
                ClosePHAL();
                exit(0);
                break;
            }

            Relinquish(interval);

        }
        return 1;
    }

}


