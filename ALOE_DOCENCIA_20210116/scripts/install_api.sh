#!/bin/sh -v


cd sw_daemons/src/cmdman
gcc -fPIC -I./ -I../../include -I../../../utils/include -I../../../hw_api/include -g -c -Wall cmdman_backend.c
gcc -fPIC -g -c -Wall cmdman_jni.c
gcc -L../../../utils/lib -L../../../hw_api/lib -L../../lib -shared -Wl,-soname,libcmdman_api.so.1 -o libcmdman_api.so.1.0.1 cmdman_backend.o cmdman_jni.o -lc ../../../utils/src/lib_libutils_a-set.o -lhw_api -lswd_common
sudo mv libcmdman_api.so.1.0.1 /usr/local/lib
sudo ln -fs /usr/local/lib/libcmdman_api.so.1.0.1 /usr/local/lib/libcmdman_api.so
sudo ldconfig -n /usr/local/lib
cd ../../../