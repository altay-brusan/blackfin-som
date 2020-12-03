#include "systemTick.h"
#include "../../../driver/src/common/coreTimer/coreTimerConfig.h"

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// force code and data from here on to internal memory
#pragma default_section(CODE, "L1_code")
#pragma default_section(DATA, "L1_data_a")
#pragma default_section(CONSTDATA, "L1_data_a")
#pragma default_section(BSZ, "L1_data_a")
#pragma default_section(ALLDATA, "L1_data_a")

static T_SYSTICK_SPEC g_tSysTickSpec;

void SystickTimerHandler(void) {
    g_tSysTickSpec.nTickCounter ++;
    g_tSysTickSpec.nCallBackIntervallCounter ++;
    if ((g_tSysTickSpec.fCallback) && (g_tSysTickSpec.nCallBackIntervallCounter == g_tSysTickSpec.nCallBackIntervall)) {
        g_tSysTickSpec.nCallBackIntervallCounter = 0;
        g_tSysTickSpec.fCallback();
    }
}

//return to default sections
#pragma default_section(ALLDATA)
#pragma default_section(BSZ)
#pragma default_section(CONSTDATA)
#pragma default_section(DATA)
#pragma default_section(CODE)
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

T_ERROR_CODE SysTickSetup(  unsigned long pa_nCoreClk,
							float pa_fTickTime,							// [ms]
							unsigned long pa_nUserCallbackIntervall,	// [nof ticks]
							T_SYSTICK_CALLBACK pa_fCallback,
							void *pa_pGeneric) {
    T_ERROR_CODE erResult = ERR_NONE;
    
    g_tSysTickSpec.nTickCounter = 0;
    g_tSysTickSpec.nCallBackIntervall = pa_nUserCallbackIntervall;
	g_tSysTickSpec.nCallBackIntervallCounter = 0;
	
    g_tSysTickSpec.fTickIntervall = pa_fTickTime;
    if (pa_fCallback) {
    	g_tSysTickSpec.fCallback = pa_fCallback;    
    }
    
    // calc tick intervall
    unsigned long nCount = (unsigned long)((pa_fTickTime / 1000.0) / (1.0 / (float)pa_nCoreClk)) - 1;
    
    // setup core timer
    erResult = CTMRsetup(true, 
						nCount,
						nCount,
						0,
						(T_CTMR_CALLBACK)SystickTimerHandler,
						pa_pGeneric);
	// enable core timer
	if (erResult == ERR_NONE) {						
		CTMRenable();						
	}
	
    return erResult;
}

// returns the uptime in us
unsigned long SysTickGetUpTime(void) {
#ifdef _USE_VDK_

#else    
    return g_tSysTickSpec.fTickIntervall * (float)g_tSysTickSpec.nTickCounter * 1000.0;
#endif    
}


unsigned long SysTickGetTickCount(void) {
#ifdef _USE_VDK_

#else    
    return g_tSysTickSpec.nTickCounter;
#endif    
}

float SysTickGetTickTime(void) {
#ifdef _USE_VDK_

#else
	return g_tSysTickSpec.fTickIntervall;
#endif
}
