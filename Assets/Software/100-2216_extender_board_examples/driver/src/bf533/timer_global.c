#include "../common/gpTimerConfig.h"

#include <cdefbf533.h>
#include <services/services.h>

T_GP_TIMER_SPEC g_aTIMERspec[] = {
		{
			false,
			0,
			ADI_INT_TIMER0,
			0,
			0x0001,
			0x00001000,
			0x00000001,
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			(unsigned long *)pTIMER_STATUS,
			pTIMER0_CONFIG,		
			pTIMER0_COUNTER,
			pTIMER0_PERIOD,
			pTIMER0_WIDTH,
			0
		},
		{
			false,
			1,
			ADI_INT_TIMER1,
			0,
			0x0002,
			0x00002000,
			0x00000002,
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			(unsigned long *)pTIMER_STATUS,
			pTIMER1_CONFIG,		
			pTIMER1_COUNTER,
			pTIMER1_PERIOD,
			pTIMER1_WIDTH,
			0
		},
		{
			false,
			2,
			ADI_INT_TIMER2,
			0,
			0x0004,
			0x00004000,
			0x00000004,			
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			(unsigned long *)pTIMER_STATUS,
			pTIMER2_CONFIG,		
			pTIMER2_COUNTER,
			pTIMER2_PERIOD,
			pTIMER2_WIDTH,
			0
		}
	};
	
unsigned int g_nTIMERcount = sizeof(g_aTIMERspec) / sizeof(T_GP_TIMER_SPEC);		//calculate the nr of elements in the array

// for platform specific initialisation
bool timer_platformInit(T_TIMER_PLATFORMINIT_PARAM *pa_aTimerInit) {
	return true;
}

