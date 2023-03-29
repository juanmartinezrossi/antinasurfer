AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/CIRC_BUFFER

# sources.
# ADD here extra source files
bin_CIRC_BUFFER_SOURCES = ../../../aloeSKs/aloeSK15/skel_SK15/src/paramsSK15.c  ../../../aloeSKs/aloeSK15/skel_SK15/src/skeletonSK15.c ../src/CIRC_BUFFER_params.c ../src/CIRC_BUFFER_functions.c ../src/CIRC_BUFFER.c 

#pre-processor flags, e.g. includes
bin_CIRC_BUFFER_CPPFLAGS = -D_COMPILE_ALOE -I../src/ -I../../typetools/include/ -I../../../utils/include -I../../../aloeSKs/aloeSK15/skel_SK15/include

#linker flags: library paths (do not modify)
bin_CIRC_BUFFER_LDFLAGS = -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib -L../../../aloeSKs/aloeSK15/aloeplot/lnx_make/lib -L../../../aloeSKs/aloeSK15/extern/datafile/lnx_make/lib

#libraries to include (do not modify)
bin_CIRC_BUFFER_LDADD =  -lskeleton -lfftw3 -lm -ltypetools -lfiletools -lDSP_tools $(hwapi) $(swapi) -lpthread -lutils -laloeplot -lrtdalfile

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../include/*.h ../doc/ ../src/*.h ../../../utils/include/*.h 
