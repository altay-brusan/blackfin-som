/* *****************************************************************

	NOTE:	Only 8 databits and the ppiclk signals are enabled by the
			ppi driver. If you need additional signals (framesyncs or
			the higher databits you have to enable it by yourself.
		
******************************************************************** */


#include "../common/PPI.h"

#include <cdefbf527.h>
#include <services/services.h>
		
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
		
		nValue = *pPORTF_MUX;
		*pPORTF_MUX &= ~0x0003;
		*pPORTF_MUX &= ~0x0003;
	
		// WORKAROUND: Read it onces, write it twice. (See anomaly list.)
		nValue = *pPORTF_MUX;
		*pPORTF_MUX |= 0x1000;	// enabling PPIClk signal.
		*pPORTF_MUX |= 0x1000;	// enabling PPIClk signal.
		return true;
	} else {
		// enable the ppi control signals (PPI_FS1, PPI_FS2, PPI_FS3 and PPIClk	
		unsigned short nValue = *pPORTG_FER;
		*pPORTG_FER |= 0x0060;  //enabling PPI_FS2 & PPI_FS3
		*pPORTG_FER |= 0x0060;
		
	    nValue = *pPORTG_MUX;
		*pPORTG_MUX &= ~0x000c;
		*pPORTG_MUX &= ~0x000c;
		*pPORTG_MUX |= 0x0008;
		*pPORTG_MUX |= 0x0008;
		
		
		nValue = *pPORTF_MUX;
		*pPORTF_MUX |= 0x1000;	// enabling PPIClk signal.
		*pPORTF_MUX |= 0x1000;	// enabling PPIClk signal.
		
		char ucCount = (pa_nPPIcontrol & 0x3800) >> 11;
		unsigned short usMask = 1;
		for(;ucCount > 0; ucCount--) {
		    usMask = (usMask << 1) + 1;
		}
		if(usMask == 1) {
		    usMask = 0;   
		}
		
		*pPORTF_FER |= 0x00ff | (usMask << 8);	// enabling PPI data signals 
        *pPORTF_FER |= 0x00ff | (usMask << 8);	// enabling PPI data signals 

        nValue = *pPORTF_MUX;

        switch(usMask) {
            case(0): 
                *pPORTF_MUX &= ~0x0003;
                *pPORTF_MUX &= ~0x0003;
                break;
            case(0x3):
                *pPORTF_MUX &= ~0x000F; // PF8 and PF9
                *pPORTF_MUX &= ~0x000F;
                break;            
            case(0x7):
                *pPORTF_MUX &= ~0x003F; // PF8, PF9, PF10
                *pPORTF_MUX &= ~0x003F;
                break;            
            case(0xF):
                *pPORTF_MUX &= ~0x00FF; // PF8, PF9, PF10, PF11
                *pPORTF_MUX &= ~0x00FF;
                break;            
            case(0x1F):
            case(0x3F):
                *pPORTF_MUX &= ~0x03FF; // PF8, PF9, PF10, PF12, PF13
                *pPORTF_MUX &= ~0x03FF;
                break;            
            case(0x7F):
            case(0xFF):
                *pPORTF_MUX &= ~0x0FFF; // PF8, PF9, PF10, PF12, PF13, PF14, PF15
                *pPORTF_MUX &= ~0x0FFF;
                break;            
        }
        return(true);        
	}
}
