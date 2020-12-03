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
#include <cdefbf533.h>

// -- CHOOSE A BASE BOARD ------------------------------------------------- //
//#define DEV_BF5xx_V2_x
//#define EVAL_BF5xx_V3_x
#define EVAL_BF5xx_V4_x
//#define DEV_BF5xx_DA_LITE
//#define DEV_BF5xx_FPGA
// -------------------------------------------------------------------------//

#define CLKIN							25

#if defined (DEV_BF5xx_V2_x)
	#if defined (__ADSPBF533__)
		#define BUTTON_FLAG				_PF5
		#define LED_FLAG				_PF8
	#elif defined (__ADSPBF537__)
		#define BUTTON_FLAG				_PG14
		#define LED_FLAG				_PG11
	#elif defined (__ADSPBF561__)
		#define BUTTON_FLAG				_PF46
		#define LED_FLAG				_PF43
	#endif
#elif defined (EVAL_BF5xx_V3_x)
	#if defined (__ADSPBF533__)
		#define BUTTON_FLAG				_PF5
		#define LED_FLAG				_PF8
	#elif defined (__ADSPBF537__)
		#define BUTTON_FLAG				_PG14
		#define LED_FLAG				_PG11
	#elif defined (__ADSPBF561__)
		#define BUTTON_FLAG				_PF46
		#define LED_FLAG				_PF43
	#endif
#elif defined (EVAL_BF5xx_V4_x)
	#if defined (__ADSPBF533__)
		#define BUTTON_FLAG				_PF5
		#define LED_FLAG				_PF8
	#elif defined (__ADSPBF537__)
		#define BUTTON_FLAG				_PG14
		#define LED_FLAG				_PG11
	#elif defined (__ADSPBF561__)
		#define BUTTON_FLAG				_PF46
		#define LED_FLAG				_PF43
	#endif
#elif defined (DEV_BF5xx_DA_LITE)
	#if defined (__ADSPBF533__)
		#define BUTTON_FLAG				_PF5
		#define LED_FLAG				_PF8
	#elif defined (__ADSPBF537__)
		#define BUTTON_FLAG				_PG14
		#define LED_FLAG				_PG11
	#elif defined (__ADSPBF561__)
		#define BUTTON_FLAG				_PF46
		#define LED_FLAG				_PF43
	#endif
#elif defined (DEV_BF5xx_FPGA)
	#if defined (__ADSPBF533__)
		#define BUTTON_FLAG				_PF5
		#define LED_FLAG				_PF8
	#elif defined (__ADSPBF537__)
		#define BUTTON_FLAG				_PF14
		#define LED_FLAG				_PF11
	#elif defined (__ADSPBF561__)
		#define BUTTON_FLAG				_PF46
		#define LED_FLAG				_PF43
	#endif
#else
#error "base board not supported"
#endif
//
//


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
volatile short sCodec1836TxRegs[CODEC_1836_REGS_LENGTH] =
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
static unsigned char intmgr_storage[(ADI_INT_SECONDARY_MEMORY * 8)];
unsigned long g_nCoreClk = 250000000;

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

	// initialize the interrupt manager
	adi_int_Init( intmgr_storage, sizeof(intmgr_storage), &response_count, &critical_reg);

#ifdef _USE_POWER_MANAGMENT_SERVICE_		
	// initialize power managment service
	ADI_PWR_COMMAND_PAIR PowerTable [] = {	// configuration table for power management	
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)ADI_PWR_PROC_BF537SKBC600 },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)ADI_PWR_PACKAGE_MBGA },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)ADI_PWR_VDDEXT_330 }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)CLKIN },		
		{ ADI_PWR_CMD_END,			NULL	},
	};
	
	// initialize power management
	int result = adi_pwr_Init(PowerTable);
	if (result != ADI_DEV_RESULT_SUCCESS) {
	    // error
		while (1);
	}
	
	//set optimal frequencies
	result = adi_pwr_SetFreq(600, 120, ADI_PWR_DF_ON);
	if (result != ADI_DEV_RESULT_SUCCESS) {
	    // error
		while (1);
	}
	
	unsigned long cclk, sclk, vco;							// frequencies
	adi_pwr_GetFreq(&cclk, &sclk, &vco);
	
	g_nCoreClk = cclk;
#endif	
	  
	Init1836();
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
