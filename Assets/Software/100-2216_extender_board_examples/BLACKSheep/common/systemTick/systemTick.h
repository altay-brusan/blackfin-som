#ifndef _SYSTEM_TICK_H_INCLUDED_
#define _SYSTEM_TICK_H_INCLUDED_

#include <datatypes.h>

#ifdef _USE_VDK_
#include <VDK.h>
#endif

typedef void (*T_SYSTICK_CALLBACK)(void);

typedef struct {
    T_SYSTICK_CALLBACK fCallback;
    unsigned long nTickCounter;
    float fTickIntervall;
    unsigned long nCallBackIntervall;
    unsigned long nCallBackIntervallCounter;    
} T_SYSTICK_SPEC;

// prototype declarations
T_ERROR_CODE SysTickSetup(  unsigned long pa_nCoreClk,
							float pa_fTickTime,							// [ms]
							unsigned long pa_nUserCallbackIntervall,	// [nof ticks]
							T_SYSTICK_CALLBACK pa_fCallback,
							void *pa_pGeneric);
unsigned long SysTickGetUpTime(void);
unsigned long SysTickGetTickCount(void);
float SysTickGetTickTime(void);


#endif
