#include <drivers/adi_dev.h>
#include <sys/exception.h>
#include "coreTimerConfig.h"

// global stuff for managment purposes
extern T_CTMR_REG g_tCTMRreg;		// declared in ctmr_global.c

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// force code and data from here on to internal memory
#pragma default_section(CODE, "L1_code")
#pragma default_section(DATA, "L1_data_a")
#pragma default_section(CONSTDATA, "L1_data_a")
#pragma default_section(BSZ, "L1_data_a")
#pragma default_section(ALLDATA, "L1_data_a")

T_CTMR_SPEC g_CTMRspec;

/*
static ADI_INT_HANDLER_RESULT CTMRinterruptHandler(void *pa_pClientArg) {
    
	if (g_CTMRspec.fCallback) {
		g_CTMRspec.fCallback();
	}
	// clear the interrupt
	*(g_tCTMRreg.pTcntl) |= 0x8;
	
	return ADI_INT_RESULT_PROCESSED;
}
*/
 
EX_INTERRUPT_HANDLER (CTMRinterruptHandler) {
//	*(g_tCTMRreg.pTcntl) = *(g_tCTMRreg.pTcntl) | 0x8;
   
	if (g_CTMRspec.fCallback) {
		g_CTMRspec.fCallback();
	}
	// clear the interrupt

	
	ssync();
}

//return to default sections
#pragma default_section(ALLDATA)
#pragma default_section(BSZ)
#pragma default_section(CONSTDATA)
#pragma default_section(DATA)
#pragma default_section(CODE)
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


T_ERROR_CODE CTMRsetup(	bool pa_bAutoReload,
						unsigned long pa_nCount,
						unsigned long pa_nPeriod,
						unsigned long pa_nScale,
						T_CTMR_CALLBACK pa_fCallback,
						void *pa_nGeneric) {
				    
						    						    
	T_ERROR_CODE erResult = ERR_NONE;
	if (pa_bAutoReload) {
		*(g_tCTMRreg.pTcntl) = 5;
	} else {
		*(g_tCTMRreg.pTcntl) = 1;
	}
	*(g_tCTMRreg.pTcount) = pa_nCount;
	*(g_tCTMRreg.pTperiod) = pa_nPeriod;
	*(g_tCTMRreg.pTscale) = pa_nScale;
	if (pa_fCallback) {
		g_CTMRspec.fCallback = pa_fCallback;
		// hook interrupt
		/*if(adi_int_CECHook(g_tCTMRreg.nIVG, CTMRinterruptHandler, (void *)&g_CTMRspec, false) == ADI_INT_RESULT_SUCCESS) {
		} else {
			erResult = ERR_CTMR_HOOK_INTERRUPT;
		}*/
		
		register_handler(ik_timer, CTMRinterruptHandler);			
	} else {
	    g_CTMRspec.fCallback = 0;
	}	
	return erResult;				    
				    
}

unsigned long CTMRgetCount(void) {
    return *(g_tCTMRreg.pTcount);
}

void CTMRsetCount(unsigned long pa_nCount) {
    *(g_tCTMRreg.pTcount) = pa_nCount;
}

section ("L1_code")
void CTMRenable (void) {
    *(g_tCTMRreg.pTcntl) |= 0x0002;
}

section ("L1_code")
void CTMRdisable (void) {
    *(g_tCTMRreg.pTcntl) &= ~0x0002;    
}
