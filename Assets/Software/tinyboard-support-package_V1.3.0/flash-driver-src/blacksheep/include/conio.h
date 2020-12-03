#ifndef _CONIO_H_INCLUDED_
#define _CONIO_H_INCLUDED_

#include "..\common\stdio\BSconio.h"
#include "..\common\misc\sleep.h"

#define delay	bs_delay

#ifdef _USE_VDK_

#define kbhit 	bs_vdk_kbhit
#define _kbhit 	bs_vdk_kbhit

#else

#define kbhit 	bs_kbhit
#define _kbhit 	bs_kbhit

#endif

#define Sleep	bs_sleep

#endif


