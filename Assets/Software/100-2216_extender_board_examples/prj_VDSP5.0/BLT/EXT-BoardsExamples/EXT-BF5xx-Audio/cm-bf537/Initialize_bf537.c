#include "Talkthrough.h"

//--------------------------------------------------------------------------//
// Function:	Init1836()													//
//																			//
// Description:	This function sets up the SPI port to configure the AD1836. //
//				The content of the array sCodec1836TxRegs is sent to the 	//
//				codec.														//
//--------------------------------------------------------------------------//
void Init1836(void)
{
	int j;
	
	// enable spi signals in port mux
	// enable MOSI, MISO and SCK pins
	unsigned short nValue = *pPORTF_FER;
	*pPORTF_FER |= 0x3800;
	*pPORTF_FER |= 0x3800;
	
	// enable SPISSEL5 pin
	nValue = *pPORTF_FER;
	*pPORTF_FER |= 0x0020;
	*pPORTF_FER |= 0x0020;

	nValue = *pPORT_MUX;
	*pPORT_MUX |= 0x0040;
	*pPORT_MUX |= 0x0040;

	// enable SPISSEL 5
	*pSPI_FLG |= FLS5;

	// Set baud rate SCK = HCLK/(2*SPIBAUD) SCK = 2MHz	
	*pSPI_BAUD = 16;
	// configure spi port
	// SPI DMA write, 16-bit data, MSB first, SPI Master
	*pSPI_CTL = TIMOD_DMA_TX | SIZE | MSTR;
	
	// Set up DMA7 to transmit
	// Map DMA7 to SPI
	*pDMA7_PERIPHERAL_MAP	= 0x7000;
	
	// Configure DMA5
	// 16-bit transfers
	*pDMA7_CONFIG = WDSIZE_16;
	// Start address of data buffer
	*pDMA7_START_ADDR = (void *)sCodec1836TxRegs;
	// DMA inner loop count
	*pDMA7_X_COUNT = CODEC_1836_REGS_LENGTH;
	// Inner loop address increment
	*pDMA7_X_MODIFY = 2;
	
	// enable DMAs
	*pDMA7_CONFIG = (*pDMA7_CONFIG | DMAEN);
	// enable spi
	*pSPI_CTL = (*pSPI_CTL | SPE);
	
	// wait until dma transfers for spi are finished 
	for (j=0; j<0xaff0; j++) asm("nop;");
	
	// disable spi
	*pSPI_CTL = 0x0000;
}


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
	*pPORT_MUX &= ~0x0007;
	
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
	*pDMA3_PERIPHERAL_MAP = 0x3000;
	
	// Configure DMA1
	// 32-bit transfers, Interrupt on completion, Autobuffer mode
	*pDMA3_CONFIG = WNR | WDSIZE_32 | DI_EN | FLOW_1;
	// Start address of data buffer
	*pDMA3_START_ADDR = (void *)iRxBuffer1;
	// DMA inner loop count
	*pDMA3_X_COUNT = 8;
	// Inner loop address increment
	*pDMA3_X_MODIFY	= 4;
	
	
	// Set up DMA4 to transmit
	// Map DMA4 to Sport0 TX
	*pDMA4_PERIPHERAL_MAP = 0x4000;
	
	// Configure DMA2
	// 32-bit transfers, Autobuffer mode
	*pDMA4_CONFIG = WDSIZE_32 | FLOW_1;
	// Start address of data buffer
	*pDMA4_START_ADDR = (void *)iTxBuffer1;
	// DMA inner loop count
	*pDMA4_X_COUNT = 8;
	// Inner loop address increment
	*pDMA4_X_MODIFY	= 4;
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
	adi_int_SICGetIVG(ADI_INT_DMA3_SPORT0_RX, &nCurrentIVG);

	// hook the sport0 rx interrupt handler 
	adi_int_CECHook(nCurrentIVG, SPORT0RxHandler, NULL, FALSE);
	adi_int_SICWakeup(ADI_INT_DMA3_SPORT0_RX, TRUE);
	adi_int_SICEnable(ADI_INT_DMA3_SPORT0_RX);
		
}


//--------------------------------------------------------------------------//
// Function:	Enable_DMA_Sport											//
//																			//
// Description:	Enable DMA3, DMA4, Sport0 TX and Sport0 RX					//
//--------------------------------------------------------------------------//
void Enable_DMA_Sport0(void)
{
	// enable DMAs
	*pDMA4_CONFIG	= (*pDMA4_CONFIG | DMAEN);
	*pDMA3_CONFIG	= (*pDMA3_CONFIG | DMAEN);
	
	// enable Sport0 TX and RX
	*pSPORT0_TCR1 	= (*pSPORT0_TCR1 | TSPEN);
	*pSPORT0_RCR1 	= (*pSPORT0_RCR1 | RSPEN);
}
