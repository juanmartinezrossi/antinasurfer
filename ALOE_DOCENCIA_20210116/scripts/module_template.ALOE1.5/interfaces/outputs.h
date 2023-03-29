

/** set your output buffer size */
#define OUTPUT_MAX_DATA	(1024*1024)


/** default output interface */
typedef char output1_t;
output1_t output_data[OUTPUT_MAX_DATA];


/** configure output interfaces */
struct utils_itf output_itfs[] = {
                {"output",
                 sizeof(output1_t),
                 OUTPUT_MAX_DATA,
                 output_data,
                 NULL,
                 NULL
                },

                {NULL, 0, 0, 0, 0, 0}};

