
#ifndef _STATS_H
#define	_STATS_H

#define SIGNAL_STAT_BUFF	512

#define OPMODE_TX		0
#define OPMODE_RX		1
#define OPMODE_RXTX		2
#define OPMODE_TEST0	3	//Alternated Tones
#define OPMODE_TEST1	4	//Quality Tone 
#define OPMODE_BYPASS	5	//Bypass


/** AD/DA Parameters from dac_cfg.h */
int op_mode=2;		// Operation Mode: 
					// 0=TX, 1=RX, 2=RX&TX, 3=TEST0 (TX generated signal), 4=TEST1 (Quality Tone), 5=BYPASS (ECHO effect)
double inputFreq;
double outputFreq;
double inputRFFreq;
double outputRFFreq;
int sampleType=DAC_SAMPLE_COMPLEXFLOAT;	//FIXED TO COMPLEX DATA: DO NOT CHANGE
int nof_channels=1;
float tx_gain=1.0;						//Lineal multiplier
float rx_gain=1.0;
float tx_bw;
float rx_bw;
int NsamplesIn=1024;
int NsamplesOut=1024;

/* Please, check carefully the param names in statman/app_name/modulename.params file*/
struct utils_param params[] = {
	/** use dac_cfg.h datatypes here */
	{"op_mode",STAT_TYPE_INT,1,&op_mode},
 //   {"sampleType",STAT_TYPE_INT,1,&sampleType},
    {"NsamplesIn",STAT_TYPE_INT,1,&NsamplesIn},
	{"NsamplesOut",STAT_TYPE_INT,1,&NsamplesOut},
    {"tx_gain",STAT_TYPE_FLOAT,1,&tx_gain},
	{"rx_gain",STAT_TYPE_FLOAT,1,&rx_gain},
    {NULL, 0, 0, 0}};


float outsignal[SIGNAL_STAT_BUFF];

struct utils_stat stats[] = {
                        	/*{"gainTX",
                            STAT_TYPE_FLOAT,
                            1,
                            &stat_gain,
                            (void*)&gain,
                            READ},

                            {"nsamples",
							STAT_TYPE_INT,
							1,
							&stat_nsamp,
							(void*)&nsamples,
							WRITE},


                            {"rcv_i",
							STAT_TYPE_INT,
							1,
							&stat_rcvi,
							(void*)&rcv_i,
							WRITE},

                            {"rcv_q",
							STAT_TYPE_INT,
							1,
							&stat_rcvq,
							(void*)&rcv_q,
							WRITE},


						   {"save_signal",
							STAT_TYPE_INT,
							1,
							&stat_save,
							&save_signal,
							READ},

                        	{"output_signal",
                            STAT_TYPE_FLOAT,
                            SIGNAL_STAT_BUFF,
                            &stat_signal,
                            (void*)outsignal,
                            WRITE},*/
                            {NULL, 0, 0, 0, 0, 0}};


#endif	/* _STATS_H */

