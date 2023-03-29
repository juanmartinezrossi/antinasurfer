AUTOMAKE_OPTIONS = subdir-objects

lib_LIBRARIES = lib/libtypetools.a lib/libfiletools.a lib/libDSP_tools.a

lib_libDSP_tools_a_SOURCES = ../src/DSP_tools.c 
lib_libDSP_tools_a_CPPFLAGS = -I../include/ -I../../../sw_api/include -I../../../hw_api/lnx/include

lib_libtypetools_a_SOURCES = ../src/typetools.c 
lib_libtypetools_a_CPPFLAGS = -I../include/ -I../../../sw_api/include -I../../../hw_api/lnx/include

lib_libfiletools_a_SOURCES = ../src/utils.c
lib_libfiletools_a_CPPFLAGS = -I../include/ -I../../../sw_api/include -I../../../hw_api/lnx/include

EXTRA_DIST = ../doc/ ../include/*.h  


