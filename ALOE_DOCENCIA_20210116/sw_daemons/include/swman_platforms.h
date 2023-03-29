

/* Configure directories and supported platforms.
 * constants for platforms defined at swman.h
 *
 */
struct platforms_cfg platforms_cfg[NOF_PLATS] = {
    {        
        PLATFORM_LIN_X86,     /* platform code */
        RELATIVE,           /* relative addressed executables */
        "linux",            /* path to executables under swman_execs/ */
        "linux"             /* platform name (for the execinfo file) */
    },{
        PLATFORM_LIN_X86_64,     /* platform code */
        RELATIVE,           /* relative addressed executables */
        "linux_64",            /* path to executables under swman_execs/ */
        "linux_64"             /* platform name (for the execinfo file) */
    },{
        PLATFORM_LIN_ARM,     /* platform code */
        RELATIVE,           /* relative addressed executables */
        "linux_arm",            /* path to executables under swman_execs/ */
        "linux_arm"             /* platform name (for the execinfo file) */
    },{
        PLATFORM_LIN_PPC,     /* platform code */
        RELATIVE,           /* relative addressed executables */
        "linux_ppc",            /* path to executables under swman_execs/ */
        "linux_ppc"             /* platform name (for the execinfo file) */
    },{
        PLATFORM_TI_6455, ABSOLUTE, "c6000", "c6000"
    }
};
