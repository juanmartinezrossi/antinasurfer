AC_INIT([ALOE], [1.6.0])
AM_INIT_AUTOMAKE
AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS(time.h)
AC_CHECK_HEADERS(stdio.h)
AC_CHECK_HEADERS(stdlib.h)
AC_CHECK_HEADERS([arpa/inet.h fcntl.h float.h netdb.h netinet/in.h stddef.h stdint.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([strdup strerror strtod time atexit bzero clock_gettime gettimeofday inet_ntoa memset mkdir pow socket sqrt strchr strdup strstr strtoul])

AC_CHECK_LIB(rt, clock_gettime,[AC_MSG_NOTICE([Found rt library])],[AC_MSG_ERROR([rt library not found!])])
AC_SUBST(swapi,"-lsw_api -lhw_api -lrt -lm -ltypetools")
AC_SUBST(hwapi,"-lhw_api -lrt -lm -ljack -lasound")

AC_SUBST([ONLY_MODULES],no)

# initialize dac driver constant checks
AC_SUBST([HAVE_JACK],no)
AC_SUBST([HAVE_UHD],no)
AC_SUBST([HAVE_X5],no)
AC_SUBST([HAVE_ALSA],no)

AC_ARG_WITH(modulesonly, [  --with-modulesonly Compile and install modules only],
[
    AC_SUBST([ONLY_MODULES],yes)
])

if test "${ONLY_MODULES}" = no; then
	
	AC_CHECK_LIB(readline, readline,[AC_MSG_NOTICE([Found realine])],[AC_MSG_ERROR([readline library not found!])])
	AC_CHECK_LIB(history, add_history,[AC_MSG_NOTICE([Found history])],[AC_MSG_ERROR([history library not found!])])
	
	####### DAC files. Any dac set the constant HAVE_DAC to yes and defines ENABLE_DAC
	# new drivers can be added here by copying the same lines and adding the AC_SUBST and
	# AM_CONDITIONAL lines below. Add also the files in hwapi_lnx/Makefile.am 
	# check if jack library is available and include required files for compilation.  
	AC_CHECK_LIB(jack, jack_client_open,
						[AC_MSG_NOTICE([Found JACK library]) 
						AC_SUBST([HAVE_JACK],yes)
						AC_DEFINE([HAVE_JACK])],
					[AC_MSG_NOTICE([JACK library not found. soundcard DAC not availabe.])])
					
	# check if jack library is available and include required files for compilation.  
	AC_LANG_CPLUSPLUS
	AC_CHECK_LIB(wdapi, main,
						[AC_MSG_NOTICE([Found X5-400 library]) 
						AC_SUBST([HAVE_X5],yes)
						AC_DEFINE([HAVE_X5])],
					[AC_MSG_NOTICE([X5-400 library not found. Board DAC not availabe.])],
					-L/usr/Innovative/Lib/Gcc/Release/)
	AC_CHECK_LIB(uhd, main,
						[AC_MSG_NOTICE([Found USRP library]) 
						AC_SUBST([HAVE_UHD],yes)
						AC_DEFINE([HAVE_UHD])
						AC_CHECK_LIB(boost_thread,main)
						],
					[AC_MSG_NOTICE([USRP library not found. Board DAC not availabe.])])
	AC_LANG_C
	AC_CHECK_LIB(asound, snd_pcm_open,
						[AC_MSG_NOTICE([Found alsa library]) 
						AC_SUBST([HAVE_ALSA],yes)
						AC_DEFINE([HAVE_ALSA])],
					[AC_MSG_NOTICE([ALSA library not found. ALSA soundcard DAC not availabe.])])
					
fi


AC_ARG_WITH(x5, [  --with-x5	Force compilation of X5 Board],
[
	AC_SUBST([HAVE_X5],yes)
])

AM_CONDITIONAL(HAVE_JACK, test "${HAVE_JACK}" = yes)
AM_CONDITIONAL(HAVE_X5, test "${HAVE_X5}" = yes)
AM_CONDITIONAL(HAVE_UHD, test "${HAVE_UHD}" = yes)
AM_CONDITIONAL(HAVE_ALSA, test "${HAVE_ALSA}" = yes)

AM_CONDITIONAL(ONLYMODULES, test "${ONLY_MODULES}" = no)

AC_FUNC_MALLOC

AC_SUBST([am__tar],['${AMTAR} cf - "$$tardir"'; am__untar='${AMTAR} xf -'])

AC_SUBST(FULL_DIST)
AC_SUBST(MAKE_OBJ)

AC_ARG_WITH(objects, [  --with-alldist	Include all directories in the dist],
[
	AC_SUBST([FULL_DIST],yes)
])

AM_CONDITIONAL(IS_FULLDIST, test -n "${FULL_DIST}")

AM_PROG_CC_C_O 

AC_CONFIG_FILES([Makefile
	aloeSKs/aloeSK15/skel_SK15/lnx_make/Makefile
	utils/lnx_make/Makefile
	hw_api/lnx/Makefile
	sw_daemons/lnx_make/Makefile
	sw_api/lnx_make/Makefile
	cmdman_console/Makefile
	modules/typetools/lnx_make/Makefile
		modules/ADSIMUL/lnx_make/Makefile
	modules/CHANNEL_ANALYZER/lnx_make/Makefile
	modules/channel_noise/lnx_make/Makefile
	modules/CIRC_BUFFER/lnx_make/Makefile
	modules/computeEVM/lnx_make/Makefile
	modules/CPLX_FILTER/lnx_make/Makefile
	modules/CRC/lnx_make/Makefile
	modules/DAC_JACK0/lnx_make/Makefile
	modules/DAC_JACK1/lnx_make/Makefile
	modules/data_source/lnx_make/Makefile
	modules/data_source_sink/lnx_make/Makefile
	modules/DDC/lnx_make/Makefile
	modules/DDC17/lnx_make/Makefile
	modules/DUC17/lnx_make/Makefile
	modules/FFT_IFFT/lnx_make/Makefile
	modules/GRAPH/lnx_make/Makefile
	modules/LTEturboCOD/lnx_make/Makefile
	modules/LTEturboCOD2/lnx_make/Makefile
	modules/MOD_QAM/lnx_make/Makefile
	modules/module_template/lnx_make/Makefile
	modules/REAL_FILTER/lnx_make/Makefile
	modules/SCRAMBLING/lnx_make/Makefile
	modules/UPLINK_DEMAPPING/lnx_make/Makefile
	modules/UPLINK_EQUALIZER/lnx_make/Makefile
	modules/UPLINK_MAPPING/lnx_make/Makefile
	modules/UPLINK_SYNCHRO/lnx_make/Makefile
	])

AC_OUTPUT


AC_MSG_RESULT([
==============================================================
				
		== FLEXNets ==
				
ALOE has been configured successfully. You can compile and
install it by typing 'make && sudo make install'
After that, you can run ALOE by:

1) Moving to the app folder: cd APPs/appTopic/appname/
2) Typing: 'runcf -c APPs/appTOPIC/appname/platform.conf'
3) Or with real-time priority: 
	'sudo runcf -c APPs/appTOPIC/appname/platform.conf'

You can choose other compiler flags, e.g.:

include debugging symbols and enable symbol resolve:
	./configure CFLAGS='-rdynamic -O0 -g'

optimize for native processor including SSE instructions 
	./configure CFLAGS='-O3 -march=native -mfpmath=sse'

==============================================================
])


