/* hwapi_linux_var.c
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
/**
 * @file phal_hw_api.c
 * @brief ALOE Hardware Library LINUX Implementation
 *
 * This file contains the implementation of the HW Library for
 * linux platforms.
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <setjmp.h>

#define __USE_GNU
#include <ucontext.h>
#include <execinfo.h>

#include "mem.h"
#include "phal_hw_api.h"
#include "hwapi_backd.h"

/** hw_api general database */
extern struct hw_api_db *hw_api_db;

extern sem_t *sem_var;

/** save my proc pointer */
extern struct hwapi_proc_i *my_proc;

/** shared vars db */
extern struct hwapi_var_i *var_db;


void hwapi_var_printlist(int nof_vars){
	int i;

//	printf("hwapi_var_list()\n");
	printf("%-64s%9s%9s%5s%9s%5s%5s%13s%13s\n", "Name", "obj_id", "stat_id",  "size", "cur_size", "opts", "type", "table_offset", "write_tstamp");
	for(i=1; i<nof_vars; i++){
		if(var_db[i].stat_id==0)break;
		printf("%-64s%9d%9d%5d%9d%5d%5d%13d%13d\n",
				var_db[i].name, var_db[i].obj_id, var_db[i].stat_id, var_db[i].size,
				var_db[i].cur_size, var_db[i].opts, var_db[i].type,
				var_db[i].table_offset, var_db[i].write_tstamp);
	}
}



/** @defgroup stats Global Variable Functions
 *
 * The Hardware API enables processes to share global variables in an easy, fast
 * and efficient way.
 *
 *
 * @{
 */

varid_t hwapi_var_getID(char *name){
	int i, ID=0;

	i=1;
//	printf("hwapi_var_getID():%s\n", name);
    while (i < MAX_VARS && var_db[i].stat_id) {
 //   	printf("%s, %s, %d\n", name, var_db[i].name, var_db[i].stat_id);
    	if(strcmp(name, var_db[i].name)==0){
 //   		printf("Global variable found: ID=%d\n", i);
    		ID=i;
    	}
        i++;
    }
    if (i >= MAX_VARS) {
        printf("HWAPI: Can't find variable name\n");

        ////////////////////////
        if (sem_post(&hw_api_db->sem_var)) {
        	perror("sem_post");
        }

        return -1;
        //////////////////////////
    }
 //   if(ID==0)printf("Global variable '%s' NOT found!!!\n", name);
    return(ID);
}



/** Create a new shared variable.
 *
 * @param size Size of the variable, in bytes
 * @returns Id for the variable, 0 if error
 */
varid_t hwapi_var_create2(char *name, int size, int opts, int type)
{
    varid_t i;
    int offset;

    assert(var_db && name && size >= 0);

    if (sem_wait(&hw_api_db->sem_var)) {
    	perror("sem_wait");
    }

    /* alloc data*/
    offset = mem_alloc(hw_api_db->var_table, size);
    if (!offset) {
        printf("HWAPI: Error allocating space for variable\n");
        if (sem_post(&hw_api_db->sem_var)) {
        	perror("sem_post");
        }
        return -1;
    }

    i = 1;
    while (i < MAX_VARS && var_db[i].stat_id) {
        i++;
    }
    if (i == MAX_VARS) {
        printf("HWAPI: Can't create more variables\n");
        if (sem_post(&hw_api_db->sem_var)) {
        	perror("sem_post");
        }
        return -1;
    }

 /*   printf("A hwapi_var_create2:name=%s, var_db[i].name=%s\n", name, var_db[i].name);*/


    /* fill database */
    var_db[i].stat_id = i;
    strncpy(var_db[i].name, name, VAR_NAME_LEN);

//    printf("B hwapi_var_create2:name=%s, var_db[%d].name=%s\n", name, i, var_db[i].name);
    var_db[i].obj_id = my_proc?my_proc->obj_id:0;
    var_db[i].size = size;
    var_db[i].type = type;
    var_db[i].table_offset = offset;
    var_db[i].opts = opts;

    if (i+1>hw_api_db->max_var_id) {
    	hw_api_db->max_var_id=i+1;
    }
    if (sem_post(&hw_api_db->sem_var)) {
    	perror("sem_post");
    }

    /* stat id is position */
    return i;
}
varid_t hwapi_var_create(char *name, int size, int opts) {
	return hwapi_var_create2(name, size, opts, 0);
}

void var_post() {
	if (sem_post(&hw_api_db->sem_var)) {
	    	perror("sem_post");
	    }
}
void var_wait() {
	 if (sem_wait(&hw_api_db->sem_var)) {
	    	perror("sem_wait");
	    }
}
/** Delete a variable
 */
int hwapi_var_delete(varid_t stat_id)
{
	int i;

    assert(var_db);

    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
    	printf("%d: Error deleting variable %d\n",getpid(),stat_id);
        return 0;
    }

    if (sem_wait(&hw_api_db->sem_var)) {
    	perror("sem_wait");
    }
    /* dealloc buffer */
    if (var_db[stat_id].table_offset) {
        mem_free(hw_api_db->var_table, var_db[stat_id].table_offset);
    }

    //memset(&var_db[stat_id], 0, sizeof (struct hwapi_var_i));
    var_db[stat_id].stat_id=0;
    var_db[stat_id].obj_id=0;
    if (sem_post(&hw_api_db->sem_var)) {
    	perror("sem_post");
    }
    /*
    if (stat_id+1==hw_api_db->max_var_id) {
    	for (i=hw_api_db->max_var_id;i>=0;i--) {
    		if (var_db[i].stat_id) {
    			hw_api_db->max_var_id=i+1;
    			break;
    		}
    	}
    }
	*/
    return 1;
}

/** Get a variable option
 */
int hwapi_var_getopt(varid_t stat_id)
{
    assert(var_db);

    if (stat_id < 1 || stat_id > MAX_VARS) {

        return 0;
    }
    if (var_db[stat_id].stat_id==0) {
    	printf("Error getting stat option. Stat has been removed\n\n");
    }

    return var_db[stat_id].opts;
}

int hwapi_var_gettype(varid_t stat_id){
    assert(var_db);

    if (stat_id < 1 || stat_id > MAX_VARS) {

        return 0;
    }
/*    printf("hwapi_var_gettype():StatName=%s, StatType=%d\n", var_db[stat_id].name, var_db[stat_id].type);*/
    if (var_db[stat_id].stat_id==0) {
    	printf("Error getting type option. Stat has been removed\n\n");
    }

    return var_db[stat_id].type;

}


/** List all shared variables.
 *
 */
int hwapi_var_list_opt(int opts, struct hwapi_var_i *var_list, int nof_elems)
{
    int i, k;

    assert(var_db && var_list);
    if (sem_wait(&hw_api_db->sem_var)) {
    	perror("sem_wait");
    }

    k = 0;
    for (i = 1; i < hw_api_db->max_var_id && k < nof_elems; i++) {
        if (var_db[i].stat_id && var_db[i].obj_id && (opts == var_db[i].opts || !opts)) {
            memcpy(&var_list[k], &var_db[i], sizeof (struct hwapi_var_i));
            k++;
        }
    }
    if (sem_post(&hw_api_db->sem_var)) {
    	perror("sem_wait");
    }

    return k;
}
/**This function must be used with caution, since it gives access to kernel space.
 * For avoiding memcpy()s, we set the **var_list ptr to the hwapi structure.
 * This way, a daemon can get inmediate access to the list of variables, but he
 * be sure not to write more than the number of variables returned by the function.
 *
 */
int hwapi_var_getptr(struct hwapi_var_i **var_list) {
	assert (var_list);
	*var_list=var_db;
	return hw_api_db->max_var_id;
}

/** Modify a variable option
 */
int hwapi_var_setopt(varid_t stat_id, int new_opt)
{
    assert(var_db);

    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error setting stat option: Invalid stat id %d\n",stat_id);
        return 0;
    }
    if (var_db[stat_id].stat_id==0) {
    	printf("Error setting stat option. Stat has been removed\n\n");
    }

    var_db[stat_id].opts = new_opt;
    return 1;
}

/** Read a variable value
 * @param value_sz Size of the buffer, In bytes
 */
int hwapi_var_read(varid_t stat_id, void *value, int buff_sz)
{
    int len;

    assert(var_db && value && buff_sz >= 0);

 //   printf("hwapi_var_read(): var_db[ID=%d]=%s\n", stat_id, var_db[stat_id].name);

    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error reading stat: Invalid var id %d, process %d (nof stats=%d)\n",stat_id,getpid(),hw_api_db->max_var_id);
        return -1;
    }

    len = var_db[stat_id].cur_size;
    if (len > buff_sz) {
        printf("HWAPI: Caution buffer provided for stat too small %d<%d (pid %d)\n",buff_sz,len,getpid());
        len = buff_sz;
    }

    mem_cpy((mem_o) hw_api_db->var_table, value, var_db[stat_id].table_offset + hw_api_db->var_table, len);

//	printf("hwapi_var_read(). len=%d\n", len);
    return len;
}

int hwapi_var_getsz(varid_t stat_id) {
    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error reading stat size: Invalid stat id %d\n",stat_id);
        return -1;
    }
    return var_db[stat_id].cur_size;
}

int hwapi_var_readts(varid_t stat_id, void *value, int buff_sz, int *tstamp)
{
    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error reading stat ts: Invalid stat id %d\n",stat_id);
        return -1;
    }
    if (tstamp) {
        *tstamp = var_db[stat_id].write_tstamp;
    }
    return hwapi_var_read(stat_id, value, buff_sz);
}

int hwapi_var_get_tstamp(varid_t stat_id)
{

    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error getting stat ts: Invalid stat id %d\n",stat_id);
        return -1;
    }
    return var_db[stat_id].write_tstamp;
}

/** Write a variable value
 * @param value_sz Size of the variable, In bytes
 */
int hwapi_var_write(varid_t stat_id, void *value, int value_sz)
{
    int len;

//printf("A hwapi_var_write\n");
    assert(var_db && value && value_sz >= 0);
//printf("B hwapi_var_write\n");
    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error writting stat: Invalid stat id %d, process %d\n",stat_id,getpid());
        return -1;
    }
//printf("C hwapi_var_write\n");
    len = value_sz;

    if (len >= var_db[stat_id].size) {
        len = var_db[stat_id].size;
    }

    var_db[stat_id].cur_size = len;

 /*AGB   printf("INIT hw_api_db->var_table=%u, END hw_api_db->var_table=%u\n", hw_api_db->var_table, hw_api_db->var_table+32*VAR_TABLE_SZ);
 //   printf("var_db[stat_id].table_offset + hw_api_db->var_table=%u\n", var_db[stat_id].table_offset + hw_api_db->var_table);*/

    mem_cpy((mem_o) hw_api_db->var_table, var_db[stat_id].table_offset + hw_api_db->var_table, value, len);
    return len;
}



int hwapi_var_writemyts(varid_t stat_id, void *value, int value_sz, int tstamp)
{
    if (stat_id < 1 || stat_id > hw_api_db->max_var_id) {
        printf("HWAPI: Error writting stat: Invalid stat id %d, process %d\n",stat_id,getpid());
        return -1;
    }
    if (!my_proc) {
        return -1;
    }
    var_db[stat_id].write_tstamp = tstamp;
    return hwapi_var_write(stat_id, value, value_sz);
}


int hwapi_var_writets(varid_t stat_id, void *value, int value_sz)
{
    return hwapi_var_writemyts(stat_id, value, value_sz, my_proc->obj_tstamp);
}


/* @} */
