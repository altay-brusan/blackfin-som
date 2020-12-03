#include <services/services.h>
#include <cdefbf548.h>
#include "../common/coreTimer/coreTimerConfig.h"

T_CTMR_REG g_tCTMRreg = {
	ADI_INT_IVG_CORE_TIMER,
	pTCNTL,
	pTCOUNT,
	pTPERIOD,
	pTSCALE
};

