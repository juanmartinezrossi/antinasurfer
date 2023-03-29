#https://sites.google.com/site/michaelsafyan/software-engineering/how-to-write-a-makefile
SHELL = /bin/sh
# Change cpp and linker flags here
#CPPFLAGS=-D_GNU_SOURCE -D_USE_GNU -O0 -g3 -Wall -Wno-format-extra-args -std=c99 -Winline -I../src -I../test -I../../../aloeSKs/aloeSK15/skel_SK15/src -I../../../aloeSKs/aloeSK15/skel_SK15/include -I../aloe/extern -I../../../aloeSKs/aloeSK15/aloeplot/src -I../../../aloeSKs/aloeSK15/aloeplot/include -I../../typetools/include/ -I../../../sw_api/include -fPIC
#LDFLAGS=-lrt -lm -lfftw3 -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib

CPPFLAGS=-D_GNU_SOURCE -D_USE_GNU -O0 -g3 -Wall -Wno-format-extra-args -std=c99 -Winline -I../src -I../test -I../../../aloeSKs/aloeSK15/skel_SK15/include -I../../../aloeSKs/aloeSK15/extern/datafile/include -I../../../aloeSKs/aloeSK15/aloeplot/include -I../../typetools/include/ -I../../../utils/include -I../../../sw_api/include -fPIC
LDFLAGS=-lrt -lm -lfftw3 -laloeplot -lutils -L../../../utils/lnx_make/lib/ -L../../typetools/lnx_make/lib/ -L../../../hw_api/lnx/lib -L../../../sw_api/lnx_make/lib

#Put here only the module_main.c file 
MODSRC=../src/DUC17.c 
#Add here more .c files required by the module DSP function
#DSPSRC=../src/module_params.c ../src/module_stats.c ../src/DUC17_functions.c
DSPSRC=../src/DUC17_params.c ../src/DUC17_functions.c

#Matlab root path
MATLAB_ROOT=/path/to/matlab
MATLAB_INC=-D_COMPILE_MEX_INCLUDE

#Comment the next line if you have matlab to avoid mexPrintf warnings
MATLAB_INC=

# DO NOT NEED TO MODIFY BELOW HERE ############################################################################33

# Name of the module 
NAME=DUC17

MATLAB_INCLUDE=$(MATLAB_ROOT)/extern/include
CC=gcc
LIBNAME=lib$(NAME).a
#Common modules files: skeleton version
COMSRC=../../../aloeSKs/aloeSK15/skel_SK15/src/paramsSK15.c ../../../aloeSKs/aloeSK15/extern/datafile/src/rtdal_datafile.c 
SASRC=../../../aloeSKs/aloeSK15/standaloneSK15/test_main.c ./test_generate.c  ../../../aloeSKs/aloeSK15/aloeplot/src/gnuplot_i.c ../../../aloeSKs/aloeSK15/aloeplot/src/complexplot.c ../../../aloeSKs/aloeSK15/aloeplot/src/plot_mode.c

SAOBJ=$(SASRC:.c=.o)
COMOBJ=$(COMSRC:.c=.o)
DSPOBJ=$(DSPSRC:.c=.o)
MODOBJ_M=$(MODSRC:.c=.om)
MODOBJ=$(MODSRC:.c=.o)

.PHONY: all clean distclean

all: $(SOURCES) $(NAME) matlab

matlab: $(MODOBJ_M) $(COMOBJ) $(DSPOBJ)
	ar rcs $(LIBNAME) $(MODOBJ_M) $(COMOBJ) $(DSPOBJ)

$(MODOBJ_M):
	$(CC) $(CPPFLAGS) -I$(MATLAB_INCLUDE) $(MATLAB_INC) -D_COMPILE_MEX -c $(MODSRC) -o $(MODOBJ_M)

$(NAME): $(MODOBJ) $(SAOBJ) $(COMOBJ) $(DSPOBJ)		 			
	$(LINK.cc) $(MODOBJ) $(COMOBJ) $(SAOBJ) $(DSPOBJ) $(LDFLAGS) -o $@	

clean:
	@- $(RM) $(LIBNAME) $(NAME)
	@- $(RM) $(MODOBJ) $(DSPOBJ) $(COMOBJ) $(MODOBJ_M) $(SAOBJ)


