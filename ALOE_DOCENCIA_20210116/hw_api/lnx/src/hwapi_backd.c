/*
 * hwapi_backd.c
 *
 * Copyright (c) 2009 Ismael Gomez-Miguelez, UPC <ismael.gomez at tsc.upc.edu>
 * All rights reserved.
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



/** System includes */
#define _GNU_SOURCE
#include <pthread.h>
#define _SVID_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sched.h>
#include <time.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/msg.h>

/** HWAPI includes */
#include "phal_hw_api.h"
#include "hwapi_backd.h"
#include "hwapi_netd.h"

/** Main HWAPI structure */
struct hw_api_db *hw_api_db = NULL;
/** this is same but for using before shared memory creation */
struct hw_api_db *tmp_db;

/** Main process pid */
pid_t runph_pid;

/** Do not print messages on process death if die_silent=0 */
int die_silent;


/** Exit level indicator */
int exit_level = 0;

/* Defines for exit levels */
#define ELEVEL_PROCESSES 3
#define ELEVEL_MEM	 2
#define ELEVEL_LOCK	 1

/** flag to avoid removing lock file if runph crashes while creating daemon */
int creating_daemon = 0;

/** do verbose and do trace prints */
int verbose=0;
int trace=0;

/** launch daemons with valgrind */
int valgrind=0;

/** save old process group id */
int old_pgid;

/** command receives thread */
pid_t command_pid,dac_pid;

/** List of sw daemons */
char *daemon_names[]={"exec","bridge","stats","frontend","swload","sync","swman","hwman","statsman","sync_master","cmdman"};

/** main kernel thread */
pthread_t runph_thread;

/************************************************************
 * 					HELPER FUNCTIONS 						*
 ************************************************************/


/* print frame function */
void print_frame(char *title) {
	printf("======================================\n");
	printf("     -= %s =-               \n", title);
	printf("======================================\n");

}

/** Print usage */
void usage(char *prog) {
	printf(
			"%s -r [repository_path] -c [platform_cfg_file] -i [xitf_cfg_file] --debug -v [--daemon] [-o output_log]\n",
			prog);
	printf("\n");
	printf("\t -r [repository_path]: Path for file resources (for manager daemons)\n");
	printf("\t -c [platform_config_file]: Platform configuration file\n");
	printf("\t -f Force deletetion of previous session\n");
	printf(
			"\t --daemon: Run as a daemon. You should select log file for each daemon in platform config file.\n");
	printf(
			"\t -o [output_log]: Redirect hwapi stdout to a file, useful when runnin as daemon.\n");
	printf("\t -t [num_processors] [number_of_mac_operations to do]: Runs number_of_mac_operations and compute MACPS performance.\n");
	printf("\t --valgrind: Enable valgrind (setup at scheduling config file).\n");
	printf("\t --debug: Run in debug mode.\n");
	printf("\t --trace: Print timing info to kernel trace.\n");
	printf("\t -v: Run in verbose mode.\n");
	exit(0);
}


/** Check clock granularity */
int gettimerres(void) {
	struct timespec ts;

	if (clock_getres(CLOCK_MONOTONIC, &ts))
		return 1;

	return ts.tv_nsec;
}



/** Exit function
 *  Tries to remove allocated resources and lock file
 */
void exit_runph() {
	int sleep_time;
	void *ret_val;
	int n;

	/** only parent process run this function */
	if (getpid() != runph_pid || pthread_self()!=runph_thread) {
		return;
	}

	printva("Exiting with exit_level %d\n",exit_level);

	sleep_time = 500;

	/* do not print dying messages during normal exit */
	if (verbose)
		die_silent = 1;
	else
		die_silent = 0;

	if (dac_pid) {
		printva("Killing DAC pid %d\n",dac_pid);
		kill(dac_pid,SIGTERM);
		waitpid(dac_pid,NULL,0);
		printv("DAC Died\n");
		dac_pid=0;
	}
	if (command_pid) {
		printva("Killing kernel commands process %d\n",command_pid);
		kill(command_pid,SIGTERM);
		waitpid(command_pid,NULL,0);
		printv("CMD Died\n");
		command_pid=0;
	}

	/** Kill all childs if created */
	if (exit_level >= ELEVEL_PROCESSES) {
		printv("Killing chlds...\n");
		kill_chlds(sleep_time);
		printv("Clear zombies...\n");
		clear_zombies();
		printv("Clear files...\n");
		clear_files();
	}

	/** Clean shared memory, if created */
	if (exit_level >= ELEVEL_MEM) {
		printv("cleaning shared memory resources...\n");
		clear_shm();
	}

	/** Remove lock file and remaining message mailbox */
	if (exit_level >= ELEVEL_LOCK && !creating_daemon) {
		printv("Removing lock file\n");
		/** This function makes sure we remove all msg mailbox associated to the lock file */
		purge_msg();
		if (remove(LOCK_FILE)) {
			perror("remove");
		}
	}
	printf("\nBye.\n");fflush(stdout);
}


/************************************************************
 * 					LOCK FILE FUNCTIONS						*
 ************************************************************/

/** Lock main file for shared resources
 *
 */
int lock_file_open() {
	int fd;

	printv("Creating lock file...\n");
	fd = open(LOCK_FILE, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
	if (fd < 0) {
		if (errno == EEXIST) {
			printf(	"Another ALOE session is currently opened with lock %s. \n"
						"Kill it or launch with -f flag to force previous session clean up.\n",
						LOCK_FILE);
			return 0;
		} else {
			printf("Error creating %s\n", LOCK_FILE);
			perror("open");
			return 0;
		}
	}
	/** close file after creation */
	close(fd);
	printv("Done\n");
	return 1;
}

/** Write pid to lock file */
int lock_file_write_pid(int pid) {
	FILE *f;
	f = fopen(LOCK_FILE, "w");
	if (!f) {
		printf("Error opening lock file  %s\n",LOCK_FILE);
		perror("fopen");
		return 0;
	}
	fprintf(f, "%d", pid);
	fclose(f);
	return 1;
}

/** This function is called with -f argument and removes resources left from previous sessions */
int lock_file_force_clean() {
	int i, fd, n;
	int pid;
	key_t key;

	printf("Cleaning resources associated to lock: %s\n", LOCK_FILE);
	fd = open(LOCK_FILE, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);
	if (fd > 0) {
		close(fd);
	}

	printf("Cleaning shared memory...\n");
	if (!clear_shm()) {
		printf("Error cleaning shared memory\n");
	}

	/* try to remove more resources */
	printf("Cleaning messages...\n");
	purge_msg();

	printf("Removing File...\n");
	if (remove(LOCK_FILE)) {
		perror("remove");
		return 0;
	} else {
		printf("Ok.\n");
	}
	return 1;
}


/************************************************************
 * 			FUNCTIONS CALLED FROM MAIN	 					*
 ************************************************************/


int go_background() {
	char *file;
	printv("Creating background process\n");

	if (tmp_db->output_file[0] == '\0') {
		printf("KERNEL:\tCaution you didn't select an output file and I'm going background. Output will be lost.\n");
		file=NULL;
	} else {
		printf(	"KERNEL:\tGoing background... Output is being redirected to %s\n", tmp_db->output_file);
		file=tmp_db->output_file;
	}
	creating_daemon = 1;
	run_daemon(file);
	creating_daemon = 0;
	return 1;
}

void start_network() {
	/* Init network thread */
	hw_api_db->net_db.net_chld = 0;
	if (hw_api_db->net_db.nof_ext_itf) {
		printv("Creating network threads\n");
		Net_thread(hw_api_db, hw_api_db->data_msg_id);
		printv("Done\n");
	}
}

void setup_dac() {

//	printf("...........................................hwapi_backd.c==>setup_dac()\n");
//	printf("hw_api_db->dac_itf=%s\n", hw_api_db->dac_itf);
	if (hw_api_db->dac_itf != NULL) {
		printv("Configuring DAC\n");
		/* configure dac */
		if (!configure_dac(hw_api_db->dac_itf, &hw_api_db->dac, &hw_api_db->cpu_info.tslen_usec)) {
			printf("KERNEL:\tError configuring DAC.Exiting...\n");
			exit(-1);
		}
		printv("Done\n");
	}

	if (!hw_api_db->cpu_info.tslen_usec) {
		printf("KERNEL:\tError can't run with tslot 0\n");
		exit(-1);
	} else {
		printf("KERNEL:\tTime slot duration set to %d usec\n", hw_api_db->cpu_info.tslen_usec);
	}
}

void setup_shm() {

	printv("Setting up Shared memory\n");

	create_shm();

	/** Save temporal db to shared memory */
	memcpy(hw_api_db, tmp_db, sizeof(struct hw_api_db));
	free(tmp_db);

	/** Initialize shared memory */
	init_shm_db();

	printv("Done\n");

}

void setup_config_files(int argc, char **argv) {
	char *filename;
	struct stat st;
	int i;

	for (i=0;i<MAX_DAEMONS;i++) {
		strcpy(tmp_db->daemons[i].path,daemon_names[i]);
		tmp_db->daemons[i].cpuid=-1;
		tmp_db->daemons[i].runnable=0;
	}

	/** get platform configuration file name  */
	if ((i=arg_find(argc,argv,"-c"))>0) {
		filename = argv[i + 1];
	} else {
		filename = DEFAULT_DAEMONS_FILE;
	}
	/* ... and parse it */
	if (!parse_platform_file(tmp_db, filename)) {
		exit(0);
	}

	/** Daemon or no-dameon arguments overwrites platform conf options */
	if (arg_find(argc,argv,"--daemon")) {
		tmp_db->run_as_daemon=1;
		printf("KERNEL:\tRunning as a daemon...\n");
	} else if (arg_find(argc,argv,"--no-daemon")) {
		printv("Running in foreground mode\n");
		tmp_db->run_as_daemon=0;
	}

	/** set mode debugging */
	if ((tmp_db->cpu_info.debug_level=arg_find(argc,argv,"--debug"))>0) {
		printf("KERNEL:\tRunning in DEBUG mode\n");
	}

	/** overwrites platform conf resource path */
	if ((i=arg_find(argc,argv,"-r"))>0) {
		strncpy(tmp_db->res_path, argv[i + 1], DEF_FILE_LEN);
	}
	/** Check if resource path exists */
	if (stat(tmp_db->res_path, &st)) {
		printf("Working path %s not found\n", tmp_db->res_path);
		exit(0);
	} else {
		printf("KERNEL:\tUsing working path: %s\n", tmp_db->res_path);
	}

	/** overwrites output file in platform conf */
	if ((i=arg_find(argc,argv,"-o"))>0) {
		strncpy(tmp_db->output_file, argv[i + 1], DEF_FILE_LEN);
	}
	if (tmp_db->output_file[0] != '\0' && tmp_db->run_as_daemon) {
		printf("KERNEL:\tRedirecting output to %s\n", tmp_db->output_file);
	} else {
		printv("Output is not redirected\n");
	}

	/** Check platform configuration and set default values */

	if (!tmp_db->cpu_info.nof_cores) {
		printf("KERNEL:\tCAUTION missing number of cores parameter. Setting to 1 core.\n");
		tmp_db->cpu_info.nof_cores = 1;
	}
	if (!tmp_db->cpu_info.C) {
		printf(	"KERNEL:\tCAUTION no CPU info was entered in daemons cfg. Setting default.\n");
		tmp_db->cpu_info.C = DEFAULT_CPU_MIPS;
	}

	/** Print platform configuration */
	printf("KERNEL:\tLocal Computing Capacity: %.1f MOP/s %d cores\n", tmp_db->cpu_info.C, tmp_db->cpu_info.nof_cores);
	printf("KERNEL:\tInternal Bandwidth: %.1f MBps\n", 	tmp_db->cpu_info.intBW);

}

/************************************************************
 * 					ARGUMENT PARSING FUNCTIONS 				*
 ************************************************************/
int arg_find(int argc, char **argv, char *param) {
	int i;

	for (i = 1; i < argc; i++) {
			if (!strncmp(param,argv[i],strlen(param))) {
				return i;
			}
	}
	return 0;
}

/************************************************************
 * 					MAIN 					 				*
 ************************************************************/


int main(int argc, char **argv) {

	int i;

	/** check if running verbose or trace */
	verbose = arg_find(argc,argv,"-v");
	trace = arg_find(argc,argv,"--trace");
	valgrind = arg_find(argc,argv,"--valgrind");
	die_silent = 1;

	/** print usage */
	if (arg_find(argc,argv,"-h")) {
		usage(argv[0]);
		exit(0);
	}

	/** check if run mac test */
	if ((i=arg_find(argc,argv,"-t"))>0) {
		if ((argc - 2) > i) {
			run_mactest(atoi(argv[i + 1]), atoi(argv[i + 2]));
			exit(0);
		} else {
			printf("Switch '-t' needs number of processors\n");
			usage(argv[0]);
		}
	}

	/** run force clean */
	if (arg_find(argc,argv,"-f")) {
		if (!lock_file_force_clean()) {
			printf("Could not clean previous lock. Please do it manually.\n");
		}
		exit(0);
	}

	/** Allocate local memory for hwapi db */
	printv("Allocating temporal memory for hwapi db\n");
	tmp_db = calloc(1, sizeof(struct hw_api_db));
	if (!tmp_db) {
		printf("KERNEL:\tError allocating temporal memory\n");
		perror("calloc");
		exit(0);
	}

	tmp_db->cpu_info.verbose=verbose;
	tmp_db->cpu_info.trace=trace;

	/** First of all, check timing resolution */
	printv("Checking timer resolution...\n");
	if (gettimerres() > 1) {
		printf(	"\nKERNEL:\tWARNING: High resolution timers not available.\nCurrent resolution=%d\n\n",gettimerres());
	}

	/* get lock file now */
	if (!lock_file_open()) {
		printf("Error getting lock. Exiting...\n");
		exit(0);
	}

	/** Register exit function and set initial exit level*/
	atexit(exit_runph);
	runph_pid = getpid();
	exit_level=ELEVEL_LOCK;

	printv("Reading configuration files\n");
	setup_config_files(argc,argv);
	printv("Done\n");

	/* go background */
	if (tmp_db->run_as_daemon) {
		if (!go_background()) {
			exit(0);
		}
		/* update my pid */
		runph_pid = getpid();
	}

	/* write final pid to lock file */
	printv("Writing pid to lock file\n");
	if (!lock_file_write_pid(runph_pid)) {
		exit(0);
	}

	exit_level = ELEVEL_MEM;
	setup_shm(); /** From now over, use hw_api_db instead of tmp_db */

	/*******************************
	 *
	 * From now on, we use hw_api_db instead of tmp_db
	 *
	 * *****************************
	 */

	/** set real-time, but do not exit if error */
	if (!set_realtime(0, hw_api_db->kernel_prio, hw_api_db->kernel_cpuid)) {
		printf("KERNEL:\tWarning, running in non real-time mode!!\n");
	}

	if (!signals_setup()) {
		exit(0);
	}

	hw_api_db->runph_pid = runph_pid;
	//old_pgid=getpgrp();
	//setpgid(getpid(),getpid());
	//hw_api_db->runph_grp=getpgrp();

	/** Create the thread for processing commands */
	if (!kernel_commands_thread()) {
		exit(0);
	}

	exit_level = ELEVEL_PROCESSES;
	start_network();
	start_daemons();

	/** setup digital converter access */
	setup_dac();

	sleep(1);

	runph_thread=pthread_self();

	printva("main is thread %d\n",pthread_self());

	printv("Running main timer\n");
	run_main_timer();

	/** this is never reached */
	return (1);
}




