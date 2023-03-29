AUTOMAKE_OPTIONS = subdir-objects
OBJDIR=obj/

bin_PROGRAMS = bin/DAC_JACK0

bin_DAC_JACK0_SOURCES = ../src/DAC_JACK0.c
bin_DAC_JACK0_CPPFLAGS = -I../interfaces  -I../../../sw_api/include -I../../../hw_api/lnx/include
bin_DAC_JACK0_LDADD =  -lskeleton $(hwapi) $(swapi)
bin_DAC_JACK0_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib

EXTRA_DIST = ../interfaces/*.h ../doc/ 
