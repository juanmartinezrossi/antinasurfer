AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = bin/modulename

bin_modulename_SOURCES = ../src/modulename_imp.c
bin_modulename_CPPFLAGS = -I../interfaces -I../../typetools/include/ -I../../../sw_api/include
bin_modulename_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib
bin_modulename_LDADD =  -lskeleton -lsw_api -lhw_api -ltypetools -lm

EXTRA_DIST = ../interfaces/*.h ../doc/ ../src/*.h
