#ifndef _SK18EXTVARS_H
#define	_SK18EXTVARS_H
#include "module_SK18_interfaces.h"	//AGB27OCT18

/** aquestes serien les variables params obligatories */


/** Control of printing input/output data array options*/
#define NOPRINT        	0 		/** Non printing choice*/
#define DATARECEIVED   	1 		/** Printing received data according specified type*/
#define DATASENT				2			/** Printing sent data out according specified type*/

#define NUMCOLS         32  	/** Number of printed data columns format*/
#define NUM_TS          0  		/** Initial Printing Time-slot */
#define NUM_TS_PRINTS   6   	/** Number of printing Time-slots*/
#define PRINT_OFFSET		0			/** Pointer offset to print */
#define PRINT_MAXLEN		128		/** maximum number of samples to print */

/** estas definint un tipus, no pots inicialitzarlo aqui sino al declarar la variable */
struct printopt {
    int mode;		/** Options: NOPRINT, DATARECEIVED, DATASENT			*/
				/** Usage: .mode = NOPRINT; 						Printing Nothing*/
				/** Usage: .mode = DATARECEIVED||DATASENT;	Printing all	*/
				/** Usage: .mode = DATARECEIVED;					Printing input only*/
    int ncolums;		/** Number of printed data columns format*/
    int TS2print;	/** Initial Printing Time-slot */
    int nprintTS;	/** Number of printing Time-slots*/
    int offset;		/** pointer offset to print */ 
    int maxlen;		/** maximum number of samples to print */
};

struct printopt printopts = {DATARECEIVED||DATASENT,NUMCOLS,NUM_TS,NUM_TS_PRINTS,PRINT_OFFSET,PRINT_MAXLEN};
//extern struct printopt printopts;

/* el input lengh l'he fet opcional, si no es defineix al params o es defineix amb un valor 0,
 * agafa totes les mostres d'entrada. Al contrari, agafa només les mostres que necessita.
 */


struct utils_param params[] = {
    /** aixo son parametres no stats, hi estas d'acord ? */
    {"print",STAT_TYPE_INT,1, &printopts.mode},
    {"printncolums",STAT_TYPE_INT,1, &printopts.ncolums},
    {"TS2print",STAT_TYPE_INT,1, &printopts.TS2print},
    {"NumTS2print",STAT_TYPE_INT,1, &printopts.nprintTS},
    {"offset",STAT_TYPE_INT,1, &printopts.offset},
    {"maxlen",STAT_TYPE_INT,1, &printopts.maxlen},
    {NULL, 0, 0, 0}};




/** no toquis res a partir d'aqui */


#define MAX_VARS 100		

int stat_ids[NUM_INPUT_ITFS+NUM_OUTPUT_ITFS+MAX_VARS];
struct utils_stat stats[NUM_INPUT_ITFS+NUM_OUTPUT_ITFS+1+MAX_VARS];
struct utils_itf input_itfs[NUM_INPUT_ITFS+1];
struct utils_itf output_itfs[NUM_OUTPUT_ITFS+1];

#endif
