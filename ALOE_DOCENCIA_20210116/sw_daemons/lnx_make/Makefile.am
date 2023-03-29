AUTOMAKE_OPTIONS = subdir-objects

if ONLYMODULES

include_HEADERS = ../include/cmdman_backend.h
EXTRA_DIST = ../src/crm/*.h ../include/ ../src/swman/*.h  ../src/statsman/*.h ../src/exec/*.h ../include/*.h

lib_LIBRARIES = lib/libswd_common.a lib/libcmdman_backend.a lib/libcrm_api.a

lib_libcrm_api_a_CPPFLAGS = -I../include
lib_libcrm_api_a_CFLAGS = -fPIC
lib_libcrm_api_a_SOURCES = ../src/crm/cost_commU_mhop_AllArchs.c ../src/crm/cost_compU.c ../src/crm/mapper.c ../src/crm/resource_trans.c ../src/crm/sort_b.c ../src/crm/sort_d.c ../src/crm/trellis12345_sortU.c ../src/crm/tw_mapping.h ../src/crm/update.c

lib_libswd_common_a_CFLAGS= -ansi -fPIC
lib_libswd_common_a_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
lib_libswd_common_a_SOURCES = ../src/common/pe.c ../src/common/xitf.c ../src/common/pkt.c ../src/common/app.c ../src/common/daemon.c ../src/common/phitf.c ../src/common/phobj.c ../src/common/downexe.c ../src/common/rtcobj.c ../src/common/var.c ../src/common/log.c

lib_libcmdman_backend_a_CFLAGS= -ansi -fPIC
lib_libcmdman_backend_a_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
lib_libcmdman_backend_a_SOURCES = ../src/cmdman_api/cmdman_backend.c  


bin_PROGRAMS = bin/nettest bin/sync bin/sync_master bin/swman bin/statsman bin/frontend bin/hwman bin/swload bin/exec bin/stats bin/bridge

bin_nettest_CFLAGS= -ansi
bin_nettest_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_nettest_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_nettest_LDADD = -lswd_common -lutils $(hwapi)
bin_nettest_SOURCES = ../src/nettest/tx.c

bin_sync_CFLAGS= -ansi
bin_sync_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_sync_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_sync_LDADD = -lswd_common -lutils  $(hwapi)
bin_sync_SOURCES = ../src/sync/sync.c

bin_sync_master_CFLAGS= -ansi
bin_sync_master_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_sync_master_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_sync_master_LDADD = -lswd_common -lhw_api -lutils -lrt
bin_sync_master_SOURCES = ../src/sync_master/sync_master.c

bin_swman_CFLAGS= -ansi
bin_swman_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_swman_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_swman_LDADD = -lswd_common -lutils $(hwapi)
bin_swman_SOURCES = ../src/swman/swman_cmds.c ../src/swman/swman.c ../src/swman/swman_parser.c ../src/common/exec.c

bin_hwman_CFLAGS= -ansi
bin_hwman_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_hwman_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_hwman_LDADD = -lswd_common -lutils -lcrm_api $(hwapi)
bin_hwman_SOURCES = ../src/hwman/hwman_cmds.c ../src/hwman/hwman.c 

bin_statsman_CFLAGS= -ansi
bin_statsman_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_statsman_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_statsman_LDADD = -lswd_common -lutils $(hwapi)
bin_statsman_SOURCES = ../src/statsman/statsman_cmds.c ../src/statsman/statsman.c ../src/statsman/statsman_parser.c

bin_frontend_CFLAGS= -ansi
bin_frontend_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_frontend_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_frontend_LDADD = -lswd_common -lutils $(hwapi)
bin_frontend_SOURCES = ../src/frontend/frontend.c

bin_swload_CFLAGS= -ansi
bin_swload_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_swload_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_swload_LDADD = -lswd_common -lutils $(hwapi)
bin_swload_SOURCES = ../src/swload/swload_cmds.c ../src/swload/swload.c

bin_exec_CFLAGS= -ansi
bin_exec_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_exec_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_exec_LDADD = -lswd_common -lutils $(hwapi)
bin_exec_SOURCES = ../src/exec/exec_cmds.c ../src/exec/exec.c

bin_stats_CFLAGS= -ansi
bin_stats_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_stats_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_stats_LDADD = -lswd_common -lutils $(hwapi)
bin_stats_SOURCES = ../src/stats/stats_cmds.c ../src/stats/stats.c

bin_bridge_CFLAGS= -ansi
bin_bridge_CPPFLAGS = -I../include -I../../hw_api/lnx/include -I../../utils/include
bin_bridge_LDFLAGS = -L../../hw_api/lnx/lib -Llib/ -L../../utils/lnx_make/lib
bin_bridge_LDADD = -lswd_common -lutils $(hwapi)
bin_bridge_SOURCES = ../src/bridge/bridge_cmds.c ../src/bridge/bridge.c

endif