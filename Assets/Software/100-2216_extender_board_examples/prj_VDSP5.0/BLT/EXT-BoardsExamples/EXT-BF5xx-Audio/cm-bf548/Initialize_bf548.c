#include "Talkthrough.h"


//--------------------------------------------------------------------------//
// Function:	Init_Sport0													//
//																			//
// Description:	Configure Sport0 for TDM mode, to transmit/receive data 	//
//				to/from the AD1836. Configure Sport for external clocks and //
//				frame syncs.												//
//--------------------------------------------------------------------------//
void Init_Sport0(void)
{
	// enable SPORT0 pins
	*pPORTC_FER |= 0x00FF;
	
	// Sport0 receive configuration
	// External CLK, External Frame sync, MSB first
	// 32-bit data
	*pSPORT0_RCR1 = RFSR;
	*pSPORT0_RCR2 = SLEN_32;
	
	// Sport0 transmit configuration
	// External CLK, External Frame sync, MSB first
	// 24-bit data
	*pSPORT0_TCR1 = TFSR;
	*pSPORT0_TCR2 = SLEN_32;
	
	// Enable MCM 8 transmit & receive channels
	*pSPORT0_MTCS0 = 0x000000FF;
	*pSPORT0_MRCS0 = 0x000000FF;
	
	// Set MCM configuration register and enable MCM mode
	*pSPORT0_MCMC1 = 0x0000;
	*pSPORT0_MCMC2 = 0x101c;
}


//--------------------------------------------------------------------------//
// Function:	Init_DMA													//
//																			//
// Description:	Initialize DMA3 in autobuffer mode to receive and DMA4 in	//
//				autobuffer mode to transmit									//
//--------------------------------------------------------------------------//
void Init_DMA(void)
{
	// Set up DMA3 to receive
	// Map DMA3 to Sport0 RX
	*pDMA0_PERIPHERAL_MAP = 0x0000;
	
	// Configure DMA1
	// 32-bit transfers, Interrupt on completion, Autobuffer mode
	*pDMA0_CONFIG = WNR | WDSIZE_32 | DI_EN | FLOW_1;
	// Start address of data buffer
	*pDMA0_START_ADDR = (void *)iRxBuffer1;
	// DMA inner loop count
	*pDMA0_X_COUNT = 8;
	// Inner loop address increment
	*pDMA0_X_MODIFY	= 4;
	
	
	// Set up DMA4 to transmit
	// Map DMA4 to Sport0 TX
	*pDMA1_PERIPHERAL_MAP = 0x1000;
	
	// Configure DMA2
	// 32-bit transfers, Autobuffer mode
	*pDMA1_CONFIG = WDSIZE_32 | FLOW_1;
	// Start address of data buffer
	*pDMA1_START_ADDR = (void *)iTxBuffer1;
	// DMA inner loop count
	*pDMA1_X_COUNT = 8;
	// Inner loop address increment
	*pDMA1_X_MODIFY	= 4;
}

//--------------------------------------------------------------------------//
// Function:	Init_Interrupts												//
//																			//
// Description:	Initialize Interrupt for Sport0 RX							//
//--------------------------------------------------------------------------//
void Init_Sport_Interrupts(void)
{

	// using the adi interrupt manager
	// get the IVG
	unsigned long nCurrentIVG;
	adi_int_SICGetIVG(ADI_INT_DMA0_SPORT0_RX, &nCurrentIVG);

	// hook the sport0 rx interrupt handler 
	adi_int_CECHook(nCurrentIVG, SPORT0RxHandler, NULL, FALSE);
	adi_int_SICWakeup(ADI_INT_DMA0_SPORT0_RX, TRUE);
	adi_int_SICEnable(ADI_INT_DMA0_SPORT0_RX);
		
}


//--------------------------------------------------------------------------//
// Function:	Enable_DMA_Sport											//
//																			//
// Description:	Enable DMA3, DMA4, Sport0 TX and Sport0 RX					//
//--------------------------------------------------------------------------//
void Enable_DMA_Sport0(void)
{
	// enable DMAs
	*pDMA1_CONFIG	= (*pDMA1_CONFIG | DMAEN);
	*pDMA0_CONFIG	= (*pDMA0_CONFIG | DMAEN);
	
	// enable Sport0 TX and RX
	*pSPORT0_TCR1 	= (*pSPORT0_TCR1 | TSPEN);
	*pSPORT0_RCR1 	= (*pSPORT0_RCR1 | RSPEN);
}
