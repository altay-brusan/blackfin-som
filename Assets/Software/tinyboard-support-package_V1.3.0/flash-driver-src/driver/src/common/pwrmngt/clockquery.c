/**
 *	@file 		clockquery.c
 *	@ingroup 	CLOCKS
 *	
 *	@brief 		Core and System Clock frequency Querying
 *	
 *						
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
 *	
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 **/

/** @defgroup CLOCKS
 *  @ingroup driverapi
 *  Core & Sys Clocks
 */


#include <environment.h>
#include <services/services.h>
#include "clockquery.h"
 
unsigned long g_nClockIn = 0; 

#ifndef PROC_CLOCK_FREQ_IN
#error "PROC_CLOCK_FREQ_IN must be defined (in Mhz)!"
#endif
#if PROC_CLOCK_FREQ_IN > 100
#error "PROC_CLOCK_FREQ_IN must be defined (in Mhz)!"
#endif





/**
 *	@public
 *	@brief		Returns the Core Clock frequency in Hz
 *	
 *	
 *	@return		Returns an unsigned long integer of the Core Clock frequency
 *	
 **/
unsigned long getCoreClockFrequency (void) {
	unsigned long nClkIn = PROC_CLOCK_FREQ_IN * 1000000;
	if (g_nClockIn) {
		nClkIn = g_nClockIn;
	}
	if (*pPLL_CTL & 0x1) {
		nClkIn >>= 1;
	}
	unsigned short msel = (*pPLL_CTL >> 9) & 0x003f;
	unsigned short csel = (*pPLL_DIV >> 4) & 0x0003;
	unsigned long nVCO = msel * nClkIn;
	unsigned long nCclk = 0;
	switch (csel) {
		case 0: nCclk = nVCO;
				break;
		case 1: nCclk = nVCO / 2;
				break;
		case 2: nCclk = nVCO / 4;
				break;
		case 3: nCclk = nVCO / 8;
				break;
		default:
				break;
	}
	return nCclk;
}





/**
 *	@public
 *	@brief		Returns the System Clock frequency in Hz
 *	
 *	
 *	@return		Returns an unsigned long integer of the System Clock frequency
 *	
 **/
unsigned long getSystemClockFrequency (void) {
	unsigned long nClkIn = PROC_CLOCK_FREQ_IN * 1000000;
	if (g_nClockIn) {
		nClkIn = g_nClockIn;
	}
	if (*pPLL_CTL & 0x1) {
		nClkIn >>= 1;
	}
	unsigned short msel = (*pPLL_CTL >> 9) & 0x003f;
	unsigned short ssel = *pPLL_DIV & 0x000f;
	unsigned long nVCO = msel * nClkIn;
	return (nVCO / ssel);
}



/**
 *	@public
 *	@brief		Set's the System Clock Input Frequency
 *	
 *	@param		pa_nClockIn		Input Clock frequency in Hz
 *	
 *	
 **/
void setClockIn (unsigned long pa_nClockIn) {
	g_nClockIn = pa_nClockIn;
}
