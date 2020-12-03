#include "Talkthrough.h"
#include <cdefBF518.h>


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
	unsigned short nValue = *pPORTG_FER;
	*pPORTG_FER |= 0x7000;
	*pPORTG_FER |= 0x7000;
		
	// set SPI CS
	nValue = *pPORTGIO_SET;
    *pPORTGIO_SET |= 0x0001;	
    *pPORTGIO_SET |= 0x0001;	
	
	nValue = *pPORTGIO_DIR;
    *pPORTGIO_DIR |= 0x0001;	
    *pPORTGIO_DIR |= 0x0001;

	// Set baud rate SCK = HCLK/(2*SPIBAUD) SCK = 2MHz	
	*pSPI0_BAUD = 16;
	// configure spi port
	// SPI DMA write, 16-bit data, MSB first, SPI Master
	*pSPI0_CTL = SIZE | MSTR;
	
	// Set up DMA7 to transmit
	// Map DMA7 to SPI
	/* *pDMA7_PERIPHERAL_MAP	= 0x7000;
	
	// Configure DMA7
	// 16-bit transfers
	*pDMA7_CONFIG = WDSIZE_16 | DI_EN | DI_SEL | DMA2D;
	// Start address of data buffer
	*pDMA7_START_ADDR = (void *)sCodec1836TxRegs;
	// DMA inner loop count
	*pDMA7_X_COUNT = 2;
	*pDMA7_Y_COUNT = CODEC_1836_REGS_LENGTH;
	// Inner loop address increment
	*pDMA7_X_MODIFY = 2;
	*pDMA7_Y_MODIFY = 1;
	
	unsigned long nCurrentIVG;
	adi_int_SICGetIVG(ADI_INT_DMA7_SPI0, &nCurrentIVG);

	// hook the sport1 rx interrupt handler 
	adi_int_CECHook(nCurrentIVG, SPI0Handler, NULL, FALSE);
	adi_int_SICWakeup(ADI_INT_DMA7_SPI0, TRUE);
	adi_int_SICEnable(ADI_INT_DMA7_SPI0);
	
	// enable DMAs
	*pDMA7_CONFIG = (*pDMA7_CONFIG | DMAEN);
	*/
	// clear CS
    
	// enable spi
	*pSPI0_CTL = (*pSPI0_CTL | SPE);

    unsigned char ucIndex = 0;
    for(ucIndex = 0; ucIndex < CODEC_1836_REGS_LENGTH; ucIndex++) {
    	nValue = *pPORTGIO_CLEAR;
        *pPORTGIO_CLEAR |= 0x0001;	
        *pPORTGIO_CLEAR |= 0x0001;	
        
        while ((*pSPI0_STAT & 0x0008) == 0x0008);
        *pSPI0_TDBR = sCodec1836TxRegs[ucIndex];
        //while ((*pSPI0_STAT & 0x0020) == 0x0000);
        unsigned short nDummyValue = *pSPI0_RDBR;
       
        unsigned char nTemp = 20;
        while(--nTemp)
            asm("nop;");
         
        nValue = *pPORTGIO_SET;
        *pPORTGIO_SET |= 0x0001;	
        *pPORTGIO_SET |= 0x0001;	
        
        nTemp = 10;
        while(--nTemp)
            asm("nop;");

    }
	
	// wait until dma transfers for spi are finished 
	//for (j=0; j<0xaff0; j++) asm("nop;");
	
	// disable spi
	*pSPI0_CTL = 0x0000;
/*	nValue = *pPORTGIO_SET;
    *pPORTGIO_SET |= 0x0001;	
    *pPORTGIO_SET |= 0x0001;	*/
	
	
}


//--------------------------------------------------------------------------//
// Function:	Init_Sport1													//
//																			//
// Description:	Configure Sport1 for TDM mode, to transmit/receive data 	//
//				to/from the AD1836. Configure Sport for external clocks and //
//				frame syncs.												//
//--------------------------------------------------------------------------//
void Init_Sport1(void)
{
	unsigned short nValue = *pPORTH_FER;
	*pPORTH_FER |= 0x00FF;
	*pPORTH_FER |= 0x00FF;
	
	// Sport1 receive configuration
	// External CLK, External Frame sync, MSB first
	// 32-bit data
	*pSPORT1_RCR1 = RFSR;
	*pSPORT1_RCR2 = SLEN_32;
	
	// Sport1 transmit configuration
	// External CLK, External Frame sync, MSB first
	// 24-bit data
	*pSPORT1_TCR1 = TFSR;
	*pSPORT1_TCR2 = SLEN_32;
	
	// Enable MCM 8 transmit & receive channels
	*pSPORT1_MTCS0 = 0x000000FF;
	*pSPORT1_MRCS0 = 0x000000FF;
	
	// Set MCM configuration register and enable MCM mode
	*pSPORT1_MCMC1 = 0x0000;
	*pSPORT1_MCMC2 = 0x101c;
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
	// Map DMA3 to Sport1 RX
	*pDMA5_PERIPHERAL_MAP = 0x5000;
	
	// Configure DMA1
	// 32-bit transfers, Interrupt on completion, Autobuffer mode
	*pDMA5_CONFIG = WNR | WDSIZE_32 | DI_EN | FLOW_1;
	// Start address of data buffer
	*pDMA5_START_ADDR = (void *)iRxBuffer1;
	// DMA inner loop count
	*pDMA5_X_COUNT = 8;
	// Inner loop address increment
	*pDMA5_X_MODIFY	= 4;
	
	
	// Set up DMA4 to transmit
	// Map DMA4 to Sport1 TX
	*pDMA6_PERIPHERAL_MAP = 0x6000;
	
	// Configure DMA2
	// 32-bit transfers, Autobuffer mode
	*pDMA6_CONFIG = WDSIZE_32 | FLOW_1;
	// Start address of data buffer
	*pDMA6_START_ADDR = (void *)iTxBuffer1;
	// DMA inner loop count
	*pDMA6_X_COUNT = 8;
	// Inner loop address increment
	*pDMA6_X_MODIFY	= 4;
}

//--------------------------------------------------------------------------//
// Function:	Init_Interrupts												//
//																			//
// Description:	Initialize Interrupt for Sport1 RX							//
//--------------------------------------------------------------------------//
void Init_Sport_Interrupts(void)
{

	// using the adi interrupt manager
	// get the IVG
	unsigned long nCurrentIVG;
	adi_int_SICGetIVG(ADI_INT_DMA5_SPORT1_RX_AND_SPI1, &nCurrentIVG);

	// hook the sport1 rx interrupt handler 
	adi_int_CECHook(nCurrentIVG, SPORT1RxHandler, NULL, FALSE);
	adi_int_SICWakeup(ADI_INT_DMA5_SPORT1_RX_AND_SPI1, TRUE);
	adi_int_SICEnable(ADI_INT_DMA5_SPORT1_RX_AND_SPI1);
		
}


//--------------------------------------------------------------------------//
// Function:	Enable_DMA_Sport											//
//																			//
// Description:	Enable DMA3, DMA4, Sport1 TX and Sport1 RX					//
//--------------------------------------------------------------------------//
void Enable_DMA_Sport1(void)
{
	// enable DMAs
	*pDMA6_CONFIG	= (*pDMA6_CONFIG | DMAEN);
	*pDMA5_CONFIG	= (*pDMA5_CONFIG | DMAEN);
	
	// enable Sport1 TX and RX
	*pSPORT1_TCR1 	= (*pSPORT1_TCR1 | TSPEN);
	*pSPORT1_RCR1 	= (*pSPORT1_RCR1 | RSPEN);
}
