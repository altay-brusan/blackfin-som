#include "../common/PPI.h"

#include <cdefbf537.h>
#include <services/services.h>

T_PPI_SPEC g_aPPIspec[] = {
		{
			false,
			PPI_DEFAULT_DMA_CHANNEL,
			ADI_INT_DMA0_PPI,
			ADI_INT_PERIPHERAL_ERROR,
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
		*pPORTG_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
		*pPORTG_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
	
		// WORKAROUND: Read it onces, write it twice. (See anomaly list.)
		nValue = *pPORTF_FER;
		*pPORTF_FER |= 0x8000;	// enabling PPIClk signal.
		*pPORTF_FER |= 0x8000;	// enabling PPIClk signal.
		return true;
	} else {
		// enable the ppi control signals (PPI_FS1, PPI_FS2, PPI_FS3 and PPIClk	
		unsigned short nValue = *pPORTF_FER;
		*pPORTF_FER |= 0x8380;
		*pPORTF_FER |= 0x8380;
		nValue = *pPORT_MUX;
		*pPORT_MUX |= 0x0100;
		*pPORT_MUX |= 0x0100;
		if ((pa_nPPIcontrol & 0x3800) == 0) {
			// 8 bit non itu 656 mode
			nValue = *pPORTG_FER;
			*pPORTG_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
			*pPORTG_FER |= 0x00ff;	// enabling PPI data signals (D0 - D7).
			return true;
		} else if ((pa_nPPIcontrol & 0x3800) == 0x0800) {
			// 10 bit non itu 656 mode
			nValue = *pPORTG_FER;
			*pPORTG_FER |= 0x03ff;	// enabling PPI data signals (D0 - D9).
			*pPORTG_FER |= 0x03ff;	// enabling PPI data signals (D0 - D9).
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0200;
			*pPORT_MUX &= ~0x0200;
			return true;
		} else if ((pa_nPPIcontrol & 0x3800) == 0x3800) {
			// 16 bit non itu 656 mode
			unsigned short nValue = *pPORTG_FER;
			*pPORTG_FER |= 0xffff;	// enabling PPI data signals (D0 - D15).
			*pPORTG_FER |= 0xffff;	
			nValue = *pPORT_MUX;
			*pPORT_MUX &= ~0x0e00;
			*pPORT_MUX &= ~0x0e00;			
		}
	}
	
	return true;
}

