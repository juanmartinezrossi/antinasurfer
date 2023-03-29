AUTOMAKE_OPTIONS = subdir-objects

if ONLYMODULES

bin_PROGRAMS = bin/cmdman

EXTRA_DIST=include/*.h

bin_cmdman_CPPFLAGS = -I$(srcdir)/include -I$(srcdir)/../hw_api/lnx/include -I$(srcdir)/../utils/include -I$(srcdir)/../sw_daemons/include
bin_cmdman_LDFLAGS = -L$(srcdir)/../sw_daemons/lnx_make/lib -L$(srcdir)/../hw_api/lnx/lib -L$(srcdir)/../utils/lnx_make/lib
bin_cmdman_LDADD = -lreadline -lhistory -ltermcap -lcmdman_backend -lswd_common -lutils $(hwapi)
bin_cmdman_SOURCES = src/cmdman.c src/cmdman_cmds.c src/cmdman_prints.c

endif
