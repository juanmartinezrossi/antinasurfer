AUTOMAKE_OPTIONS = subdir-objects

# name of the program
bin_PROGRAMS = bin/module_templateSK18

# sources.
# ADD here extra source files 
bin_module_templateSK18_SOURCES = ../../../sw_api/src/skeletonExtSK18.c ../src/module_templateSK18_functions.c ../src/module_templateSK18.c

#pre-processor flags, e.g. includes
bin_module_templateSK18_CPPFLAGS = -I../../../sw_api/include -I../../../utils/include -I../src -I../interfaces -I../../typetools/include/ -I../include -I../../../sw_api/include -I../../../sw_daemons/include

#linker flags: library paths (do not modify)
bin_module_templateSK18_LDFLAGS = -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib

#libraries to include (do not modify)
bin_module_templateSK18_LDADD =  -lskeleton -lfftw3 -lsw_api -lhw_api -ltypetools -lm -lutils -lpthread

# include these extra files in the distribution
# IMPORTANT: Add ../src/*.h files if you include headers in the src/ directory
EXTRA_DIST = ../doc/ ../../../utils/include/*.h ../sw_api/include/*.h


