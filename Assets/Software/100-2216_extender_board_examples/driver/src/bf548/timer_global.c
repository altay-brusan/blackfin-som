#include "../common/gpTimerConfig.h"

#include <cdefbf548.h>
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
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
			(unsigned long *)pTIMER_ENABLE0,
			(unsigned long *)pTIMER_DISABLE0,
			pTIMER_STATUS0,
			pTIMER7_CONFIG,		
			pTIMER7_COUNTER,
			pTIMER7_PERIOD,
			pTIMER7_WIDTH,
			0
		},
		// Timers 8-10 are controlled via pTimer_ENABLE1 and pTIMER_DISABLE1 pTIMER_STATUS1
		{
			false,
			8,
			ADI_INT_TIMER8,
			0,
			0x0001,
			0x00001000,
			0x00000001,		
			(unsigned long *)pTIMER_ENABLE1,
			(unsigned long *)pTIMER_DISABLE1,
			pTIMER_STATUS1,
			pTIMER8_CONFIG,		
			pTIMER8_COUNTER,
			pTIMER8_PERIOD,
			pTIMER8_WIDTH,
			0
		}
		,
		{
			false,
			9,
			ADI_INT_TIMER9,
			0,
			0x0002,
			0x00002000,
			0x00020000,		
			(unsigned long *)pTIMER_ENABLE1,
			(unsigned long *)pTIMER_DISABLE1,
			pTIMER_STATUS1,
			pTIMER9_CONFIG,		
			pTIMER9_COUNTER,
			pTIMER9_PERIOD,
			pTIMER9_WIDTH,
			0
		}
		,
		{
			false,
			10,
			ADI_INT_TIMER10,
			0,
			0x0004,
			0x00004000,
			0x00040000,		
			(unsigned long *)pTIMER_ENABLE1,
			(unsigned long *)pTIMER_DISABLE1,
			pTIMER_STATUS1,
			pTIMER10_CONFIG,		
			pTIMER10_COUNTER,
			pTIMER10_PERIOD,
			pTIMER10_WIDTH,
			0
		}
	};
	
unsigned int g_nTIMERcount = sizeof(g_aTIMERspec) / sizeof(T_GP_TIMER_SPEC);		//calculate the nr of elements in the array

// for platform specific initialisation
bool timer_platformInit(T_TIMER_PLATFORMINIT_PARAM *pa_aTimerInit) {
	
	if (pa_aTimerInit->nConfig & 0xa0) {
		unsigned short nValue = *pPORTG_FER;
		*pPORTG_FER |= 0x0001;
		*pPORTG_FER |= 0x0001;
		// enable input buffer for PG0
		nValue = *pPORTG_INEN;
		*pPORTG_INEN |= 0x0001;
		*pPORTG_INEN |= 0x0001;
	}
	
	// enable appropriate timer pin on portx_fer and port_mux registers
	// WORKAROUND: read it onces and write it twice
	switch (pa_aTimerInit->nTimer) {
		case 0: {
			// enable timer 0 pin PB8
			unsigned short nValue = *pPORTB_FER;
			*pPORTB_FER |= (0x1 << 8);
			*pPORTB_FER |= (0x1 << 8);
			
			nValue = *pPORTB_MUX;
			*pPORTB_MUX |= (0x01 << 16);
			*pPORTB_MUX |= (0x01 << 16);
						
			
			break;
		}
		case 1: {
			// enable timer 1 pin PB9
			unsigned short nValue = *pPORTB_FER;
			*pPORTB_FER |= (0x1 << 9);
			*pPORTB_FER |= (0x1 << 9);
			
			nValue = *pPORTB_MUX;
			*pPORTB_MUX |= (0x01 << 18);
			*pPORTB_MUX |= (0x01 << 18);
			
			break;
		}
		case 2: {
			// enable timer 2 pin PB10
			unsigned short nValue = *pPORTB_FER;
			*pPORTB_FER |= (0x1 << 10);
			*pPORTB_FER |= (0x1 << 10);
			
			nValue = *pPORTB_MUX;
			*pPORTB_MUX |= (0x01 << 20);
			*pPORTB_MUX |= (0x01 << 20);

			break;
		}
		case 3: {
			// enable timer 3 pin PB11
			unsigned short nValue = *pPORTB_FER;
			*pPORTB_FER |= (0x1 << 11);
			*pPORTB_FER |= (0x1 << 11);
			
			nValue = *pPORTB_MUX;
			*pPORTB_MUX |= (0x01 << 22);
			*pPORTB_MUX |= (0x01 << 22);
			
			break;
		}
		case 4: {
			// enable timer 4 pin PA1
			unsigned short nValue = *pPORTA_FER;
			*pPORTA_FER |= (0x1 << 1);
			*pPORTA_FER |= (0x1 << 1);
			
			nValue = *pPORTA_MUX;
			*pPORTA_MUX |= (0x01 << 2);
			*pPORTA_MUX |= (0x01 << 2);
			
			break;
		}
		case 5: {
			// enable timer 5 pin PA5
			unsigned short nValue = *pPORTA_FER;
			*pPORTA_FER |= (0x1 << 5);
			*pPORTA_FER |= (0x1 << 5);
			
			nValue = *pPORTA_MUX;
			*pPORTA_MUX |= (0x01 << 10);
			*pPORTA_MUX |= (0x01 << 10);
			
			break;
		}
		case 6: {
			// enable timer 6 pin PA9
			unsigned short nValue = *pPORTA_FER;
			*pPORTA_FER |= (0x1 << 9);
			*pPORTA_FER |= (0x1 << 9);
			
			nValue = *pPORTA_MUX;
			*pPORTA_MUX |= (0x01 << 18);
			*pPORTA_MUX |= (0x01 << 18);
			
			break;
		}
		case 7: {
			// enable timer 7 pin PA13
			unsigned short nValue = *pPORTA_FER;
			*pPORTA_FER |= (0x1 << 13);
			*pPORTA_FER |= (0x1 << 13);
			
			nValue = *pPORTA_MUX;
			*pPORTA_MUX |= (0x01 << 26);
			*pPORTA_MUX |= (0x01 << 26);
			
			break;
		}
		case 8: {
			// enable timer 8 pin PH2
			unsigned short nValue = *pPORTH_FER;
			*pPORTH_FER |= (0x1 << 2);
			*pPORTH_FER |= (0x1 << 2);
			
			nValue = *pPORTH_MUX;
			*pPORTH_MUX |= (0x01 << 4);
			*pPORTH_MUX |= (0x01 << 4);
			
			break;
		}
		case 9: {
			// enable timer 9 pin PH3
			unsigned short nValue = *pPORTH_FER;
			*pPORTH_FER |= (0x1 << 3);
			*pPORTH_FER |= (0x1 << 3);
			
			nValue = *pPORTH_MUX;
			*pPORTH_MUX |= (0x01 << 6);
			*pPORTH_MUX |= (0x01 << 6);
			
			break;
		}
		case 10: {
			// enable timer 10 pin PH4
			unsigned short nValue = *pPORTH_FER;
			*pPORTH_FER |= (0x1 << 4);
			*pPORTH_FER |= (0x1 << 4);
			
			nValue = *pPORTH_MUX;
			*pPORTH_MUX |= (0x01 << 8);
			*pPORTH_MUX |= (0x01 << 8);
			
			break;
		}


		default:
			// unknown timer
			return false;
	}
	return true;
}

