//--------------------------------------------------------------------------//
// Function:	Sport0_RX_ISR												//
//																			//
// Description: This ISR is executed after a complete frame of input data 	//
//				has been received. The new samples are stored in 			//
//				iChannel0LeftIn, iChannel0RightIn, iChannel1LeftIn and 		//
//				iChannel1RightIn respectively.  Then the function 			//
//				Process_Data() is called in which user code can be executed.//
//				After that the processed values are copied from the 		//
//				variables iChannel0LeftOut, iChannel0RightOut, 				//
//				iChannel1LeftOut and iChannel1RightOut into the dma 		//
//				transmit buffer.											//
//--------------------------------------------------------------------------//
#include "Talkthrough.h"

#ifdef _MEASURE_SAMPLING_RATE_
extern float g_fSamplingRate;				// declared in main.c
extern unsigned long g_nCoreClk;			// declared in main.c
#endif

ADI_INT_HANDLER_RESULT SPORT0RxHandler(void *pa_pClientArg) 
{
#ifdef _MEASURE_SAMPLING_RATE_
   	// disable core timer
	*pTCNTL &= ~2;	
#endif	
    
	*pDMA2_0_IRQ_STATUS |= 0x0001;

	// copy input data from dma input buffer into variables
	iChannel0LeftIn = iRxBuffer1[INTERNAL_ADC_L0];
	iChannel0RightIn = iRxBuffer1[INTERNAL_ADC_R0];
	iChannel1LeftIn = iRxBuffer1[INTERNAL_ADC_L1];
	iChannel1RightIn = iRxBuffer1[INTERNAL_ADC_R1];

	// call function that contains user code
	Process_Data();				

	// copy processed data from variables into dma output buffer
	iTxBuffer1[INTERNAL_DAC_L0] = iChannel0LeftOut;
	iTxBuffer1[INTERNAL_DAC_R0] = iChannel0RightOut;
	iTxBuffer1[INTERNAL_DAC_L1] = iChannel1LeftOut;
	iTxBuffer1[INTERNAL_DAC_R1] = iChannel1RightOut;
	iTxBuffer1[INTERNAL_DAC_L2] = iChannel2LeftOut;
	iTxBuffer1[INTERNAL_DAC_R2] = iChannel2RightOut;
	
#ifdef _MEASURE_SAMPLING_RATE_
	// calculate sampling rate
	unsigned long nCountVal = 0xffffffff - *pTCOUNT;
	g_fSamplingRate = 1.0 / ((float)nCountVal * (1.0 / (float)g_nCoreClk));
	// reset count value
	*pTCOUNT = 0xffffffff;
	// enable core timer
	*pTCNTL |= 2;	
#endif
//	gpio_toggle(_PG0);
	
	return ADI_INT_RESULT_PROCESSED;
}

