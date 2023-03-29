#ifndef TIME_UTILS_H_
#define TIME_UTILS_H_


#include <time.h>
typedef struct timespec mytime_t;

int get_time_r(mytime_t *x);
void get_time_interval_r(mytime_t *tdata);

#endif /*TIME_UTILS_H_*/
