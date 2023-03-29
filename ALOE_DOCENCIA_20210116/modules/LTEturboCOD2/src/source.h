
#ifndef _SOURCE_H
#define _SOURCE_H

#include "ctrl_source.h"

void init_source(ctrl_source_t * control, int type, int length);
void bitrand (int len, char * output);
void allzeros (int len, char * output);
void allones (int len, char * output);
void source (ctrl_source_t * control, char * data);

#endif
