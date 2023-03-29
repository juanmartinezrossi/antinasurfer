/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <complex.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <getopt.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include "print_utils.h"

#include <alsa/asoundlib.h>

#include "set.h"
#include "str.h"
#include "cfg_parser.h"
#include "dac_cfg.h"

#define PI		3.14159265359	/* pi approximation */
#define PIx2	6.28318530718
//struct timeval tv0, tv1, diff;
struct timespec tv0, tv1, diff;

// ALSA TX parameters
static char *device = "plughw:0,0";                     /* playback device */
static snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE; /* sample format */
static unsigned int rate = 48000;                       /* stream rate */
static unsigned int channels = 2;                       /* count of channels */
static unsigned int buffer_time = 500000;               /* ring buffer length in us */
unsigned int period_timeRX = 42666;               		/* period time in us */
double freqRX = 999.0;                              	 	/* sinusoidal wave frequency in Hz */

static int verbose = 1;                                 /* verbose flag */
int addhelpRX=0;											/* Print usage*/

static int resample = 0;                                /* enable alsa-lib resampling */
static int period_event = 0;                            /* produce poll event after each period */
static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;
static snd_output_t *output = NULL;
static int block_size=0;

snd_pcm_t *handleRX;
int err, morehelp;
snd_pcm_hw_params_t *hwparams;
snd_pcm_sw_params_t *swparams;
signed short *samples;
unsigned int chn;
snd_pcm_channel_area_t *areas;
char filepath[200];
int SamplingFrequency;


/* Pointer to ALOE input/output buffers*/
_Complex float *datoutputRX, *datinputRX;

// Predefined Functions
static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
static int async_loop(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);
static void help(void);

int init_toneCOMPLEXRX(_Complex float *table, int length, float ref_freq, float gain, float sampl_freq);
int gen_toneCOMPLEXRX(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float ref_freq, float tone_freq, float sampl_freq);
void CPLXconvert2pcm8bitsstreamRX(_Complex float *in, signed char *out, int numframesIN);
void CPLXconvert2pcm16bitsstreamRX(_Complex float *in, signed short *out, int numframesIN);
void pcm16bitsstream2CPLXconvertRX( signed short *in, _Complex float *out, int numframesIN);
static void generate_sine(const snd_pcm_channel_area_t *areas, 
                          snd_pcm_uframes_t offset,
                          int count, double *_phase);
void (*syncFunction)(void);

static void signal_handler(int sig)
{
	snd_pcm_close(handleRX);;
	fprintf(stderr, "signal received, exiting ...sig=%d\n", sig);
	exit(0);
}

struct async_private_data {
        signed short *samples;
        snd_pcm_channel_area_t *areas;
        double phase;
};

static void async_callback(snd_async_handler_t *ahandler)
{
	int i;	
	static int Tslot=0;
    snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
    struct async_private_data *data = snd_async_handler_get_callback_private(ahandler);
    signed short *samples = data->samples;
    snd_pcm_channel_area_t *areas = data->areas;
    snd_pcm_sframes_t avail;
    int err;
    _Complex float *soundsignal;    

	clock_gettime(CLOCK_REALTIME, &tv0);
//	clock_gettime(CLOCK_REALTIME, &tv1);
	diff.tv_nsec=(tv0.tv_sec-tv1.tv_sec)*1000000000+(tv0.tv_nsec-tv1.tv_nsec);
	tv1.tv_sec=tv0.tv_sec;
	tv1.tv_nsec=tv0.tv_nsec;
	printf("diff=%ld (ns)\n", diff.tv_nsec);
	


	soundsignal=datinputRX;
	avail = snd_pcm_avail_update(handle);
	while (avail >= period_size){
		err = snd_pcm_readi(handle, samples, block_size);
		pcm16bitsstream2CPLXconvertRX(samples, soundsignal, (int)block_size);
        if (err < 0) {
                printf("Write error: %s\n", snd_strerror(err));
                exit(EXIT_FAILURE);
        }
        if (err != block_size) {
                printf("Write error: written %i expected %li\n", err, (long int)block_size);
                exit(EXIT_FAILURE);
        }
        avail = snd_pcm_avail_update(handle);
		soundsignal += block_size;
	}
	Tslot++;
}


int alsacardRX_readcfg(struct dac_cfg *dac_cfg) {
	int i=0, j, offset;
	char *buffer;
	cfg_o cfg;
	sect_o sect;
	key_o key;
	char *name;
	float tmp;
	int block_sz=0;

	dac_cfg->inputFreq=48000.0;
	dac_cfg->outputFreq=48000.0;
	dac_cfg->inputRFFreq=0.0;
	dac_cfg->outputRFFreq=0.0;
	dac_cfg->sampleType=DAC_SAMPLE_COMPLEXFLOAT;
	dac_cfg->nof_channels=1;
	dac_cfg->tx_gain=1.0;
	dac_cfg->rx_gain=1.0;
	dac_cfg->tx_bw=1.0;
	dac_cfg->rx_bw=1.0;
	dac_cfg->NsamplesIn=0;
	dac_cfg->NsamplesOut=0;
	dac_cfg->DAC_Active=0;

	hwapi_mem_silent(1);
	name=dac_cfg->filename;
	sprintf(filepath, "/usr/local/etc/%s", name);
	offset = hwapi_res_parseall_(filepath, &buffer,1);
	if (offset < 0) {
		printf ("Error reading file %s\n", filepath);
		return 0;
	}

	cfg = cfg_new(buffer, offset);
	if (!cfg) {
		printf ("Error parsing file %s\n", filepath);
		free(buffer);
		return 0;
	}
	hwapi_mem_silent(0);
	//READ SECTION "tx"
	sect=Set_find(cfg_sections(cfg),"tx",sect_findtitle);
	if (!sect) {
		printf("TX is disabled\n");
	} else {
		if (!(key = Set_find(sect_keys(sect),"samp_freq",key_findname))) {
			printf("Warning unspecified tx sample frequency. Setting to 1 MHz\n");
			dac_cfg->outputFreq = 48000;
		} else {
			key_value(key,1,PARAM_FLOAT,&tmp);
			dac_cfg->outputFreq=(double) tmp;
			printf("ALSASOUND_CARD_TX: outputFreq=%3.1f\n", (float)dac_cfg->outputFreq);
		}
		rate=(unsigned int)dac_cfg->outputFreq;

		if (!(key = Set_find(sect_keys(sect),"freq",key_findname))) {
			printf("Warning unspecified tx frequency. Setting to default\n");
			dac_cfg->outputRFFreq = 0;
		} else {
			key_value(key,1,PARAM_FLOAT,&tmp);
			dac_cfg->outputRFFreq=(double) tmp;
			printf("ALSASOUND_CARD_TX: outputRFFreq=%3.1f\n", (float)dac_cfg->outputRFFreq);
		}
		/* block_size*/
		if (!(key = Set_find(sect_keys(sect),"blocksize",key_findname))) {
			printf("Warning unspecified tx blocksize. Setting to 1K\n");
			block_sz=1024;
		} else {
			key_value(key,1,PARAM_INT,&block_sz);
			printf("ALSASOUND_CARD_TX: block_size=%d\n", (int)block_sz);
			if((int)block_sz > DAC_BUFFER_SZ){
				printf("sndcard_imp.c==>sndcard_readcfg(): ERROR:  block_size=%d > DAC_BUFFER_SZ=%d\n",
							(int)block_sz, DAC_BUFFER_SZ);
				printf("Please, decrease the 'blocksize' parameter in hw_api/lnx/cfg/alsasound.conf");
				return(0);					
			}
		}
		block_size=block_sz;
		/* period_size*/
		if (!(key = Set_find(sect_keys(sect),"period_size",key_findname))) {
			printf("Warning unspecified tx blocksize. Setting to 1K\n");
			dac_cfg->NsamplesOut = 1024;
		} else {
			key_value(key,1,PARAM_INT,&dac_cfg->NsamplesOut);
			printf("ALSASOUND_CARD_TX: NsamplesOut=%d\n", (int)dac_cfg->NsamplesOut);
			if((int)dac_cfg->NsamplesOut > DAC_BUFFER_SZ){
				printf("alsacard_imp.c==>sndcard_readcfg(): ERROR:  NsamplesOut=%d > DAC_BUFFER_SZ=%d\n",
							(int)dac_cfg->NsamplesOut, DAC_BUFFER_SZ);
				printf("Please, decrease the 'blocksize' parameter in hw_api/lnx/cfg/alsasound.conf");
				return(0);					
			}
		}
		period_size=(snd_pcm_sframes_t)dac_cfg->NsamplesOut;
		period_timeRX=(unsigned int)((period_size*1000000)/rate);
		
		if (!(key = Set_find(sect_keys(sect),"gain",key_findname))) {
			printf("Warning unspecified tx gain. Setting to default\n");
			dac_cfg->tx_gain = 1.0;
		} else {
			key_value(key,1,PARAM_FLOAT,&dac_cfg->tx_gain);
			printf("ALSASOUND_CARD_TX: tx_gain=%3.1f\n", (float)dac_cfg->tx_gain);
		}
		if (!(key = Set_find(sect_keys(sect),"bw",key_findname))) {
			printf("Warning unspecified bw. Setting to default\n");
			dac_cfg->tx_bw = 24000.0;
		} else {
			key_value(key,1,PARAM_FLOAT,&dac_cfg->tx_bw);
			printf("ALSASOUND_CARD_TX: tx_bw=%3.1f\n", (float)dac_cfg->tx_bw);
		}
	}
	//READ SECTION "rx"
	sect=Set_find(cfg_sections(cfg),"rx",sect_findtitle);
	if (!sect) {
		printf("RX is disabled\n");
	} else {
		if (!(key = Set_find(sect_keys(sect),"samp_freq",key_findname))) {
			printf("Warning unspecified rx sample frequency. Setting to 1 MHz\n");
			dac_cfg->inputFreq = 48000;
		} else {
			key_value(key,1,PARAM_FLOAT,&tmp);
			dac_cfg->inputFreq = (double) tmp;
			printf("ALSASOUND_CARD_RX: inputFreq=%3.1f\n", (float)dac_cfg->inputFreq);
		}
		if (!(key = Set_find(sect_keys(sect),"freq",key_findname))) {
			printf("Warning unspecified rx frequency. Setting to default\n");
			dac_cfg->inputRFFreq = 0;
		} else {
			key_value(key,1,PARAM_FLOAT,&tmp);
			dac_cfg->inputRFFreq = (double) tmp;
			printf("ALSASOUND_CARD_RX: inputRFFreq=%3.1f\n", (float)dac_cfg->inputRFFreq);
		}
		/* block_size*/
		if (!(key = Set_find(sect_keys(sect),"blocksize",key_findname))) {
			printf("Warning unspecified tx blocksize. Setting to 1K\n");
			block_sz=1024;
		} else {
			key_value(key,1,PARAM_INT,&block_sz);
			printf("ALSASOUND_CARD_TX: block_size=%d\n", (int)block_sz);
			if((int)block_sz > DAC_BUFFER_SZ){
				printf("sndcard_imp.c==>sndcard_readcfg(): ERROR:  block_size=%d > DAC_BUFFER_SZ=%d\n",
							(int)block_sz, DAC_BUFFER_SZ);
				printf("Please, decrease the 'blocksize' parameter in hw_api/lnx/cfg/alsasound.conf");
				return(0);					
			}
		}
		block_size=block_sz;

		/* period_size*/
		if (!(key = Set_find(sect_keys(sect),"period_size",key_findname))) {
			printf("Warning unspecified tx blocksize. Setting to 1K\n");
			dac_cfg->NsamplesIn = 1024;
		} else {
			key_value(key,1,PARAM_INT,&dac_cfg->NsamplesIn);
			printf("ALSASOUND_CARD_TX: NsamplesIn=%d\n", (int)dac_cfg->NsamplesIn);
			if((int)dac_cfg->NsamplesIn > DAC_BUFFER_SZ){
				printf("alsacard_imp.c==>sndcard_readcfg(): ERROR:  NsamplesOut=%d > DAC_BUFFER_SZ=%d\n",
							(int)dac_cfg->NsamplesIn, DAC_BUFFER_SZ);
				printf("Please, decrease the 'blocksize' parameter in hw_api/lnx/cfg/alsasound.conf");
				return(0);					
			}
		}
		period_size=(snd_pcm_sframes_t)dac_cfg->NsamplesIn;
		period_timeRX=(unsigned int)((period_size*1000000)/rate);


		if (!(key = Set_find(sect_keys(sect),"gain",key_findname))) {
			printf("Warning unspecified rx gain. Setting to default\n");
			dac_cfg->rx_gain = 1.0;
		} else {
			key_value(key,1,PARAM_FLOAT,&dac_cfg->rx_gain);
			printf("ALSASOUND_CARD_RX: rx_gain=%3.1f\n", (float)dac_cfg->rx_gain);
		}
		if (!(key = Set_find(sect_keys(sect),"bw",key_findname))) {
			printf("Warning unspecified rx bw. Setting to default\n");
			dac_cfg->rx_bw = 24000;
		} else {
			key_value(key,1,PARAM_FLOAT,&dac_cfg->rx_bw);
			printf("ALSASOUND_CARD_RX: rx_bw=%3.1f\n", (float)dac_cfg->rx_bw);
		}
	}
	if(verbose){
		printf("==============================================================================================0\n");
		printf("dac_cfg->outputFreq=%6.1f, dac_cfg->outputRFFreq=%6.1f\n", dac_cfg->outputFreq, dac_cfg->outputRFFreq);
		printf("dac_cfg->sampleType=%d, dac_cfg->nof_channels=%d\n", dac_cfg->sampleType, dac_cfg->nof_channels);
		printf("dac_cfg->tx_gain=%6.1f, dac_cfg->tx_bw=%6.1f\n", dac_cfg->tx_gain, dac_cfg->tx_bw);
		printf("dac_cfg->NsamplesIn=%d\n", dac_cfg->NsamplesIn);
		printf("dac_cfg->DAC_Active=%d\n", dac_cfg->DAC_Active);
		printf("alsacard_readcfg(): period_size=%d, period_timeRX=%d\n", (int)period_size, period_timeRX);
		printf("==============================================================================================0\n");
	}
	free(buffer);
	cfg_delete(&cfg);
	return 1;
}


int alsacardRX_init(struct dac_cfg *cfg, int *timeSlotLength, void (*sync)(void)) {

		/* Capture input/output buffer pointers*/
		datinputRX = (_Complex float *)&cfg->dacinbuff[0][0];
		datoutputRX = (_Complex float *)&cfg->dacoutbuff[0][0];

		syncFunction=sync;

		/* Print ALSA driver usage*/
		if(addhelpRX==1)help();

		/* Configure ALSA params*/
		snd_pcm_hw_params_alloca(&hwparams);
		snd_pcm_sw_params_alloca(&swparams);

		/* Calculate Time Slot */
		SamplingFrequency = (int)(cfg->inputFreq);
		*timeSlotLength=(int)(1000000.0*((float)(cfg->NsamplesIn)/(float)(SamplingFrequency)));	

		if(SamplingFrequency != (int)(cfg->inputFreq)){
			printf("Output Sampling Frequency defines timeslot length\n");
			printf("Please, assume the Output Sampling Frequency as the reference one\n");
		}		
		printf(RESET BOLD T_BLUE "RX============================================================================================0\n");
		printf(RESET BOLD T_LIGHTBLUE);
		printf("\nO  ALSA SOUND CARD PROPERLY INITIALLIZED\n");
		printf("O  *timeSlotLength=%d (us), cfg->NsamplesIn=%d, SamplingFrequency=%d\n", *timeSlotLength, cfg->NsamplesIn, (int)(cfg->inputFreq));
		printf("O\n");
		printf("O  WARNING!!!; Required TimeSlot=%d (us). \n", *timeSlotLength);
		printf("O            Please, setup accordingly at hw_api/lnx/cfg/platform.conf   \n");
		printf("O\n");
        printf("O  Capture device is %s\n", device);
        printf("O  Stream parameters are %iHz, %s, %i channels\n", rate, snd_pcm_format_name(format), channels);
        printf("O  Sine wave rate is %.4fHz\n", freqRX);
        printf("O  Using transfer methodRX: '%s'\n", "async");
		printf("O  ALSA SOUND CARD PROPERLY INITIALLIZED\n\n");
		printf(RESET BOLD T_BLUE "==============================================================================================0\n");
		printf(RESET BOLD T_BLACK);

		err = snd_output_stdio_attach(&output, stdout, 0);
        if (err < 0) {
                printf("Output failed: %s\n", snd_strerror(err));
                return -1;
        }
        if ((err = snd_pcm_open(&handleRX, device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
                printf("Capture open error: %s\n", snd_strerror(err));
                return -1;
        }
        if ((err = set_hwparams(handleRX, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
                printf("Setting of hwparams failed: %s\n", snd_strerror(err));
                return -1;
        }
        if ((err = set_swparams(handleRX, swparams)) < 0) {
                printf("Setting of swparams failed: %s\n", snd_strerror(err));
                return -1;
        }
       	if (verbose > 0)snd_pcm_dump(handleRX, output);

        samples = malloc((period_size * channels * snd_pcm_format_physical_width(format)) / 8);
        if (samples == NULL) {
                printf("No enough memory\n");
                return -1;
        }     
        areas = calloc(channels, sizeof(snd_pcm_channel_area_t));
        if (areas == NULL) {
                printf("No enough memory\n");
                return -1;
        }
        for (chn = 0; chn < channels; chn++) {
                areas[chn].addr = samples;
                areas[chn].first = chn * snd_pcm_format_physical_width(format);
                areas[chn].step = channels * snd_pcm_format_physical_width(format);
        }
		err = async_loop(handleRX, samples, areas);
        if (err < 0)
                printf("Transfer failed: %s\n", snd_strerror(err));
        free(areas);
        free(samples);
        snd_pcm_close(handleRX);

	return 1;
}

void alsacardRX_close() {
    free(areas);
    free(samples);
    snd_pcm_close(handleRX);
}


static int set_hwparams(snd_pcm_t *handle,
                        snd_pcm_hw_params_t *params,
                        snd_pcm_access_t access)
{
        unsigned int rrate;
        snd_pcm_uframes_t size;
        int err, dir;
        /* choose all parameters */
        err = snd_pcm_hw_params_any(handle, params);
        if (err < 0) {
                printf("Broken configuration for playback: no configurations available: %s\n", snd_strerror(err));
                return err;
        }
        /* set hardware resampling */
        err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
        if (err < 0) {
                printf("Resampling setup failed for capture: %s\n", snd_strerror(err));
                return err;
        }
        /* set the interleaved read/write format */
        err = snd_pcm_hw_params_set_access(handle, params, access);
        if (err < 0) {
                printf("Access type not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the sample format */
        err = snd_pcm_hw_params_set_format(handle, params, format);
        if (err < 0) {
                printf("Sample format not available for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* set the count of channels */
        err = snd_pcm_hw_params_set_channels(handle, params, channels);
        if (err < 0) {
                printf("Channels count (%i) not available for playbacks: %s\n", channels, snd_strerror(err));
                return err;
        }
        /* set the stream rate */
        rrate = rate;
        err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
        if (err < 0) {
                printf("Rate %iHz not available for playback: %s\n", rate, snd_strerror(err));
                return err;
        }
        if (rrate != rate) {
                printf("Rate doesn't match (requested %iHz, get %iHz)\n", rate, err);
                return -EINVAL;
        }
        /* set the buffer time */
        err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time, &dir);
        if (err < 0) {
                printf("Unable to set buffer time %i for playback: %s\n", buffer_time, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_buffer_size(params, &size);
        if (err < 0) {
                printf("Unable to get buffer size for playback: %s\n", snd_strerror(err));
                return err;
        }
		printf("buffer_time=%d, size=%d\n", buffer_time, size);

        buffer_size = size;
        /* set the period time */

        err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_timeRX, &dir);
        if (err < 0) {
                printf("Unable to set period time %i for playback: %s\n", period_timeRX, snd_strerror(err));
                return err;
        }
        err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
        if (err < 0) {
                printf("Unable to get period size for playback: %s\n", snd_strerror(err));
                return err;
        }
		printf("period_timeRX=%d, size=%d\n", period_timeRX, size);

        period_size = size;
        /* write the parameters to device */
        err = snd_pcm_hw_params(handle, params);
        if (err < 0) {
                printf("Unable to set hw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
        int err;
        /* get the current swparams */
        err = snd_pcm_sw_params_current(handle, swparams);
        if (err < 0) {
                printf("Unable to determine current swparams for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* start the transfer when the buffer is almost full: */
        /* (buffer_size / avail_min) * avail_min */
        err = snd_pcm_sw_params_set_start_threshold(handle, swparams, (buffer_size / period_size) * period_size);
        if (err < 0) {
                printf("Unable to set start threshold mode for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* allow the transfer when at least period_size samples can be processed */
        /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
        err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_event ? buffer_size : period_size);
        if (err < 0) {
                printf("Unable to set avail min for playback: %s\n", snd_strerror(err));
                return err;
        }
        /* enable period events when requested */
        if (period_event) {
                err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
                if (err < 0) {
                        printf("Unable to set period event: %s\n", snd_strerror(err));
                        return err;
                }
        }
        /* write the parameters to the playback device */
        err = snd_pcm_sw_params(handle, swparams);
        if (err < 0) {
                printf("Unable to set sw params for playback: %s\n", snd_strerror(err));
                return err;
        }
        return 0;
}

static int async_loop(snd_pcm_t *handle,
                      signed short *samples,
                      snd_pcm_channel_area_t *areas)
{
        struct async_private_data data;
        snd_async_handler_t *ahandler;
        int err, count;
        data.samples = samples;
        data.areas = areas;
        data.phase = 0;

        err = snd_async_add_pcm_handler(&ahandler, handle, async_callback, &data);
        if (err < 0) {
                printf("Unable to register async handler\n");
                exit(EXIT_FAILURE);
        }
/*        for (count = 0; count < 2; count++) {
          //      generate_sine(areas, 0, period_size, &data.phase);
                err = snd_pcm_readi(handle, samples, period_size);
                if (err < 0) {
                        printf("Initial write error: %s\n", snd_strerror(err));
                        return -1;
                }
                if (err != period_size) {
                        printf("Initial write error: written %i expected %li\n", err, period_size);
                        return -1;
                }
        }
*/
        if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
                err = snd_pcm_start(handle);
                if (err < 0) {
                        printf("Start error: %s\n", snd_strerror(err));
                        return -1;
                }
        }

	 	/* install a signal handler to properly quits jack client */
		signal(SIGQUIT, signal_handler);
		signal(SIGTERM, signal_handler);
		signal(SIGHUP, signal_handler);
		signal(SIGINT, signal_handler);
        /* because all other work is done in the signal handler,
           suspend the process */
		printf("BEFORE SLEEP\n");
        while (1) {
                sleep(1);
        }
}


static void help(void)
{
        int k;
        printf(
"Usage: pcm [OPTION]... [FILE]...\n"
"-h,--help      help\n"
"-D,--device    playback device\n"
"-r,--rate      stream rate in Hz\n"
"-c,--channels  count of channels in stream\n"
"-f,--frequency sine wave frequency in Hz\n"
"-b,--buffer    ring buffer size in us\n"
"-p,--period    period size in us\n"
"-m,--methodRX    transfer methodRX\n"
"-o,--format    sample format\n"
"-v,--verbose   show the PCM setup parameters\n"
"-n,--noresample  do not resample\n"
"-e,--pevent    enable poll event after each period\n"
"\n");
        printf("Recognized sample formats are:");
        for (k = 0; k < SND_PCM_FORMAT_LAST; ++k) {
                const char *s = snd_pcm_format_name(k);
                if (s)
                        printf(" %s", s);
        }
        printf("\n");
 /*       printf("Recognized transfer methodRXs are:");
        for (k = 0; transfer_methodRXs[k].name; k++)
                printf(" %s", transfer_methodRXs[k].name);
        printf("\n");
*/
}


static void generate_sine(const snd_pcm_channel_area_t *areas, 
                          snd_pcm_uframes_t offset,
                          int count, double *_phase)
{
        static double max_phase = 2. * M_PI;
        double phase = *_phase;
        double step = max_phase*freqRX/(double)rate;
        unsigned char *samples[channels];
        int steps[channels];
        unsigned int chn;
        int format_bits = snd_pcm_format_width(format);
        unsigned int maxval = (1 << (format_bits - 1)) - 1;
        int bps = format_bits / 8;  // bytes per sample 
        int phys_bps = snd_pcm_format_physical_width(format) / 8;
        int big_endian = snd_pcm_format_big_endian(format) == 1;
        int to_unsigned = snd_pcm_format_unsigned(format) == 1;
        int is_float = (format == SND_PCM_FORMAT_FLOAT_LE ||
                        format == SND_PCM_FORMAT_FLOAT_BE);
        // verify and prepare the contents of areas 
        for (chn = 0; chn < channels; chn++) {
                if ((areas[chn].first % 8) != 0) {
                        printf("areas[%i].first == %i, aborting...\n", chn, areas[chn].first);
                        exit(EXIT_FAILURE);
                }
		//		samples[chn] = (signed short *)(((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
                samples[chn] = (((unsigned char *)areas[chn].addr) + (areas[chn].first / 8));
                if ((areas[chn].step % 16) != 0) {
                        printf("areas[%i].step == %i, aborting...\n", chn, areas[chn].step);
                        exit(EXIT_FAILURE);
                }
                steps[chn] = areas[chn].step / 8;
                samples[chn] += offset * steps[chn];
        }
        // fill the channel areas 
        while (count-- > 0) {
                union {
                        float f;
                        int i;
                } fval;
                int res, i;
                if (is_float) {
                        fval.f = sin(phase);
                        res = fval.i;
                } else
                        res = sin(phase) * maxval;
                if (to_unsigned)
                        res ^= 1U << (format_bits - 1);
                for (chn = 0; chn < channels; chn++) {
                        // Generate data in native endian format 
                        if (big_endian) {
                                for (i = 0; i < bps; i++)
                                        *(samples[chn] + phys_bps - 1 - i) = (res >> i * 8) & 0xff;
                        } else {
                                for (i = 0; i < bps; i++)
                                        *(samples[chn] + i) = (res >>  i * 8) & 0xff;
                        }
                        samples[chn] += steps[chn];
                }
                phase += step;
                if (phase >= max_phase)
                        phase -= max_phase;
        }
        *_phase = phase;

}




int init_toneCOMPLEXRX(_Complex float *table, int length, float ref_freq, float gain, float sampl_freq){
	int i;
	double arg=PIx2/((float)length);

	for (i=0;i<length;i++) {
		__real__ table[i]=(float)cos(arg*(float)i);
		__imag__ table[i]=(float)sin(arg*(float)i);
	}
	return(1);
}

int gen_toneCOMPLEXRX(_Complex float *func_out, _Complex float *tablel, int tablesz, 
					int datalen, float ref_freq, float tone_freq, float sampl_freq){
	int i, k=1;
	static int j=0;
	float ref_freq_c;

	ref_freq_c=sampl_freq/(float)tablesz;
	k=(int)(tone_freq/ref_freq_c);
	for (i=0;i<datalen;i++) {
		func_out[i] = tablel[j];
		j += k;
		if(j>=tablesz)j-=tablesz;
	}
	return(1);
}

void CPLXconvert2pcm8bitsstreamRX(_Complex float *in, signed char *out, int numframesIN){
	
	int i;

	for(i=0; i<numframesIN; i++){
		*(out+2*i)=(signed char)(((int)((__real__ *(in+i))*127))&0xFF);
		*(out+2*i+1)=(signed char)(((int)((__imag__ *(in+i))*127))&0xFF);
	}
}


void CPLXconvert2pcm16bitsstreamRX(_Complex float *in, signed short *out, int numframesIN){
	
	int i;

	for(i=0; i<numframesIN; i++){
		*(out+2*i)=(signed short)(((int)((__real__ *(in+i))*32767))&0xFFFF);
		*(out+2*i+1)=(signed short)(((int)((__imag__ *(in+i))*32767))&0xFFFF);
	}
}


void pcm16bitsstream2CPLXconvertRX( signed short *in, _Complex float *out, int numframesIN){
	
	int i, j=0;

	for(i=0; i<numframesIN; i++){
		__real__ *(out+i) = (float)*(in+j);
		__imag__ *(out+i) = (float)*(in+j+1);
		j+=2;
	}
}


