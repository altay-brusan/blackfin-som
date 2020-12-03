/* *****************************************************************

	NOTE:	Only 8 databits and the ppiclk signals are enabled by the
			ppi driver. If you need additional signals (framesyncs or
			the higher databits you have to enable it by yourself.
			
			PPI0 enables PPIClk on pin PG5, PPIFS1 on pin PG6 and PPIFS2 on pin FS7
			PPI1 enables PPIClk on pin PG12, PPIFS1 on pin PG13 and PPIFS2 on pin FS14
		
******************************************************************** */


#include "../common/PPI.h"

#include <cdefbf518.h>
#include <services/services.h>
#include <environment.h>
		
T_PPI_SPEC g_aPPIspec[] = {
		{
			false,
			PPI_DEFAULT_DMA_CHANNEL,
			ADI_INT_DMA0_PPI,
			ADI_INT_PPI_ERROR,
			ADI_INT_DMA_ERROR,
			0,
			pPPI_FRAME,
			pPPI_DELAY,
			pPPI_COUNT,
			pPPI_CONTROL,
			pPPI_STATUS,
			pDMA0_IRQ_STATUS,
			pDMA0_CONFIG,			
			(volatile void **)DMA0_START_ADDR,
			pDMA0_X_MODIFY,
			pDMA0_X_COUNT,
			pDMA0_CURR_X_COUNT,			
			pDMA0_Y_MODIFY,
			pDMA0_Y_COUNT,
			pDMA0_CURR_Y_COUNT,
			(volatile void **)pDMA0_NEXT_DESC_PTR,			
			0
		}
	};
	
unsigned int g_nPPIcount = sizeof(g_aPPIspec) / sizeof(T_PPI_SPEC);		//calculate the nr of elements in the array
	
bool ppi_platformInit(unsigned char pa_cPPIindex, unsigned short pa_nPPIcontrol) {
	// setting the appropriate alternate functions in PORTx_FER so that the PPI signals are available.
	// WORKAROUND: Read it onces, write it twice. (See anomaly list.)
    if ((pa_nPPIcontrol & 0x000c) != 0x000c) {
        // we are in ITU656 mode
	    unsigned short nValue = *pPORTG_FER;
	    *pPORTF_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
	    *pPORTF_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
		
	    nValue = *pPORTG_MUX;
	    *pPORTF_MUX &= ~0x003f;
	    *pPORTF_MUX &= ~0x003f;
	    *pPORTF_MUX |= 0x0015;
	    *pPORTF_MUX |= 0x0015;
	
	    // WORKAROUND: Read it onces, write it twice. (See anomaly list.)
	    nValue = *pPORTG_FER;
	    *pPORTG_FER |= 0x0020;	// enabling PPIClk signal on pin PG5.
	    *pPORTG_FER |= 0x0020;	// enabling PPIClk signal on pin PG5.
		
	    nValue = *pPORTG_MUX;
	    *pPORTG_MUX &= ~0x00c0;
	    *pPORTG_MUX &= ~0x00c0;
	    *pPORTG_MUX |= 0x0080;
	    *pPORTG_MUX |= 0x0080;
	    return true;
	} else {
	   	// enable the ppi control signals PPIClk (pinPG5), (PPI_FS1 (pin PG6) and PPI_FS2 (pin PG7)
#ifndef PPI_USE_ALTERNATIVE_SYNC_PINS	   	
	    unsigned short nValue = *pPORTG_FER;
	    *pPORTG_FER |= 0x00e0;
	    *pPORTG_FER |= 0x00e0;
		
	    nValue = *pPORTG_MUX;
	    *pPORTG_MUX &= ~0x00c0;
	    *pPORTG_MUX &= ~0x00c0;
	    *pPORTG_MUX |= 0x0080;
	    *pPORTG_MUX |= 0x0080;
#else
        unsigned short nValue = *pPORTG_FER;
	    *pPORTG_FER |= 0x7000;
	    *pPORTG_FER |= 0x7000;
		
	    nValue = *pPORTG_MUX;
	    *pPORTG_MUX &= ~0xc000;
	    *pPORTG_MUX &= ~0xc000;
	    *pPORTG_MUX |= 0x4000;
	    *pPORTG_MUX |= 0x4000;

#endif    
	    
	    if ((pa_nPPIcontrol & 0x3800) == 0) {
	        // 8 bit non itu 656 mode
		    unsigned short nValue = *pPORTF_FER;
		    *pPORTF_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
		    *pPORTF_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
		
		    *pPORTF_MUX &= ~0x003f;
		    *pPORTF_MUX &= ~0x003f;
		    *pPORTF_MUX |= 0x0015;
		    *pPORTF_MUX |= 0x0015;
			return true;
	    } else if ((pa_nPPIcontrol & 0x3800) == 0x0800) {
		    // 10 bit non itu 656 mode
		    unsigned short nValue = *pPORTF_FER;
		    *pPORTF_FER |= 0x03ff;	// enabling PPI data signals (D0 - D9).
		    *pPORTF_FER |= 0x03ff;	// enabling PPI data signals (D0 - D9).
		
		    *pPORTF_MUX &= ~0x00ff;
		    *pPORTF_MUX &= ~0x00ff;
		    *pPORTF_MUX |= 0x0055;
		    *pPORTF_MUX |= 0x0055;
        	return true;
        } else if ((pa_nPPIcontrol & 0x3800) == 0x1800) {
		    // 12 bit non itu 656 mode
		    unsigned short nValue = *pPORTF_FER;
		    *pPORTF_FER |= 0x0fff;	// enabling PPI data signals (D0 - D11).
		    *pPORTF_FER |= 0x0fff;	// enabling PPI data signals (D0 - D11).
		
		    *pPORTF_MUX &= ~0x03ff;
		    *pPORTF_MUX &= ~0x03ff;
		    *pPORTF_MUX |= 0x0155;
		    *pPORTF_MUX |= 0x0155;
        	return true;
        }
    }
	return false;
}
