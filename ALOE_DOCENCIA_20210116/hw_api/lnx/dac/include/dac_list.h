
//#include "alsacard.h"
#include "alsacardRX.h"
#include "alsacardTX.h"
#include "alsacardRXTX.h"
#include "sndcard.h"
#include "x5400.h"
#include "uhd.h"
#include "uhd2.h"

//#define HAVE_USRP2	

/** Use this structure to configure available dacs and their functions
 * (terminate with null)
 */
struct dac_interface system_dacs[] = {

#ifdef HAVE_UHD
		/** USRP board*/
		{	"uhd",
			uhd_readcfg,
			uhd_init,
			uhd_close
		},

#endif

//#ifdef HAVE_USRP2
		/** USRP2 board*/
//		{	"uhd2",
//			uhd2_readcfg,
//			uhd2_init,
//			uhd2_close
//		},

//#endif


#ifdef HAVE_JACK
		/** soundcard dac */
		{	"soundcard",
			sndcard_readcfg,
			sndcard_init,
			sndcard_close
		},
#endif

#ifdef HAVE_X5
		/** x5-400 board dac */
		{	"x5-400",
			x5_readcfg,
			x5_init,
			x5_close
		},

#endif

#ifdef HAVE_ALSA
		/** alsacard dac */
		{	"alsacardTX",
			alsacardTX_readcfg,
			alsacardTX_init,
			alsacardTX_close
		},
#endif

#ifdef HAVE_ALSA
		/** alsacardRX dac */
		{	"alsacardRX",
			alsacardRX_readcfg,
			alsacardRX_init,
			alsacardRX_close
		},
#endif

#ifdef HAVE_ALSA
		/** alsacardRXTX dac */
		{	"alsacardRXTX",
			alsacardRXTX_readcfg,
			alsacardRXTX_init,
			alsacardRXTX_close
		},
#endif
		/** add here your dac */

		/* end of list */
		{NULL,NULL,NULL}};

