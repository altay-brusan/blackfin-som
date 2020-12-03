#ifndef _CORE_TIMER_CONFIG_H_INCLUDED_
#define _CORE_TIMER_CONFIG_H_INCLUDED_

#include <datatypes.h>

#ifdef _USE_VDK_
	#warning "Core timer is used by the VDK kernel as tick timer!"
#endif

// error codes
#define ERR_CTMR_HOOK_INTERRUPT					(ERR_GENERIC - 10)

typedef void (*T_CTMR_CALLBACK)(void);

typedef struct {
    T_CTMR_CALLBACK fCallback;
} T_CTMR_SPEC;

typedef struct {
    unsigned long nIVG;
    volatile unsigned long *pTcntl;
    volatile unsigned long *pTcount;
    volatile unsigned long *pTperiod;
    volatile unsigned long *pTscale;
} T_CTMR_REG;

// prototype declarations
T_ERROR_CODE CTMRsetup(	bool pa_bAutoReload,
						unsigned long pa_nCount,
						unsigned long pa_nPeriod,
						unsigned long pa_nScale,
						T_CTMR_CALLBACK pa_fCallback,
						void *pa_nGeneric);
unsigned long CTMRgetCount(void);
void CTMRsetCount(unsigned long pa_nCount);
void CTMRenable (void);
void CTMRdisable (void);

#endif
