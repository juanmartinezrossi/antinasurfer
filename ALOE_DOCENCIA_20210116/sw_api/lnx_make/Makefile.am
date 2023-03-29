AUTOMAKE_OPTIONS = subdir-objects

lib_LIBRARIES = lib/libsw_api.a lib/libskeleton.a 
lib_libsw_api_a_CFLAGS = -ansi
lib_libsw_api_a_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../../sw_daemons/include -I$(srcdir)/../../utils/include -I../../hw_api/lnx/include -I../../sw_api/lnx_make/include
lib_libsw_api_a_SOURCES = ../src/phal_services.c ../src/phal_cm_wrapper.c

lib_libskeleton_a_CFLAGS = -ansi
lib_libskeleton_a_CPPFLAGS = -I$(srcdir)/../include -I../../sw_api/lnx_make/include
lib_libskeleton_a_SOURCES = ../src/skeleton.c
#lib_libskeleton_a_LDFLAGS = -L../../aloeSKs/aloeSK15/skel_SK15/lnx_make/lib

include_HEADERS = ../include/phal_sw_api.h ../include/cm_ifc.h ../include/swapi_utils.h

EXTRA_DIST = ../include/*.h ../src/*.h 
