#include "../common/UART/uart.h"
#include <cdefbf537.h>
#include <services/services.h>

// global data for synchronization of UARTs

bool g_bUARTsInitialized = false;

T_UART_ENTRY g_UARTtable[MAX_NOF_UART];

T_UART_SPEC g_UARTspec[] = {
	{
		ADI_INT_DMA8_UART0_RX,
		ADI_INT_DMA9_UART0_TX,
		ADI_INT_PERIPHERAL_ERROR,
		ADI_INT_DMA_ERROR,
		ADI_INT_DMA_ERROR,
		pUART0_LCR,
		pUART0_GCTL,
		pUART0_DLL,
		pUART0_DLH,
		pUART0_IIR,
		pUART0_LSR,
		pUART0_THR,
		pUART0_RBR,
		pUART0_IER,
		pDMA9_CONFIG,
		(volatile void **)DMA9_START_ADDR,
		pDMA9_X_COUNT,
		pDMA9_X_MODIFY,
		pDMA9_Y_COUNT,
		pDMA9_Y_MODIFY,
		(volatile void **)DMA9_NEXT_DESC_PTR,
		pDMA9_IRQ_STATUS,
		pDMA8_CONFIG,
		(volatile void **)DMA8_START_ADDR,
		pDMA8_X_COUNT,
		pDMA8_X_MODIFY,
		pDMA8_Y_COUNT,
		pDMA8_Y_MODIFY,
		(volatile void **)DMA8_NEXT_DESC_PTR,
		pDMA8_IRQ_STATUS
	},
	{
		ADI_INT_DMA10_UART1_RX,
		ADI_INT_DMA11_UART1_TX,
		ADI_INT_PERIPHERAL_ERROR,
		ADI_INT_DMA_ERROR,
		ADI_INT_DMA_ERROR,		
		pUART1_LCR,
		pUART1_GCTL,
		pUART1_DLL,
		pUART1_DLH,
		pUART1_IIR,
		pUART1_LSR,
		pUART1_THR,
		pUART1_RBR,
		pUART1_IER,
		pDMA11_CONFIG,
		(volatile void **)DMA11_START_ADDR,
		pDMA11_X_COUNT,
		pDMA11_X_MODIFY,
		pDMA11_Y_COUNT,
		pDMA11_Y_MODIFY,
		(volatile void **)DMA11_NEXT_DESC_PTR,
		pDMA11_IRQ_STATUS,
		pDMA10_CONFIG,
		(volatile void **)DMA10_START_ADDR,
		pDMA10_X_COUNT,
		pDMA10_X_MODIFY,
		pDMA10_Y_COUNT,
		pDMA10_Y_MODIFY,
		(volatile void **)DMA10_NEXT_DESC_PTR,
		pDMA10_IRQ_STATUS
	}
};

unsigned int g_nUARTcount = sizeof (g_UARTspec) / sizeof (T_UART_SPEC);

bool uart_platformInit(int pa_nUARTnumber) {
	bool bResult = true;
	
	if (pa_nUARTnumber == 0) {
		//enable peripheral functionality for PF0 and PF1. (TX0 and RX0)
		//WORKAROUND: Read it onces and write it twices. (See bf537 anomaly list)
		unsigned short nValue = *pPORTF_FER;
		*pPORTF_FER |= 0x0003;	
		*pPORTF_FER |= 0x0003;
	
		//MAP UART0 RX and TX pin to output.
		//WORKAROUND: Read it onces and write it twices. (See bf537 anomaly list)
		nValue =	*pPORT_MUX;
		*pPORT_MUX &= ~0x0008;	
		*pPORT_MUX &= ~0x0008;
	}
	else if (pa_nUARTnumber == 1) {
		//enable peripheral functionality for PF2 and PF3. (TX1 and RX1)
		//WORKAROUND: Read it onces and write it twices. (See bf537 anomaly list)
		unsigned short nValue = *pPORTF_FER;
		*pPORTF_FER |= 0x000c;	
		*pPORTF_FER |= 0x000c;
		
		//MAP UART1 RX and TX pin to output.
		//WORKAROUND: Read it onces and write it twices. (See bf537 anomaly list)
		nValue =	*pPORT_MUX;
		*pPORT_MUX &= ~0x0010;	
		*pPORT_MUX &= ~0x0010;
	} else {
		bResult = false;
	}
	
	return bResult;
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
