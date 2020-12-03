#include "../common/PPI.h"

#include <cdefbf533.h>
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
			(volatile void **)DMA0_NEXT_DESC_PTR,
			0
		}		
	};

unsigned int g_nPPIcount = sizeof(g_aPPIspec) / sizeof(T_PPI_SPEC);		//calculate the nr of elements in the array
	
bool ppi_platformInit(unsigned char pa_cPPIindex, unsigned short pa_nPPIcontrol) {
	return true;
}

