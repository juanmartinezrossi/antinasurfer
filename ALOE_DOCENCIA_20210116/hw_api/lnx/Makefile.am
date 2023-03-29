AUTOMAKE_OPTIONS = subdir-objects


lib_LIBRARIES = lib/libhw_api.a
lib_libhw_api_a_CPPFLAGS = -I$(srcdir)/include/
lib_libhw_api_a_CFLAGS = -fPIC
lib_libhw_api_a_SOURCES = src/hwapi_linux_base.c src/hwapi_linux_man.c src/hwapi_linux_hwinfo.c src/hwapi_linux_itf.c src/hwapi_linux_mem.c src/hwapi_linux_proc.c src/hwapi_linux_time.c src/hwapi_linux_dac.c src/hwapi_linux_var.c src/parser.c src/mem.c
#lib_libhw_api_a_LIBADD = -lm

if ONLYMODULES
bin_PROGRAMS = bin/runcf
bin_runcf_CC = g++
bin_runcf_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../../utils/include -I$(srcdir)/../../sw_daemons/include -I$(srcdir)/dac/include  -I$(srcdir)/dac/  
bin_runcf_LDFLAGS = -L$(srcdir)/lib/ -L$(srcdir)/../../utils/lnx_make/lib -L$(srcdir)/../../sw_daemons/lnx_make/lib
bin_runcf_LDADD = lib/libhw_api.a -lutils -lpthread -lrt 
bin_runcf_SOURCES = src/hwapi_backd_cmds.c src/hwapi_backd_exec.c src/hwapi_backd_mem.c src/hwapi_backd_signals.c src/hwapi_backd_processes.c src/hwapi_backd.c src/hwapi_netd.c src/mem.c src/hwapi_backd_dac.c src/mac_test.c
bin_runcf_SOURCES += src/parser.c src/hwapi_backd_parser_itf.c src/hwapi_backd_parser_sched.c src/hwapi_backd_parser_platform.c src/hwapi_backd_parser.c


if HAVE_UHD
bin_runcf_CPPFLAGS += -I$(srcdir)/src
bin_runcf_SOURCES += dac/uhd/uhd_imp.cpp dac/uhd/uhd_parser.c
bin_runcf_LDADD += -lboost_thread -luhd
endif

if HAVE_JACK
bin_runcf_LDADD += -ljack
bin_runcf_SOURCES += dac/soundcard/sndcard_imp.c
endif

if HAVE_ALSA
bin_runcf_LDADD += -lasound
bin_runcf_SOURCES += dac/alsacardTX/alsacardTX_imp.c
endif

if HAVE_ALSA
bin_runcf_LDADD += -lasound
bin_runcf_SOURCES += dac/alsacardRX/alsacardRX_imp.c
endif

if HAVE_ALSA
bin_runcf_LDADD += -lasound
bin_runcf_SOURCES += dac/alsacardRXTX/alsacardRXTX_imp.c
endif


if HAVE_X5
bin_runcf_CPPFLAGS += -I/usr/Innovative/Malibu/ -I/usr/include/c++/4.4/ -DLINUX 
bin_runcf_LDFLAGS += -L/usr/Innovative/Lib/Gcc/Release 
bin_runcf_LDADD += -lPci_Mb -lFicl_Mb  -lHardware_Mb -lFramework_Con -lFramework_Mb  -lUtility_Mb    -lHardware_Mb    -lOs_Mb -lFramework_Con   -lAnalysis_Mb -lwdapi
bin_runcf_SOURCES += dac/x5-400/x5_parser.c dac/x5-400/x5_stub.cpp dac/x5-400/BoardIo.cpp
endif
endif

include_HEADERS = include/phal_hw_api.h include/phal_hw_api_man.h include/phid.h
EXTRA_DIST = include/*.h src/*.h dac/include/*.h 

dist_sysconf_DATA = cfg/*

dist_bin_SCRIPTS=scripts/killph scripts/restartph scripts/rmsem







