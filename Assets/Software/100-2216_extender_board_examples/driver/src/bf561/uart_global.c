#include "../common/UART/uart.h"
#include <cdefbf561.h>
#include <services/services.h>

// global data for synchronization of UARTs

bool g_bUARTsInitialized = false;

T_UART_ENTRY g_UARTtable[MAX_NOF_UART];
T_UART_SPEC g_UARTspec[] = {
	{
		ADI_INT_DMA2_5_UART_RX,
		ADI_INT_DMA2_6_UART_TX,
		ADI_INT_UART_ERROR,
        ADI_INT_DMA2_ERROR,
        ADI_INT_DMA2_ERROR,
		pUART_LCR,
		pUART_GCTL,
		pUART_DLL,
		pUART_DLH,
		pUART_IIR,
		pUART_LSR,
		pUART_THR,
		pUART_RBR,
		pUART_IER,
		pDMA2_6_CONFIG,
		(volatile void **)DMA2_6_START_ADDR,
		pDMA2_6_X_COUNT,
		pDMA2_6_X_MODIFY,
		pDMA2_6_Y_COUNT,
		pDMA2_6_Y_MODIFY,
		(volatile void **)DMA2_6_NEXT_DESC_PTR,
		pDMA2_6_IRQ_STATUS,
		pDMA2_5_CONFIG,
		(volatile void **)DMA2_5_START_ADDR,
		pDMA2_5_X_COUNT,
		pDMA2_5_X_MODIFY,
		pDMA2_5_Y_COUNT,
		pDMA2_5_Y_MODIFY,
		(volatile void **)DMA2_5_NEXT_DESC_PTR,
		pDMA2_5_IRQ_STATUS		
	}
};

unsigned int g_nUARTcount = 1;

bool uart_platformInit(int pa_nUARTnumber) {
	// do nothing
	return true;
}

/**
 *	@private
 *	@brief		Initialises the global UART description table
 *	
 **/
void UARTs_init(void) {
#ifdef _USE_VDK_
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);		// Enter a Critical Region (Interrupts Disabled!)
#endif	
	
		int i = 0;
		for (i = 0; i < MAX_NOF_UART; ++i) {
			g_UARTtable[i].nUARTstatus = 0;
		}

#ifdef _USE_VDK_		
	adi_int_ExitCriticalRegion(pExitCriticalArg);		// Exit the Critical Region
#endif	
}
