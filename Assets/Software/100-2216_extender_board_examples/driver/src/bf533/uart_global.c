#include "../common/UART/uart.h"
#include <cdefbf533.h>
#include <services/services.h>

// global data for synchronization of UARTs

bool g_bUARTsInitialized = false;

T_UART_ENTRY g_UARTtable[MAX_NOF_UART];
T_UART_SPEC g_UARTspec[] = {
	{
		ADI_INT_DMA6_UART_RX,
		ADI_INT_DMA7_UART_TX,
		ADI_INT_UART_ERROR,
		ADI_INT_DMA_ERROR,
		ADI_INT_DMA_ERROR,
		pUART_LCR,
		pUART_GCTL,
		pUART_DLL,
		pUART_DLH,
		pUART_IIR,
		pUART_LSR,
		pUART_THR,
		pUART_RBR,
		pUART_IER,
		pDMA7_CONFIG,
		(volatile void **)DMA7_START_ADDR,
		pDMA7_X_COUNT,
		pDMA7_X_MODIFY,
		pDMA7_Y_COUNT,
		pDMA7_Y_MODIFY,
		(volatile void **)DMA7_NEXT_DESC_PTR,
		pDMA7_IRQ_STATUS,
		pDMA6_CONFIG,
		(volatile void **)DMA6_START_ADDR,
		pDMA6_X_COUNT,
		pDMA6_X_MODIFY,
		pDMA6_Y_COUNT,
		pDMA6_Y_MODIFY,
		(volatile void **)DMA6_NEXT_DESC_PTR,
		pDMA6_IRQ_STATUS
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

