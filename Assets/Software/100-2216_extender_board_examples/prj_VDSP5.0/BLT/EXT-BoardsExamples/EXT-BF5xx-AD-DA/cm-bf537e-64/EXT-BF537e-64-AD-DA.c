/*****************************************************************************
 * EXT-BF537e-64-AD-DA.c
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <drivers/adi_dev.h>
#include <cycle_count_bf.h>
 
#include "Environment.h"
#include "EXT-BF537e-64-AD-DA.h"
 
static u8 intmgr_storage[(ADI_INT_SECONDARY_MEMORY * 8)];

//unsigned long long g_cur[7];
bool g_bAD7266ready;
bool g_bAD5415ready;
bool g_bAD5405B1ready;
bool g_bAD5405B2ready;

unsigned short *g_pAD7266ProcessBuffer;
unsigned short *g_pAD5415ProcessBuffer;


void main( void ) {
	
    u32 response_count;
	u32 critical_reg;
	unsigned long cclk; 							// frequencies
	unsigned long sclk;
	unsigned long vco;
//	unsigned short nBurstSizeAD5415;
#ifdef USE_PROCESS_DATA
	unsigned short aProcessBuffer[AD7266_DEFAULT_BURST_SIZE * 2 * AD7266_DEFAULT_STRIDE];
#endif
	unsigned short i, iMax;
	unsigned short iSin, iRamp;
	unsigned short iSinMax, iRampMax;
	int result;
	
	T_AD7266_CONFIG aAD7266Configuration;
	T_AD5415_CONFIG aAD5415Configuration;
	T_AD5405_CONFIG aAD5405Configuration;
	
	T_ERROR_CODE erResultAD7266;
	T_ERROR_CODE erResultAD5415;
	T_ERROR_CODE erResultAD5405;
	T_ERROR_CODE erResultAD5405WriteB1;
	T_ERROR_CODE erResultAD5405WriteB2;
	T_ERROR_CODE erPrepBuf;
    
    T_AD5415_CHANNEL_MODE nAD5415ChannelMode;
    
	
	iSin = 0;
	iRamp = 0;
		
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
	
	g_bAD7266ready = false;
	g_bAD5415ready = false;
	
//	nBurstSizeAD5415 = sizeof(anSinCurve)/sizeof(unsigned short);
	
	
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
	
	// enable asynchronous memory banks
	ami_setup ();
    
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
	
	printf("Core Clk:\t%u\nSystem Clk:\t%u\n\n",cclk, sclk);
	
#ifdef USE_AD7266_SERIAL
	
	aAD7266Configuration.nChannel = AD7266_DEFAULT_CHANNEL;
	aAD7266Configuration.nSamplingRate = AD7266_DEFAULT_SAMPLING_RATE;
	aAD7266Configuration.nMaxElements = AD7266_DEFAULT_BURST_SIZE;
	aAD7266Configuration.nStride = AD7266_DEFAULT_STRIDE;
	aAD7266Configuration.nOffset = AD7266_DEFAULT_OFFSET;
	aAD7266Configuration.fnCallback = AD7266UserCallback;
	aAD7266Configuration.adr0 = AD7266_A0;
	aAD7266Configuration.adr1 = AD7266_A1;
	aAD7266Configuration.adr2 = AD7266_A2;
	aAD7266Configuration.nSclk = sclk;
	aAD7266Configuration.nBufferMode = AD7266_PINGPONG_BUFFER;
	aAD7266Configuration.nSPORTperIVG = 0;
	aAD7266Configuration.nDMAerrIVG = 0;
	aAD7266Configuration.nSPORTerrIVG = 0;
	aAD7266Configuration.nSPORT = 0xffff;
	aAD7266Configuration.nFlags = 0;
	
    erResultAD7266 = AD7266Setup(&aAD7266Configuration);
    
    if(erResultAD7266 == ERR_NONE) {
        printf("AD7266 initialized\nno error occured\n\n");
    }else {
        /// error handling
        printf("Error during initialization of AD7266: %d\n", erResultAD7266);
        result = erResultAD7266;
    }
#endif
	
	
#ifdef USE_AD5415_SERIAL
    unsigned long nSamplingRate;
    
#ifdef USE_AD7266_SERIAL
    nSamplingRate = (unsigned long)(aAD7266Configuration.fRealSamplingRate);
#else
    nSamplingRate = (unsigned long)AD5415_DEFAULT_SAMPLING_RATE;
#endif

    aAD5415Configuration.nChannelMode = GLOBAL_AD5415_CHANNEL_MODE;
    aAD5415Configuration.nSamplingRate = nSamplingRate;
	aAD5415Configuration.nMaxElements = AD5415_DEFAULT_BURST_SIZE;
	aAD5415Configuration.nStride = AD5415_DEFAULT_STRIDE;
	aAD5415Configuration.nOffset = AD5415_DEFAULT_OFFSET;
	aAD5415Configuration.fnCallback = AD5415UserCallback;
	aAD5415Configuration.nSclk = sclk;
	aAD5415Configuration.nBufferMode = AD5415_PINGPONG_BUFFER;
	aAD5415Configuration.nSPORTperIVG = 0;
	aAD5415Configuration.nDMAerrIVG = 0;
	aAD5415Configuration.nSPORTerrIVG = 0;
	aAD5415Configuration.nSPORT = 0xffff;

    erResultAD5415 = AD5415Setup(&aAD5415Configuration);
           	                        
    if(erResultAD5415 == ERR_NONE) {
        printf("AD5415 initialized\nno error occured\n\n");
    }else {
        printf("Error during initialization of AD5415: %d\n",erResultAD5415);
        result = erResultAD5415;
    }
#endif
#ifdef USE_AD5405_PARALLEL
    unsigned long nSamplingRate;
    
#ifdef USE_AD7266_SERIAL
    nSamplingRate = (unsigned long)(aAD7266Configuration.fRealSamplingRate);
#else
    nSamplingRate = (unsigned long)AD5405_DEFAULT_SAMPLING_RATE;
#endif

    aAD5405Configuration.nSamplingRate = nSamplingRate;
    aAD5405Configuration.nMaxElements = AD5405_DEFAULT_RING_BUFFER_SIZE;
    aAD5405Configuration.nTimerNr = AD5405_DEFAULT_TIMER;
    aAD5405Configuration.nSystemClk = sclk;
    aAD5405Configuration.nWriteBufferAddress = AD5405_BASE_ADDRESS;
    aAD5405Configuration.nChannelMode = GLOBAL_AD5405_CHANNEL_MODE;
    aAD5405Configuration.nBufferEmptyMode = AD5405_BUFFER_EMPTY_WRITE_LAST_SAMPLE;
    aAD5405Configuration.fnCallback = AD5405UserCallback;
    aAD5405Configuration.nMaxFillStatus = 90;
    aAD5405Configuration.nMinFillStatus = 20;

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
        
#ifdef USE_AD7266_SERIAL
        AD7266Start();
#endif

#ifdef USE_AD5415_SERIAL
        AD5415Start();
#endif

#ifdef USE_AD5405_PARALLEL
        AD5405Start();
#endif

    }
    
    iSin = 0;
    iRamp = 0;
    g_bAD5405B1ready = true;
    g_bAD5405B2ready = true;
    
    while(1) {
    
#ifdef USE_AD5415_SERIAL
#ifndef USE_SIGNAL_SAMPLES

//        _GET_CYCLE_COUNT(g_cur[0]);
        
        while(!g_bAD7266ready || !g_bAD5415ready) {
            ;
        }
        
        
//        _GET_CYCLE_COUNT(g_cur[1]);
        
        
#ifdef USE_PROCESS_DATA

//        memcpy((void *)&aProcessBuffer, (void *)g_pAD7266ProcessBuffer, (aAD7266Configuration.nMaxElements * sizeof(unsigned short) * aAD5415Configuration.nChannelMode * AD7266_DEFAULT_STRIDE) + aAD7266Configuration.nOffset);

        iMax = aAD7266Configuration.nMaxElements * aAD7266Configuration.nStride * aAD5415Configuration.nChannelMode;
        
        for(i=aAD7266Configuration.nOffset; i<iMax; i+= aAD7266Configuration.nStride) {
            aProcessBuffer[i] = *(g_pAD7266ProcessBuffer + i);
        }
        
        ProcessData(   GLOBAL_GAIN,
                       (unsigned short *)&aProcessBuffer,
                       aAD5415Configuration.nMaxElements,
                       aAD5415Configuration.nStride,
                       aAD5415Configuration.nOffset,
                       aAD5415Configuration.nChannelMode,
                       PROCESS_GAIN);
//        memcpy((void *)g_pAD5415ProcessBuffer, (void *)&aProcessBuffer, (aAD5415Configuration.nMaxElements * sizeof(unsigned short) * aAD5415Configuration.nChannelMode * aAD7266Configuration.nStride) + aAD7266Configuration.nOffset);
        
        iMax = aAD7266Configuration.nMaxElements * aAD7266Configuration.nStride * aAD5415Configuration.nChannelMode;
        
        for(i=aAD7266Configuration.nOffset; i<iMax; i+= aAD7266Configuration.nStride) {
            *(g_pAD5415ProcessBuffer + i) = aProcessBuffer[i];
        }
        
#else
//        memcpy((void *)g_pAD5415ProcessBuffer, (void *)g_pAD7266ProcessBuffer, ((aAD5415Configuration.nMaxElements * aAD5415Configuration.nChannelMode * aAD5415Configuration.nStride) + aAD5415Configuration.nOffset) * sizeof(unsigned short));
        
        
        iMax = aAD7266Configuration.nMaxElements * aAD7266Configuration.nStride * aAD5415Configuration.nChannelMode;
        
        for(i=aAD7266Configuration.nOffset; i<iMax; i+= aAD7266Configuration.nStride) {
            *(g_pAD5415ProcessBuffer + i) = *(g_pAD7266ProcessBuffer + i);
        }
        
        
#endif


        erPrepBuf = PrepareAD5415Buffer(    g_pAD5415ProcessBuffer,
                                            aAD5415Configuration.nMaxElements,
                                            aAD5415Configuration.nStride,
                                            aAD5415Configuration.nOffset,
                                            aAD5415Configuration.nChannelMode);
        
        g_bAD7266ready = false;
        g_bAD5415ready = false;
        
//        _GET_CYCLE_COUNT(g_cur[2]);
        
//        g_cur[4] = g_cur[1] - g_cur[0];
//        g_cur[5] = g_cur[2] - g_cur[1];
//        g_cur[6] = g_cur[0] - g_cur[3];
//        g_cur[3] = g_cur[0];
        
#else
        while(!g_bAD5415ready) {
            ;
        }
        
        iMax = (aAD5415Configuration.nMaxElements);
        iSinMax = sizeof(anSinCurve) / sizeof(unsigned short);
        iRampMax = sizeof(anRampData) / sizeof(unsigned short);
        
        
        for(i=0; i<iMax; i++) {
            *((unsigned short *)g_pAD5415ProcessBuffer + i) = (unsigned short)anSinCurve[iSin];
            iSin++;
            if(iSin>=iSinMax) {
                iSin = 0;
            }
        }
        
        if(aAD5415Configuration.nChannelMode == AD5415_DUAL_CHANNEL) {
            iMax = (aAD5415Configuration.nMaxElements * aAD5415Configuration.nChannelMode);
            for(i=aAD5415Configuration.nMaxElements; i<iMax; i++) {
                *((unsigned short *)g_pAD5415ProcessBuffer + i) = (unsigned short)anRampData[iRamp];
                iRamp++;
                if(iRamp>=iRampMax) {
                    iRamp = 0;
                }
            }
        }
        
        erPrepBuf = PrepareAD5415Buffer(    g_pAD5415ProcessBuffer,
                                            aAD5415Configuration.nMaxElements,
                                            aAD5415Configuration.nStride,
                                            aAD5415Configuration.nOffset,
                                            aAD5415Configuration.nChannelMode);
                                            
        g_bAD5415ready = false;
        
#endif
#endif

#ifdef USE_AD5405_PARALLEL
        
#ifndef USE_SIGNAL_SAMPLES
        i=0;
//        _GET_CYCLE_COUNT(g_cur[0]);
        erResultAD5405WriteB1 = ERR_NONE;
        erResultAD5405WriteB2 = ERR_NONE;
        
        while(!g_bAD7266ready) {
            ;
        }
        
//        _GET_CYCLE_COUNT(g_cur[1]);
        
        if(aAD5405Configuration.nChannelMode == AD5405_SINGLE_CHANNEL) {
            while(i<aAD7266Configuration.nMaxElements) {
                
                do {
                    erResultAD5405WriteB1 = AD5405Write(*((unsigned short *)g_pAD7266ProcessBuffer + (i * aAD7266Configuration.nStride) + aAD7266Configuration.nOffset), 0);
                }while(erResultAD5405WriteB1 == ERR_AD5405_RINGBUFFER_FULL);
                i++;
            }
        }else {
            while(i<aAD7266Configuration.nMaxElements) {
                
                do {
                    erResultAD5405WriteB1 = AD5405Write(*((unsigned short *)g_pAD7266ProcessBuffer + (i * aAD7266Configuration.nStride) + aAD7266Configuration.nOffset), 0);
                }while(erResultAD5405WriteB1 == ERR_AD5405_RINGBUFFER_FULL);
                
                do {
                    erResultAD5405WriteB2 = AD5405Write(*((unsigned short *)g_pAD7266ProcessBuffer + (i * aAD7266Configuration.nStride) + aAD7266Configuration.nOffset + aAD7266Configuration.nMaxElements), 1);
                }while(erResultAD5405WriteB2 == ERR_AD5405_RINGBUFFER_FULL);
                
                i++;
            }
        }
        
        g_bAD7266ready = false;
        
//        _GET_CYCLE_COUNT(g_cur[2]);
//        g_cur[4] = g_cur[1] - g_cur[0];
//        g_cur[5] = g_cur[2] - g_cur[1];
//        g_cur[6] = g_cur[0] - g_cur[3];
//        g_cur[3] = g_cur[0];

#else
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
#endif
        
    }
}


void AD5415UserCallback(void *pa_pBufferAccessable) {
    
    g_pAD5415ProcessBuffer = pa_pBufferAccessable;
    
    g_bAD5415ready = true;
}


void AD7266UserCallback(void *pa_pBufferAccessable) {
    
    g_pAD7266ProcessBuffer = pa_pBufferAccessable;
    g_bAD7266ready = true;
}


void AD5405UserCallback(T_AD5405_CALLBACK_ARG *pa_pBufferFillStatus) {
    
    if((pa_pBufferFillStatus->nChannel) == 0) {
        
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


T_ERROR_CODE ProcessData(   float pa_fGain,
                            unsigned short *pa_pBufferBase,
                            unsigned short pa_nBurstSize,
                            unsigned short pa_nStride,
                            unsigned short pa_nOffset,
                            T_AD5415_CHANNEL_MODE pa_nChannelMode,
                            T_PROCESS_MODE pa_nProcessMode) {
    
    unsigned short i;
    unsigned short *pBufferBaseCh2;
    unsigned short nMaxVal = pa_nBurstSize;
    unsigned short nGain;
    T_ERROR_CODE erResult = ERR_NONE;
    
    nGain = (unsigned short)(0x0800 * pa_fGain);
    
    pBufferBaseCh2 = (unsigned short *)pa_pBufferBase + (pa_nBurstSize * pa_nStride) + pa_nOffset;
    
    switch(pa_nProcessMode) {
        
        case PROCESS_GAIN: {
            
            if(pa_fGain == 1.0) {
                 break;
            } else {
                
                if(pa_nChannelMode == AD5415_SINGLE_CHANNEL) {
                    for(i=pa_nOffset; i<nMaxVal; i+=pa_nStride) {
                        *((unsigned short *)pa_pBufferBase + i) = (unsigned short)(((unsigned long)(*((unsigned short *)pa_pBufferBase + i)) * nGain)>>11);
                    }
                } else {
                    for(i=pa_nOffset; i<nMaxVal; i+=pa_nStride) {
                        *((unsigned short *)pa_pBufferBase + i) = (unsigned short)(((unsigned long)(*((unsigned short *)pa_pBufferBase + i)) * nGain)>>11);
                        *((unsigned short *)pBufferBaseCh2 + i) = (unsigned short)(((unsigned long)(*((unsigned short *)pBufferBaseCh2 + i)) * nGain)>>11);
                    }
                }
            }
            break;
        }
        
        case PROCESS_CHANNEL_ADDITION: {
            
            for(i=pa_nOffset; i<nMaxVal; i+=pa_nStride) {
                *((unsigned short *)pa_pBufferBase + i) = (unsigned short)(*((unsigned short *)pa_pBufferBase + i) + *((unsigned short *)pBufferBaseCh2 + i));
            }
            break;
        }
        
        case PROCESS_CHANNEL_SUBTRACTION_1_2: {
            
            for(i=pa_nOffset; i<nMaxVal; i+=pa_nStride) {
                *((unsigned short *)pa_pBufferBase + i) = (unsigned short)((((unsigned long)(*((unsigned short *)pa_pBufferBase + i)) * nGain)>>11) - *((unsigned short *)pBufferBaseCh2 + i));
            }
            break;
        }
        
        case PROCESS_CHANNEL_SUBTRACTION_2_1: {
            
            for(i=pa_nOffset; i<nMaxVal; i+=pa_nStride) {
                *((unsigned short *)pa_pBufferBase + i) = (unsigned short)(*((unsigned short *)pBufferBaseCh2 + i) - (unsigned short)(((unsigned long)(*((unsigned short *)pa_pBufferBase + i)) * nGain)>>11));
            }
            break;
        }
        
        default: {
            
            break;
        }
    }
    return erResult;
}


void ami_setup (void) {
	*pEBIU_AMBCTL0 = 0xffc2ffc2;
	*pEBIU_AMBCTL1 = 0xffc2ffc2;
	*pEBIU_AMGCTL = 0x01f8;
}

