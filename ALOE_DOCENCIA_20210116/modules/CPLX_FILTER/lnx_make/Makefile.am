AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/CPLX_FILTER

# sources.
# ADD here extra source files
bin_CPLX_FILTER_SOURCES = ../aloe/skeleton/skeleton.c ../aloe/skeleton/params.c ../aloe/extern/rtdal_datafile.c ../standalone/test_generate.c ../src/CPLX_FILTER_functions.c ../src/CPLX_FILTER.c 

#pre-processor flags, e.g. includes
bin_CPLX_FILTER_CPPFLAGS = -D_COMPILE_ALOE -I../interfaces -I../src/ -I../aloe/skeleton -I../aloe/extern -I../../typetools/include/ -I../../../sw_api/include -I../../../utils/include
#linker flags: library paths (do not modify)
bin_CPLX_FILTER_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib
#libraries to include (do not modify)
bin_CPLX_FILTER_LDADD =  -lfftw3 -lm -lskeleton $(hwapi) $(swapi) -lpthread -lutils

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../include/*.h ../doc/ ../src/*.h ../standalone/*.h ../../../utils/include/*.h
