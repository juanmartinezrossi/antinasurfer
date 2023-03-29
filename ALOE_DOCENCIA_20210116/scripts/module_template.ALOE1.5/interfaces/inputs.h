
/** set your input buffer size */
#define INPUT_MAX_DATA	(1024*1024)

/** default input interface */
typedef char input1_t;
input1_t input_data[INPUT_MAX_DATA];
int process_input(int len);


/** configure input interfaces */
struct utils_itf input_itfs[] = {
                    
                    {"input",
                    sizeof(input1_t),
                    INPUT_MAX_DATA,
                    input_data,         
                    NULL,
                    process_input},

                    {NULL, 0, 0, 0, 0, 0}};

/** =============== End ================  */





