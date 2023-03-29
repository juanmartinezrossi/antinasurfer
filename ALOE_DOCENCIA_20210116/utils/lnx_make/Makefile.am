AUTOMAKE_OPTIONS = subdir-objects


lib_LIBRARIES = lib/libutils.a
lib_libutils_a_CFLAGS = -ansi -fPIC
lib_libutils_a_CPPFLAGS = -I../include -I../../hw_api/lnx/include
lib_libutils_a_SOURCES = ../src/cfg_parser.c ../src/set.c ../src/str.c ../src/net_utils.c ../src/strdata.c ../src/params.c ../src/time_utils.c ../src/print_utils.c ../src/bits_utils.c
#lib_libutils_a_LIBADD = -lrt 

EXTRA_DIST=../include/*.h
