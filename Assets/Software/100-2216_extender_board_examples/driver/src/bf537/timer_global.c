#include "../common/gpTimerConfig.h"

#include <cdefbf537.h>
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
			pTIMER_STATUS,
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
			pTIMER_STATUS,
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
			pTIMER_STATUS,
			pTIMER2_CONFIG,		
			pTIMER2_COUNTER,
			pTIMER2_PERIOD,
			pTIMER2_WIDTH,
			0
		},
		{
			false,
			3,
			ADI_INT_TIMER3,
			0,
			0x0008,
			0x00008000,
			0x00000008,		
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			pTIMER_STATUS,
			pTIMER3_CONFIG,		
			pTIMER3_COUNTER,
			pTIMER3_PERIOD,
			pTIMER3_WIDTH,
			0
		},
		{
			false,
			4,
			ADI_INT_TIMER4,
			0,
			0x0010,
			0x10000000,
			0x00010000,	
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			pTIMER_STATUS,
			pTIMER4_CONFIG,		
			pTIMER4_COUNTER,
			pTIMER4_PERIOD,
			pTIMER4_WIDTH,
			0
		},
		{
			false,
			5,
			ADI_INT_TIMER5,
			0,
			0x0020,
			0x20000000,
			0x00020000,		
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			pTIMER_STATUS,
			pTIMER5_CONFIG,		
			pTIMER5_COUNTER,
			pTIMER5_PERIOD,
			pTIMER5_WIDTH,
			0
		},
		{
			false,
			6,
			ADI_INT_TIMER6,
			0,
			0x0040,
			0x40000000,
			0x00040000,				
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			pTIMER_STATUS,
			pTIMER6_CONFIG,		
			pTIMER6_COUNTER,
			pTIMER6_PERIOD,
			pTIMER6_WIDTH,
			0
		},
		{
			false,
			7,
			ADI_INT_TIMER7,
			0,
			0x0080,
			0x80000000,
			0x00080000,		
			(unsigned long *)pTIMER_ENABLE,
			(unsigned long *)pTIMER_DISABLE,
			pTIMER_STATUS,
			pTIMER7_CONFIG,		
			pTIMER7_COUNTER,
			pTIMER7_PERIOD,
			pTIMER7_WIDTH,
			0
		}
	};
	
unsigned int g_nTIMERcount = sizeof(g_aTIMERspec) / sizeof(T_GP_TIMER_SPEC);		//calculate the nr of elements in the array

// for platform specific initialisation
bool timer_platformInit(T_TIMER_PLATFORMINIT_PARAM *pa_aTimerInit) {
	// enable appropriate timer pin on portx_fer and port_mux registers
	// WORKAROUND: read it onces and write it twice
	
	// if clock by TMRCLK is enabled, set alternate function of PF15
	if (pa_aTimerInit->nConfig & 0xa0) {
		unsigned short nValue = *pPORTF_FER;
		*pPORTF_FER |= 0x8000;
		*pPORTF_FER |= 0x8000;
		// enable input buffer for PF15
		nValue = *pPORTFIO_INEN;
		*pPORTFIO_INEN |= 0x8000;
		*pPORTFIO_INEN |= 0x8000;
	}
	
	switch (pa_aTimerInit->nTimer) {
		case 0: {
			// enable timer 0 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0200;
			*pPORTF_FER |= 0x0200;
			break;
		}
		case 1: {
			// enable timer 1 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0100;
			*pPORTF_FER |= 0x0100;
			break;
		}
		case 2: {
			// enable timer 2 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0080;
			*pPORTF_FER |= 0x0080;
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0100;
			*pPORT_MUX &= ~0x0100;
			break;
		}
		case 3: {
			// enable timer 3 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0040;
			*pPORTF_FER |= 0x0040;
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0080;
			*pPORT_MUX &= ~0x0080;
			break;
		}
		case 4: {
			// enable timer 4 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0020;
			*pPORTF_FER |= 0x0020;
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0040;
			*pPORT_MUX &= ~0x0040;
			break;
		}
		case 5: {
			// enable timer 5 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0010;
			*pPORTF_FER |= 0x0010;
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0020;
			*pPORT_MUX &= ~0x0020;
			break;
		}
		case 6: {
			// enable timer 6 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0008;
			*pPORTF_FER |= 0x0008;
			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0010;
			*pPORT_MUX |= 0x0010;
			break;
		}
		case 7: {
			// enable timer 7 pin
			unsigned short nValue = *pPORTF_FER;
			*pPORTF_FER |= 0x0004;
			*pPORTF_FER |= 0x0004;
			nValue = *pPORT_MUX;
			*pPORT_MUX |= 0x0010;
			*pPORT_MUX |= 0x0010;
			break;
		}
		default:
			// unknown timer
			return false;
	}
	return true;
}

