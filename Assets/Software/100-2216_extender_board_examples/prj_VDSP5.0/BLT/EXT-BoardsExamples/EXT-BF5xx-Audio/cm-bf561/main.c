//--------------------------------------------------------------------------//
//																			//
//	 Name: 	Talkthrough for the EXT-BF5xx-Audio board						//
//																			//
//--------------------------------------------------------------------------//
//																			//
//	Hardware:		EXT-BF5xx-Audio board									//
//																			//
//	Connections:	Connect an input source (such as a radio) to the Audio	//
//					input jack and an output source (such as headphones) to //
//					the Audio output jack									//
//																			//
//	Purpose:		This program sets up the SPI port on the ADSP-BF5xx to  //
//					configure the AD1836 codec.  The SPI port is disabled 	//
//					after initialization.  The data to/from the codec are 	//
//					transfered over SPORT0 in TDM mode						//
//																			//
//--------------------------------------------------------------------------//

#include "Talkthrough.h"
#include "sysreg.h"
#include "../../../../../driver/src/common/gpioconfig.h"
#include <math.h>
#include "../../../../../driver/src/common/pwrmngt/clockquery.h"
#include <adi_ssl_init.h>
#include "../../../../../driver/src/common/SPI/SPI.h"
#include <environment.h>


#define BUTTON_FLAG				_PF46
#define LED_FLAG				_PF43

//--------------------------------------------------------------------------//
// Variables																//
//																			//
// Description:	The variables iChannelxLeftIn and iChannelxRightIn contain 	//
//				the data coming from the codec AD1836.  The (processed)		//
//				playback data are written into the variables 				//
//				iChannelxLeftOut and iChannelxRightOut respectively, which 	//
//				are then sent back to the codec in the SPORT0 ISR.  		//
//				The values in the array iCodec1836TxRegs can be modified to //
//				set up the codec in different configurations according to   //
//				the AD1885 data sheet.										//
//--------------------------------------------------------------------------//
// left input data from ad1836
int iChannel0LeftIn, iChannel1LeftIn;
// right input data from ad1836
int iChannel0RightIn, iChannel1RightIn;
// left ouput data for ad1836	
int iChannel0LeftOut, iChannel1LeftOut, iChannel2LeftOut;
// right ouput data for ad1836
int iChannel0RightOut, iChannel1RightOut, iChannel2RightOut;
// array for registers to configure the ad1836
// names are defined in "Talkthrough.h"
unsigned short sCodec1836TxRegs[CODEC_1836_REGS_LENGTH] =
{									
					DAC_CONTROL_1	| 0x000,
					DAC_CONTROL_2	| 0x000,
					DAC_VOLUME_0	| 0x3ff,
					DAC_VOLUME_1	| 0x3ff,
					DAC_VOLUME_2	| 0x3ff,
					DAC_VOLUME_3	| 0x3ff,
					DAC_VOLUME_4	| 0x3ff,
					DAC_VOLUME_5	| 0x3ff,
					ADC_CONTROL_1	| 0x000,
					ADC_CONTROL_2	| 0x180,
					ADC_CONTROL_3	| 0x000
					
};
// SPORT0 DMA transmit buffer
volatile int iTxBuffer1[8];
// SPORT0 DMA receive buffer
volatile int iRxBuffer1[8];

volatile bool bToggleInput = false;

#pragma alignment_region (4)
static unsigned char s_cDevMngStorage[ADI_DEV_BASE_MEMORY + (ADI_DEV_DEVICE_MEMORY * 5)];
static unsigned char s_cIntMngStorage[ADI_INT_SECONDARY_MEMORY * BS_INT_MNG_NOF_SECONDARY_HANDLER * BS_NOF_USER_IVGS];                         ///< Pointer to an area of memory used by the interrupt manager
static u8 DMAMgrData[ADI_DMA_BASE_MEMORY + (ADI_DMA_CHANNEL_MEMORY * 4)];
#pragma alignment_region_end

ADI_DEV_MANAGER_HANDLE g_hADIDevMng;                     // ADI Device Manager Handle

unsigned long g_nCoreClk;

#ifdef _MEASURE_SAMPLING_RATE_
float g_fSamplingRate = 0.0;
#endif

/* --------------------------------------------------------------------------
   Function:	main														
																			
   Description:	After calling a few initalization routines, main() remains  
				in a loop forever by checking a button. If the button was pressed
				the input source is changed.
				The user can place the code to process the data in the function
				Process_Data() in the file "Process_Data.c".										
//-------------------------------------------------------------------------- */
void main(void)
{
    unsigned long response_count;
	unsigned long  critical_reg;
	unsigned long cclk, sclk, vco;							// frequencies
	
    // initialize the ADI Service System Libraries
    unsigned long nResult = ADISSLinit(&g_hADIDevMng, 
                                       s_cIntMngStorage, 
                                       sizeof(s_cIntMngStorage),
                                       s_cDevMngStorage,
                                       sizeof(s_cDevMngStorage));
	
	
	cclk = getCoreClockFrequency();
	sclk = getSystemClockFrequency();
	
	
	g_nCoreClk = cclk;
	  
    SPIsetup(0, sclk, cclk, true);
    
    T_SPI_CONFIG tSPIConfig;
    tSPIConfig.mbMaSl = true;
    tSPIConfig.mtCSEL = (_PF46 | 0x80000000);
    tSPIConfig.mnRxBufferSize = 0;
    tSPIConfig.mnTxBufferSize = 0;
    tSPIConfig.mfnDataCallback = 0;
    tSPIConfig.mnBaudrate = 2000000;
    tSPIConfig.mnTransferSize = SPI_16BIT;
    tSPIConfig.mnClockPolarity = SPI_CPOL_HIGH;
    tSPIConfig.mnClockPhase = SPI_CPHA_HIGH;
    tSPIConfig.mnMsbLsbFirst = SPI_MSB_FIRST;
    tSPIConfig.mnMasterErrorFlag = SPI_NONE;
    tSPIConfig.msDefaultWord = 0;
    T_ERROR_CODE tError = ERR_NONE;            
    
    T_SPI_HANDLE hSPI = SPIopen(0, &tSPIConfig, &tError);
    
    unsigned char ucIndex;
    for(ucIndex=0; ucIndex < CODEC_1836_REGS_LENGTH; ucIndex++) {
        // init AD1836
        SPIselect(hSPI);
        SPIwrite(hSPI, &sCodec1836TxRegs[ucIndex], 2, 0);
        SPIdeselect(hSPI);
    }
    
    Init_Sport0();
	Init_DMA();
	Init_Sport_Interrupts();
	Enable_DMA_Sport0();
	
	gpio_becomeInput(BUTTON_FLAG);		// set gpio for button as input
	gpio_becomeOutput(LED_FLAG);		// set gpio for led as output

#ifdef _MEASURE_SAMPLING_RATE_
	// configure core timer to measure sampling rate
	*pTCNTL = 5;
	*pTPERIOD = 0xffffffff;
	*pTSCALE = 0;
	*pTCOUNT = 0xffffffff;
#endif	
	//gpio_becomeOutput(_PG0);		

	unsigned long nWait;
	while (1) {
	    // wait for button pressing
	    while (gpio_readFlag(BUTTON_FLAG));
	    // the button was pressed so change input
	    bToggleInput = !bToggleInput;
	    // toggle led
	    gpio_toggle(LED_FLAG);
	    // wait until button is stable
	    for (nWait=0; nWait<0x1ffffff; nWait++);
	} 
}
