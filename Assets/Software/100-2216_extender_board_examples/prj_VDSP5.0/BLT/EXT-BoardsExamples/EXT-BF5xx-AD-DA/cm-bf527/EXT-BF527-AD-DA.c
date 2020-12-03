/*****************************************************************************
 * EXT-BF561-AD-DA.c
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <drivers/adi_dev.h>
#include <cycle_count_bf.h>

#include "Environment.h"
#include "EXT-BF527-AD-DA.h"
 
static u8 intmgr_storage[(ADI_INT_SECONDARY_MEMORY * 8)];

//unsigned long long g_cur[7];
bool g_bAD5405B1ready;
bool g_bAD5405B2ready;

void main( void ) {
    
    u32 response_count;
	u32 critical_reg;
	unsigned long cclk; 							// frequencies
	unsigned long sclk;
	unsigned long vco;
	unsigned short nBurstSizeAD5415;
	
#ifdef USE_PROCESS_DATA
	unsigned short aProcessBuffer[(AD7266_DEFAULT_BURST_SIZE + 50) * 2];
#endif

	unsigned short i, iMax;
	unsigned short iSin, iRamp;
	unsigned short iSinMax, iRampMax;
	unsigned short iBuf1, iBuf2;
	int result;
	
	T_AD5405_CONFIG aAD5405Configuration;
	
	T_ERROR_CODE erResultAD5405;
	T_ERROR_CODE erResultAD5405WriteB1;
	T_ERROR_CODE erResultAD5405WriteB2;
	T_ERROR_CODE erPrepBuf;
	iSin = 0;
	iRamp = 0;
	iBuf1 = 0;
	iBuf2 = 0;
	
	unsigned short anSinCurve[] = {                 // predefined curve for AD5415/AD5405 output
	    2048,
	    2404,
	    2748,
	    3072,
	    3364,
	    3617,
	    3822,
	    3972,
	    4065,
	    4095,
	    4065,
	    3972,
	    3822,
	    3617,
	    3364,
	    3072,
	    2748,
	    2404,
	    2048,
	    1692,
	    1348,
	    1024,
	    732,
	    479,
	    274,
	    124,
	    31,
	    0,
	    31,
	    124,
	    274,
	    479,
	    732,
	    1024,
	    1348,
	    1692
    };
	    
	unsigned short anRampData[] = {                 // predefined curve for AD5415/AD5405 output
		0x0000, 
		0x0071, 
		0x00E2, 
		0x0153, 
		0x01C4, 
		0x0235, 
		0x02A6, 
		0x0317, 
		0x0388, 
		0x03F9, 
		0x046A, 
		0x04DB, 
		0x054C, 
		0x05BD,
		0x062E, 
		0x069F, 
		0x0710, 
		0x0781, 
		0x07F2, 
		0x0863, 
		0x08D4, 
		0x0954, 
		0x09B6, 
		0x0A27, 
		0x0A98, 
		0x0B09, 
		0x0B7A, 
		0x0BEB, 
		0x0C5C,
		0x0CCD, 
		0x0D3E, 
		0x0DAF, 
		0x0E20, 
		0x0E91, 
		0x0F02,
		0x0F73
	};
	

	
	
    result = adi_int_Init( intmgr_storage,
                           sizeof(intmgr_storage),
                           &response_count,
                           &critical_reg);                                                                                
	if (result != ERR_NONE) {
	    // error
	}
	
	
#if defined (__ADSPBF537__)	
	ADI_PWR_COMMAND_PAIR PowerTable [] = {	// configuration table for bf537 power management	
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)ADI_PWR_PROC_BF537SKBC600 },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)ADI_PWR_PACKAGE_MBGA },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)ADI_PWR_VDDEXT_330 }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN },		
		{ ADI_PWR_CMD_END,			NULL	},
	};
#elif defined (__ADSPBF533__)	
	ADI_PWR_COMMAND_PAIR PowerTable [] = {	// configuration table for bf533 power management	
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)ADI_PWR_PROC_BF533SKBC600 },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)ADI_PWR_PACKAGE_MBGA },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)ADI_PWR_VDDEXT_330 }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN },		
		{ ADI_PWR_CMD_END,			NULL	},
	};
#elif defined (__ADSPBF561__)
	ADI_PWR_COMMAND_PAIR PowerTable [] = {	// configuration table for bf561 power management	
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)ADI_PWR_PROC_BF561SBB600 },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)ADI_PWR_PACKAGE_MBGA },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)ADI_PWR_VDDEXT_330 }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN },		
		{ ADI_PWR_CMD_END,			NULL	},
	};
#elif defined (__ADSPBF527__)
    ADI_PWR_COMMAND_PAIR PowerTable [] = {	// configuration table for bf527 power management	
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)ADI_PWR_PROC_BF527SBBC1600 },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)ADI_PWR_PACKAGE_MBGA },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)ADI_PWR_VDDEXT_330 }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN },		
		{ ADI_PWR_CMD_END,			NULL	},
	};
#elif defined (__ADSPBF548__)
    ADI_PWR_COMMAND_PAIR PowerTable [] = {	// configuration table for bf548 power management	
		{ ADI_PWR_CMD_SET_PROC_VARIANT, 	(void*)ADI_PWR_PROC_BF548SKBC1600 },
		{ ADI_PWR_CMD_SET_PACKAGE, 			(void*)ADI_PWR_PACKAGE_MBGA },
		{ ADI_PWR_CMD_SET_VDDEXT, 			(void*)ADI_PWR_VDDEXT_330 }, 
		{ ADI_PWR_CMD_SET_CLKIN, 			(void*)PROC_CLOCK_FREQ_IN },		
		{ ADI_PWR_CMD_END,			NULL	},
	};
#else
#error "processor not yet supported!"
#endif

    // initialize power management
	result = adi_pwr_Init(PowerTable);
	if (result != ERR_NONE) {
		/// error handling
	}
    
	//set optimal frequencies
	result = adi_pwr_SetFreq(CORE_CLK, SYSTEM_CLK, ADI_PWR_DF_ON);
	if (result != ERR_NONE) {
	    /// error handling
	}
	
	
	adi_pwr_GetFreq(&cclk, &sclk, &vco);
	
	// enable asynchronous memory banks
	ami_setup();
	
	printf("Core Clk:\t%u\nSystem Clk:\t%u\n\n",cclk, sclk);
	
	// to clear CS and generating reference voltage
	gpio_becomeOutput(_PH10);
	gpio_clear(_PH10);
	
#ifdef USE_AD5405_PARALLEL

    aAD5405Configuration.nSamplingRate = AD5405_DEFAULT_SAMPLING_RATE;
    aAD5405Configuration.nMaxElements = AD5405_DEFAULT_RING_BUFFER_SIZE;
    aAD5405Configuration.nTimerNr = AD5405_DEFAULT_TIMER;
    aAD5405Configuration.nSystemClk = sclk;
    aAD5405Configuration.nWriteBufferAddress = AD5405_BASE_ADDRESS;
    aAD5405Configuration.nChannelMode = GLOBAL_AD5405_CHANNEL_MODE;
    aAD5405Configuration.nBufferEmptyMode = AD5405_BUFFER_EMPTY_WRITE_LAST_SAMPLE;
    aAD5405Configuration.fnCallback = AD5405UserCallback;
    aAD5405Configuration.nMaxFillStatus = 90;
    aAD5405Configuration.nMinFillStatus = 50;

    erResultAD5405 = AD5405Setup(&aAD5405Configuration);
           	                        
    if(erResultAD5405 == ERR_NONE) {

	    printf("AD5405 initialized\nno error occured\n\n");
    }else {
        printf("Error during initialization of AD5405: %d\n",erResultAD5405);
        result = erResultAD5405;
    }
#endif

    if(result == ERR_NONE) {
        
        printf("Going to loop\n\n");

#ifdef USE_AD5405_PARALLEL
        AD5405Start();
#endif

    }
    
    iSin = 0;
    iRamp = 0;
    g_bAD5405B1ready = true;
    g_bAD5405B2ready = true;    
    while(1) {

    
#ifdef USE_AD5405_PARALLEL
        
//        _GET_CYCLE_COUNT(g_cur[0]);
        while(!g_bAD5405B1ready && !g_bAD5405B2ready) {
            ;
        }
//        _GET_CYCLE_COUNT(g_cur[1]);
        erResultAD5405WriteB1 = ERR_NONE;
        erResultAD5405WriteB2 = ERR_NONE;
        
        iSinMax = sizeof(anSinCurve) / sizeof(unsigned short);
        iRampMax = sizeof(anRampData) / sizeof(unsigned short);
        
        while(g_bAD5405B1ready || g_bAD5405B2ready) {
            
            if(g_bAD5405B1ready) {
                erResultAD5405WriteB1 = AD5405Write(anSinCurve[iSin], 0);
                if(erResultAD5405WriteB1 != ERR_AD5405_RINGBUFFER_FULL) {
                    iSin++;
                    
                    if(iSin >= iSinMax) {
                        iSin = 0;
                    }
                }
                
            }
            
            if(g_bAD5405B2ready) {
                erResultAD5405WriteB2 = AD5405Write(anRampData[iRamp], 1);
                if(erResultAD5405WriteB2 != ERR_AD5405_RINGBUFFER_FULL) {
                    iRamp++;
                    
                    if(iRamp >= iRampMax) {
                        iRamp = 0;
                    
                    }
                }
                
            }
            
            if(erResultAD5405WriteB1 == ERR_AD5405_RINGBUFFER_FULL && erResultAD5405WriteB2 == ERR_AD5405_RINGBUFFER_FULL) {
                g_bAD5405B1ready = false;
				g_bAD5405B2ready = false;
            }
        }
        
//        _GET_CYCLE_COUNT(g_cur[2]);
//        g_cur[4] = g_cur[1] - g_cur[0];
//        g_cur[5] = g_cur[2] - g_cur[1];
//        g_cur[6] = g_cur[0] - g_cur[3];
//        g_cur[3] = g_cur[0];
#endif
        
    }
}


void AD5405UserCallback(T_AD5405_CALLBACK_ARG *pa_pBufferFillStatus) {
    
    if((pa_pBufferFillStatus->nChannel) == 0) {
        
        if(!pa_pBufferFillStatus->nElementsLoaded) {
        }
        
        if(!(pa_pBufferFillStatus->bBufferFull)) {
        
            g_bAD5405B1ready = true;
        } else {
            g_bAD5405B1ready = false;
        }
        
    }else if((pa_pBufferFillStatus->nChannel) == 1) {
        
        if(!(pa_pBufferFillStatus->bBufferFull)) {
        
            g_bAD5405B2ready = true;
        } else {
            g_bAD5405B2ready = false;
        }
    }
}


void ami_setup (void) {             // enables asynchronous memory banks for bf527
	// *pEBIU_AMBCTL0 = 0x7bb07bb0;
	// *pEBIU_AMBCTL1 = 0x7bb07bb0;
	
	*pEBIU_AMBCTL0 = 0xffc2ffc2;
	*pEBIU_AMBCTL1 = 0xffc2ffc2;
	
	*pEBIU_AMGCTL = 0x01f8;			// give dma priority over cor for external accesses
}

