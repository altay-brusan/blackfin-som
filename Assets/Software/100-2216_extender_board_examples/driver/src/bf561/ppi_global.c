#include "../common/PPI.h"

#include <cdefbf561.h>
#include <services/services.h>

T_PPI_SPEC g_aPPIspec[] = {
		{
			false,
			ADI_DMA_DMA1_0,
			ADI_INT_DMA1_0_PPI0,
			ADI_INT_PPI0_ERROR,
			ADI_INT_DMA1_ERROR,
			0,
			pPPI0_FRAME,
			pPPI0_DELAY,
			pPPI0_COUNT,
			pPPI0_CONTROL,
			pPPI0_STATUS,
			pDMA1_0_IRQ_STATUS,
			pDMA1_0_CONFIG,			
			(volatile void **)pDMA1_0_START_ADDR,
			pDMA1_0_X_MODIFY,
			pDMA1_0_X_COUNT,
			pDMA1_0_CURR_X_COUNT,
			pDMA1_0_Y_MODIFY,
			pDMA1_0_Y_COUNT,
			pDMA1_0_CURR_Y_COUNT,
			(volatile void **)pDMA1_0_NEXT_DESC_PTR,
			0
		},
		{
			false,
			ADI_DMA_DMA1_1,
			ADI_INT_DMA1_1_PPI1,
			ADI_INT_PPI1_ERROR,
			ADI_INT_DMA1_ERROR,
			0,
			pPPI1_FRAME,
			pPPI1_DELAY,
			pPPI1_COUNT,
			pPPI1_CONTROL,
			pPPI1_STATUS,
			pDMA1_1_IRQ_STATUS,
			pDMA1_1_CONFIG,			
			(volatile void **)pDMA1_1_START_ADDR,
			pDMA1_1_X_MODIFY,
			pDMA1_1_X_COUNT,
			pDMA1_1_CURR_X_COUNT,
			pDMA1_1_Y_MODIFY,
			pDMA1_1_Y_COUNT,
			pDMA1_1_CURR_Y_COUNT,
			(volatile void **)pDMA1_1_NEXT_DESC_PTR,			
			0,
		}
	};
	
	
unsigned int g_nPPIcount = sizeof(g_aPPIspec) / sizeof(T_PPI_SPEC);		//calculate the nr of elements in the array
	

bool ppi_platformInit(unsigned char pa_cPPIindex, unsigned short pa_nPPIcontrol) {  
	return true;
}

