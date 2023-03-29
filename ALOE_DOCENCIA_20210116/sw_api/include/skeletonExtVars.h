
/** aquestes serien les variables params obligatories */


/** Control of printing input/output data array options*/
#define NOPRINT        	0 	/** Non printing choice*/
#define DATARECEIVED   	1 	/** Printing received data according specified type*/
#define PROCESS_IN	2	/** Printing processing data in according specified type*/
#define PROCESS_OUT	3	/** Printing processing data out according specified type*/
#define DATASENT	4	/** Printing sent data out according specified type*/

#define NUMCOLS         32  	/** Number of printed data columns format*/
#define NUM_TS          0  	/** Initial Printing Time-slot */
#define NUM_TS_PRINTS   2   	/** Number of printing Time-slots*/
#define PRINT_OFFSET	0	/** Pointer offset to print */
#define PRINT_MAXLEN	128	/** maximum number of samples to print */

/** estas definint un tipus, no pots inicialitzarlo aqui sino al declarar la variable */
struct printopt {
    int mode;		/** Options: NOPRINT, DATARECEIVED, PROCESS_IN, PROCESS_OUT,DATASENT			*/
				/** Usage: .mode = NOPRINT; 						Printing Nothing*/
				/** Usage: .mode = DATARECEIVED||PROCESS_IN||PROCESS_OUT||DATASENT;	Printing all	*/
				/** Usage: .mode = DATARECEIVED;					Printing input only*/
				/** Usage: .mode = DATARECEIVED||PROCESS_IN;				Printing inputs*/	
    int ncolums;		/** Number of printed data columns format*/
    int TS2print;	/** Initial Printing Time-slot */
    int nprintTS;	/** Number of printing Time-slots*/
    int offset;		/** pointer offset to print */ 
    int maxlen;		/** maximum number of samples to print */
};

struct printopt printopts = {NOPRINT,NUMCOLS,NUM_TS,NUM_TS_PRINTS,PRINT_OFFSET,PRINT_MAXLEN};


/* el input lengh l'he fet opcional, si no es defineix al params o es defineix amb un valor 0,
 * agafa totes les mostres d'entrada. Al contrari, agafa només les mostres que necessita.
 */

int inputdatalength=0;

int transporttypeIN=0;
int transporttypeOUT=0;

float inputgain=1.0;
float outputgain=1.0;

/** special transport types for interfaces can be configured in params:
 *
 * input_datatype_0 = <type>
 */
#define INPUT_TYPE_STR "input_datatype"
#define OUTPUT_TYPE_STR "output_datatype"

/** Special input lengths for interfaces can be configured in params:
 *
 * input_length_0 = <length>
 */
#define INPUT_LENGTH_STR "input_length"

/** Special gains for interfaces can be configured in params:
 *
 * input_gain_0 = <length>
 */
#define INPUT_GAIN_STR "input_gain"
#define OUTPUT_GAIN_STR "output_gain"


struct utils_param params[] = {

    {INPUT_TYPE_STR,STAT_TYPE_INT,1, &transporttypeIN},
    {OUTPUT_TYPE_STR,STAT_TYPE_INT,1, &transporttypeOUT},
    {INPUT_LENGTH_STR,STAT_TYPE_INT,1, &inputdatalength},

    {INPUT_GAIN_STR,STAT_TYPE_FLOAT,1, &inputgain},
    {OUTPUT_GAIN_STR,STAT_TYPE_FLOAT,1, &outputgain},

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


