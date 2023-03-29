AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/module_template4

# sources.
# ADD here extra source files
bin_module_template4_SOURCES = ../../../aloeSKs/aloeSK15/skel_SK15/src/paramsSK15.c  ../../../aloeSKs/aloeSK15/skel_SK15/src/skeletonSK15.c ../src/module_template4_params.c ../src/module_template4_functions.c ../src/module_template4.c 

#pre-processor flags, e.g. includes
bin_module_template4_CPPFLAGS = -D_COMPILE_ALOE -I../src/ -I../../typetools/include/ -I../../../utils/include -I../../../aloeSKs/aloeSK15/skel_SK15/include

#linker flags: library paths (do not modify)
bin_module_template4_LDFLAGS = -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib -L../../../aloeSKs/aloeSK15/aloeplot/lnx_make/lib -L../../../aloeSKs/aloeSK15/extern/datafile/lnx_make/lib

#libraries to include (do not modify)
bin_module_template4_LDADD =  -lskeleton -lfftw3 -lm -ltypetools -lfiletools -lDSP_tools $(hwapi) $(swapi) -lpthread -lutils -laloeplot -lrtdalfile

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../include/*.h ../doc/ ../src/*.h ../../../utils/include/*.h 
