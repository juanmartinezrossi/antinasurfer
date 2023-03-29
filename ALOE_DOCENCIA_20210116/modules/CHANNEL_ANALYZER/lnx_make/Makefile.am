AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/CHANNEL_ANALYZER

# sources.
# ADD here extra source files
bin_CHANNEL_ANALYZER_SOURCES = ../aloe/skeleton/skeleton.c ../aloe/skeleton/params.c ../aloe/extern/rtdal_datafile.c ../standalone/test_generate.c ../src/CHANNEL_ANALYZER_functions.c ../src/CHANNEL_ANALYZER.c 

#pre-processor flags, e.g. includes
bin_CHANNEL_ANALYZER_CPPFLAGS = -D_COMPILE_ALOE -I../interfaces -I../src/ -I../aloe/skeleton -I../aloe/extern -I../../typetools/include/ -I../../../sw_api/include -I../../../utils/include
#linker flags: library paths (do not modify)
bin_CHANNEL_ANALYZER_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib
#libraries to include (do not modify)
bin_CHANNEL_ANALYZER_LDADD =  -lfftw3 -lskeleton $(hwapi) $(swapi) -lpthread -lutils

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../include/*.h ../doc/ ../src/*.h ../standalone/*.h ../../../utils/include/*.h
