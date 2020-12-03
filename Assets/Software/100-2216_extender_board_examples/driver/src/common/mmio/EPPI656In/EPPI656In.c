/**
    @file EPPI656In.c
    @ingroup video
    @brief 
    
    
    
    BLT_DISCLAIMER(TBD)
    @author Alexander Froemel
    @version 1.0
    @date 20.04.2010
    
    @startcond Changelog
    
    @endcond
**/

#include <cplbtab.h>
#include <stdio.h>
#include <conio.h>
#include <cycle_count_bf.h>
#include "EPPI656In.h"
#include "../../EPPI.h"

extern T_AV_STANDARD_DESCR g_tPALstd;               ///< declared in analogVideo.c
extern T_AV_STANDARD_DESCR g_tNTSCstd;              ///< declared in analogVideo.c
extern unsigned int g_nEPPIcount;                    ///< declared in EPPIglobal.c
extern T_EPPI_SPEC g_aEPPIspec[];                     ///< declared in EPPIglobal.c

static T_EPPI656I_HANDLE g_atEPPI656Itable[EPPI656I_MAX_DEVICES];
static unsigned char g_cEPPI656InoDevices = 0;              ///< number of initialized devices

#pragma section ("L1_data_a")
unsigned short g_anEPPI656IDMAdesc[EPPI656I_MAX_DEVICES][EPPI656I_DMA_DESC_PER_PLANE * EPPI656I_DMA_DESC_SIZE * VDM_MAX_NOF_PLANES];

unsigned long g_nEPPI656InFieldCount = 0;
unsigned long g_nEPPI656InErrorCount = 0;
unsigned long g_nEPPI656InDmaErrorCount = 0;
unsigned long g_nEPPI656InErrorLtoCount = 0;

// interrupt handler for EPPI interrupt
#pragma section ("L1_code")
ADI_INT_HANDLER_RESULT EPPI656IintHandler(void *pa_pClientArg) {
    bool bRestartEPPI = true;
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_pClientArg;
    if(pInst) {
        // check flag   
        if((*g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat) & DMA_DONE) {
            
            // clear IRQ request        
            *( g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat ) |= DMA_DONE;
            int nDummy = *(g_aEPPIspec[pInst->cEPPInr].pEPPIstatus);
                
            // check if a full frame was transfered
            pInst->nTransmittedLines += pInst->tAVstandard.nHeight >> 1;
            g_nEPPI656InFieldCount++;
            if(pInst->nTransmittedLines >= pInst->tAVstandard.nHeight )
            {
                if( (__cplb_ctrl & CPLB_ENABLE_DCACHE ) || (__cplb_ctrl & CPLB_ENABLE_DCACHE2)) {
                    dcache_invalidate_both();                                   // invalidate cache because data is written by DMA
                }
            
                pInst->nTransmittedLines = 0;
                                    
                // save time stamp in frame structure
                if(pInst->fnGetTimeStamp) {
                    pInst->atFrames[pInst->cActiveFrameIn].ulTS = ((T_EPPI656I_GET_TIME_STAMP)(pInst->fnGetTimeStamp))(pa_pClientArg);
                }
                else {
                    pInst->atFrames[pInst->cActiveFrameIn].ulTS = pInst->nTimeStampCount++;
                }
                
                // set next active frame
                if(pInst->cFramesInBuffer < pInst->cNumberOfFrames) {
                    pInst->cFramesInBuffer++;
                    pInst->cActiveFrameIn++;
                    // check the range of ActiveFrameIn
                    if(pInst->cActiveFrameIn >= pInst->cNumberOfFrames) {
                        pInst->cActiveFrameIn = 0;
                    }
                }
                else {
                       pInst->cActiveFrameIn = 0;
                }
                
                switch(pInst->tBufferMode) {
                    case(EPPI656I_BUFFER_OVERWRITE): {
                        // nothing to do, just keep receiving data
                        break;
                    }
                    case(EPPI656I_BUFFER_NOT_OVERWRITE): { 
                        if(pInst->cFramesInBuffer == (pInst->cNumberOfFrames)) {
                            // there's no free frame
                            // stop the frame transfer on EPPI
                            bRestartEPPI = false;
                        }
                        
                        EPPI656IstopEPPI((T_EPPI656I_HANDLE)pa_pClientArg);
                        *(g_aEPPIspec[pInst->cEPPInr].pDMA0nextDescrPtr) = (void *)((unsigned long)&g_anEPPI656IDMAdesc[pInst->cDeviceNr][EPPI656I_DMA_DESC_PER_PLANE * EPPI656I_DMA_DESC_SIZE * pInst->cActiveFrameIn]);            
                        
                        if(bRestartEPPI) {
                            EPPI656IstartEPPI((T_EPPI656I_HANDLE)pa_pClientArg);
                        }
                        
                        break;
                    }
                    default:
                        return ADI_INT_RESULT_NOT_PROCESSED;
                }
                           
                // execute callback function
                if(pInst->fnTransferCallback) {
                	pInst->fnTransferCallback(pa_pClientArg);
                }            
            }
               
            return ADI_INT_RESULT_PROCESSED;
        } else {  
            return ADI_INT_RESULT_NOT_PROCESSED;
        }
    } 
    else {
        return ADI_INT_RESULT_NOT_PROCESSED;
    }
}



#pragma section ("L1_code")
static ADI_INT_HANDLER_RESULT EPPI656IerrorHandler(void *pa_pClientArg) {
	T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_pClientArg;
	
	if(pInst) {
    	int nDummy = 0;
		
    	// clear status register
    	nDummy = *(g_aEPPIspec[pInst->cEPPInr].pEPPIstatus);
	
    	if (nDummy & 0x41ff) {
    	    if(nDummy & 0x0004) {
                g_nEPPI656InErrorLtoCount++;
    	    } else {
		
                EPPI656IstopEPPI((T_EPPI656I_HANDLE)pa_pClientArg);
        	    *(g_aEPPIspec[pInst->cEPPInr].pEPPIstatus) |= 0x01ff;
        
                g_nEPPI656InErrorCount++;
	    
        	    if (pInst->fnErrorCallback) {										// Check if a Callback function was hooked
        		    pInst->fnErrorCallback(pa_pClientArg);						// If so, execute it!
        		}
                EPPI656IstartEPPI((T_EPPI656I_HANDLE)pa_pClientArg);
    	    }
		
		
	        
    		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
    	} 
	}
	return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
}


#pragma section ("L1_code")

ADI_INT_HANDLER_RESULT EPPI6565IDMAErrorInterruptHandler(void *pa_pClientArg) {
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_pClientArg;
            
    if (pInst) {	
    	if (*(g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat) & DMA_ERR) {
            EPPI656IstopEPPI((T_EPPI656I_HANDLE)pa_pClientArg);
            
    	    g_nEPPI656InDmaErrorCount++;
    	    
    		*(g_aEPPIspec[pInst->cEPPInr].pDMA0irqStat) |= DMA_ERR;
    		
            EPPI656IstartEPPI((T_EPPI656I_HANDLE)pa_pClientArg);
                      
    		return ADI_INT_RESULT_PROCESSED;
    	}
    }
        
    return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
}


void EPPI656Isetup(void) {
    // clear the handle table
    int i=0;
    for(i = 0; i < EPPI656I_MAX_DEVICES ; i++) {
        g_atEPPI656Itable[i] = 0;
    }
}


void EPPI656Icleanup(void) {
    // currently nothing to do
}



/**
    @brief save handle in tbale 
**/
char EPPI656IsetHandle(T_EPPI656I_HANDLE pa_tHndl) {
    unsigned char i;
    
    for(i = 0; i < EPPI656I_MAX_DEVICES ; i++) {
        if(g_atEPPI656Itable[i] == 0) {
            g_atEPPI656Itable[i] = pa_tHndl;
            return i;
        }
    }
    return -1;
} 

/**
    @brief remove handle from table
**/
void EPPI656IremoveHandle(T_EPPI656I_HANDLE pa_tHndl) {
    unsigned char i;
    
    for(i = 0; i < EPPI656I_MAX_DEVICES ; i++) {
        if(g_atEPPI656Itable[i] == pa_tHndl) {
            g_atEPPI656Itable[i] = 0;
        }
    }
}



/**
 *	@private
 *	@brief		Initialize the array of frame structures
 *
 *	@param		pa_pClientArg	Handle to a EPPI656In 
 *	
 *	@return		NONE
 *	
 **/
void EPPI656IinitializeFrameStructure(T_EPPI656I_HANDLE pa_pClientArg) {
    
    unsigned short nIndex;
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_pClientArg;
    
    for(nIndex = 0; nIndex < (pInst->cNumberOfFrames - 1); nIndex++) {
        pInst->atFrames[nIndex].ulTS = 0;
        pInst->atFrames[nIndex].pucFramePtr = pInst->pcFramebuffer[nIndex];
        pInst->atFrames[nIndex].nxt = &pInst->atFrames[nIndex + 1];
    }
    
    pInst->atFrames[nIndex].ulTS = 0;
    pInst->atFrames[nIndex].pucFramePtr = pInst->pcFramebuffer[nIndex];
    pInst->atFrames[nIndex].nxt = NULL;
}



T_EPPI656I_HANDLE EPPI656Iopen(T_EPPI656I_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_tError, void **pa_tFGfunctions) {
    
    *pa_tError = ERR_NONE;
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST*) malloc(sizeof(T_EPPI656I_INST));
    
    if(pInst) {
        
        //get video standard fill standard descriptor
        switch(pa_ptConfig->tAVformat) {
            case AV_PAL_BGH:
            case AV_PAL: {
                pInst->tAVstandard = g_tPALstd;
                break;
            }
            case AV_NTSC: {
                pInst->tAVstandard = g_tNTSCstd;
                break;
            }
            
            default: {
                *pa_tError = ERR_EPPI656I_UNKNWON_AV_STD;
                free(pInst);
                return 0;
            }
        }
        
        //check for max devices
        char cIndex = EPPI656IsetHandle((T_EPPI656I_HANDLE)pInst);
        
        if(cIndex < 0) {
            *pa_tError = ERR_EPPI656I_MAX_DEVICES;
        } else {
             pInst->cDeviceNr = cIndex;
        }
        
        //check for EPPI
        if (g_aEPPIspec[pa_ptConfig->cEPPInr].bInUse) {
                    *pa_tError = ERR_EPPI656I_EPPI_NOT_AVAILABLE;
                }
        
        if(*pa_tError == ERR_NONE) {
        
            if(pa_ptConfig->cNumberOfFrames == 0) {
                pa_ptConfig->cNumberOfFrames = 1;
            }
        
            if(pa_ptConfig->cNumberOfFrames > FG_MAX_NOF_BUFFERED_FRAMES) {
                pa_ptConfig->cNumberOfFrames = FG_MAX_NOF_BUFFERED_FRAMES;
            }
            
            // allocate space for frame buffers
            pInst->pcFramebuffer[0] = (unsigned char*)calloc(pa_ptConfig->cNumberOfFrames * 
                                                             pInst->tAVstandard.nWidth * 
                                                             pInst->tAVstandard.nHeight, 
                                                             pInst->tAVstandard.nPixelStride);
        
            if(pInst->pcFramebuffer[0]) {
                unsigned short i;
                //fill framebuffer pointer array
                for (i = 1; i < pa_ptConfig->cNumberOfFrames; i++) {
                    pInst->pcFramebuffer[i] = pInst->pcFramebuffer[0] + 
                                              i * pInst->tAVstandard.nWidth * pInst->tAVstandard.nPixelStride * pInst->tAVstandard.nHeight;  
                }
                
                g_cEPPI656InoDevices ++;
                //setup EPPI & dma
                
                pInst->cNumberOfFrames = pa_ptConfig->cNumberOfFrames;
                pInst->cEPPInr = pa_ptConfig->cEPPInr;
                pInst->nEPPIIVG= pa_ptConfig->nEPPIIVG;
                pInst->nErrorIVG = pa_ptConfig->nErrorIVG;
                pInst->tAVformat = pa_ptConfig->tAVformat;
                pInst->tBufferMode = (pa_ptConfig->cNumberOfFrames == 1) ? EPPI656I_BUFFER_OVERWRITE : pa_ptConfig->tBufferMode;
                pInst->nTimeStampCount = 0;
                pInst->cActiveFrameIn = 0;
                pInst->cActiveFrameOut = 0;
                pInst->nTransmittedLines = 0;
                pInst->bDequeueInProgress = false;
                pInst->fnErrorCallback = pa_ptConfig->fnErrorCallback;
                pInst->fnTransferCallback = pa_ptConfig->fnTransferCallback;
                pInst->fnGetTimeStamp = pa_ptConfig->fnGetTimeStamp;
                pInst->cFramesInBuffer = 0;
                pInst->bGrabbingStarted = false;
                pInst->fnHwDevCloseDev = pa_ptConfig->fnHwDevCloseDev;
                pInst->hHwDevHndl = pa_ptConfig->hHwDevHndl;
                
                //fill framegrabberfunctions
                pa_tFGfunctions[FG_DEVICE_RESET] 			= (void *)EPPI656IresetDevice;
                pa_tFGfunctions[FG_FREE_FRAME] 				= (void *)EPPI656IfreeFrame;
                pa_tFGfunctions[FG_FRAME_AVAILABLE] 		= (void *)EPPI656IframesAvailable;
                pa_tFGfunctions[FG_DEQUEUE_FRAME] 			= (void *)EPPI656IDequeueFrame;
                pa_tFGfunctions[FG_GET_FRAMERATE] 			= (void *)EPPI656IgetFramerate;
                pa_tFGfunctions[FG_GET_VIDEO_DEVICE_INFO] 	= (void *)EPPI656IgetDeviceInfo;
                pa_tFGfunctions[FG_START_GRABBING] 			= (void *)EPPI656IstartGrabbing;
                pa_tFGfunctions[FG_STOP_GRABBING] 			= (void *)EPPI656IstopGrabbing;
                pa_tFGfunctions[FG_CONFIG_DEVICE] 			= (void *)EPPI656IconfigDevice;
                
                /*
                switch(pInst->tBufferMode) {
                    case : {
                        */
                    // set dma registers, 32 Bit transfer
                    *(g_aEPPIspec[pInst->cEPPInr].pDMA0xcount)    = pInst->tAVstandard.nActiveVideoLineSize >> 2;
                    *(g_aEPPIspec[pInst->cEPPInr].pDMA0xmodify)   = 4;
                
                    *(g_aEPPIspec[pInst->cEPPInr].pDMA0ycount)    = pInst->tAVstandard.nHeight >> 1;
                    *(g_aEPPIspec[pInst->cEPPInr].pDMA0ymodify)   = pInst->tAVstandard.nActiveVideoLineSize + 4;
                
                    *(g_aEPPIspec[pInst->cEPPInr].pDMA0config)    = 0x74BA;
                    
                   // break;
                   // }
                
               // }
                
                // set eppi registers
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIhdelay)    = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIvdelay)    = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIclkdiv)    = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs1w_hbl)  = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs1p_avpl) = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs2w_lvb)  = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIfs2p_lavf) = 0;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIclip)      = 0xff00ff00;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol)   = 0x00101040;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIframe)     = pInst->tAVstandard.nTotalLines;
                *(g_aEPPIspec[pInst->cEPPInr].pEPPIline)      = pInst->tAVstandard.nLineStride + 8;
                
                // call the eppi platform init with the control register
                if( eppi_platformInit(pInst->cEPPInr, *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol)) ) {
                    
                    //fill descriptor array for each plane
                    unsigned short i;                
                    
                    for(i=0; i<pInst->cNumberOfFrames; i++) {
                        
                        unsigned long nNextStartAddr = (unsigned long)pInst->pcFramebuffer[i];// + 2 * pInst->tAVstandard.nWidth;
                        
                        // NEXT_DESC_PTR LSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i] = (unsigned long)(&g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 4]) & 0xffff;
                        // NEXT_DESC_PTR HSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 1] = ((unsigned long)(&g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 4]) >> 16) & 0xffff;
                        
                        // START_ADDR LSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 2] = nNextStartAddr & 0xffff;
                        // START_ADDR HSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 3] = (nNextStartAddr >> 16) & 0xffff;
                        
                        nNextStartAddr +=  2 * pInst->tAVstandard.nWidth;
                        // NEXT_DESC_PTR LSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 4] = (unsigned long)(&g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 8]) & 0xffff;
                        // NEXT_DESC_PTR LSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 5] = ((unsigned long)(&g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 8]) >> 16) & 0xffff;
                        // START_ADDR LSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 6] = nNextStartAddr & 0xffff;
                        // START_ADDR LSBs
                        g_anEPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 7] = (nNextStartAddr >> 16) & 0xffff;
                        
                    }

                    //the last one points to the first
                    g_anEPPI656IDMAdesc[pInst->cDeviceNr][(pInst->cNumberOfFrames - 1) * 8 + 4] = (unsigned long)(&g_anEPPI656IDMAdesc[pInst->cDeviceNr][0]) & 0xffff;
                    //the last one points to the first
                    g_anEPPI656IDMAdesc[pInst->cDeviceNr][(pInst->cNumberOfFrames - 1) * 8 + 5] = ((unsigned long)(&g_anEPPI656IDMAdesc[pInst->cDeviceNr][0]) >> 16) & 0xffff;
            	    // Initialize the array of frames structure
            	    
            	    EPPI656IinitializeFrameStructure((T_EPPI656I_HANDLE)pInst);
            	            

            	    // initialize frame buffer
            	    //memset(pInst->pcFramebuffer[0],0,pInst->tAVstandard.nWidth*pInst->tAVstandard.nHeight*2*pInst->cNumberOfFrames);

            	    
                    *(g_aEPPIspec[pInst->cEPPInr].pDMA0nextDescrPtr) = (void *)((unsigned long)&g_anEPPI656IDMAdesc[pInst->cDeviceNr][0]);
                    
                    //hook interrupts
                 	if(!pa_ptConfig->nEPPIIVG) {
    				    adi_int_SICGetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0, &pInst->nEPPIIVG);
    				} else {
    				    adi_int_SICSetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0, pa_ptConfig->nEPPIIVG);
    				    pInst->nEPPIIVG = pa_ptConfig->nEPPIIVG; //use custom ivg
    				}
					
    				// hook the interrupt
    				if(adi_int_CECHook(pInst->nEPPIIVG, EPPI656IintHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
    					adi_int_SICWakeup(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0, FALSE);
    					adi_int_SICEnable(g_aEPPIspec[pa_ptConfig->cEPPInr].peripheralIntId0);
    				} else {
    					*pa_tError = ERR_EPPI656I_HOOK_INTERRUPT;
    				}
    				/*
                 	if(!pa_ptConfig->nErrorIVG) {
    				    adi_int_SICGetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0, &pInst->nErrorIVG);
    				} else {
    				    adi_int_SICSetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0, pa_ptConfig->nErrorIVG);
    				    pInst->nErrorIVG = pa_ptConfig->nErrorIVG; //use custom ivg
    				}    				
    				//hook error interupt 							
    				if(adi_int_CECHook(pInst->nErrorIVG, EPPI656IerrorHandler, (void *)pInst, true) == ADI_INT_RESULT_SUCCESS) {
    					adi_int_SICWakeup(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0, TRUE);
    					adi_int_SICEnable(g_aEPPIspec[pa_ptConfig->cEPPInr].perErrorIntId0);
    				} else {
    					*pa_tError = ERR_EPPI656I_HOOK_INTERRUPT;
    				}	
    				*/
                    if(!pa_ptConfig->nDMAErrorIVG) {
                        adi_int_SICGetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0, &pInst->nDMAErrorIVG);
                    } else {
                        adi_int_SICSetIVG(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0, pa_ptConfig->nDMAErrorIVG);
                        pInst->nDMAErrorIVG = pa_ptConfig->nDMAErrorIVG; //use custom ivg
                    }       
                    // hook DMA error interrupt
                    if(adi_int_CECHook(pInst->nDMAErrorIVG, EPPI6565IDMAErrorInterruptHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                        adi_int_SICWakeup(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0, FALSE);
                        adi_int_SICEnable(g_aEPPIspec[pa_ptConfig->cEPPInr].DMAerrorIntId0);
                    } else {
                        *pa_tError = ERR_EPPI656I_HOOK_INTERRUPT;
                    }
				       				
                } else {
                    *pa_tError = ERR_EPPI6565I_PLATFORM_INIT; 
                    free(pInst->pcFramebuffer[0]);
                    free(pInst);
                }
            } else {
                *pa_tError = ERR_EPPI656I_OUT_OF_MEM;
                free(pInst);
            }
        
        } 
    } else {
        *pa_tError = ERR_EPPI656I_OUT_OF_MEM;      
    }
    
    if(*pa_tError == ERR_NONE) {
        return ((T_EPPI656I_HANDLE)pInst);
    }
    else {
        return 0;   
    }
}



T_ERROR_CODE EPPI656Iclose(T_EPPI656I_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST*) pa_tHndl;
    if(pInst) {
    
        // stop the EPPI
        EPPI656IstopEPPI(pa_tHndl);
    
        // unhook the interrupts
        adi_int_SICDisable(g_aEPPIspec[pInst->cEPPInr].peripheralIntId0);
        adi_int_CECUnhook(pInst->nEPPIIVG, EPPI656IintHandler, (void *)pInst);
        
        adi_int_SICDisable(g_aEPPIspec[pInst->cEPPInr].perErrorIntId0);
        adi_int_CECUnhook(pInst->nErrorIVG, EPPI656IerrorHandler, (void *)pInst);
        
        adi_int_SICDisable(g_aEPPIspec[pInst->cEPPInr].DMAerrorIntId0);
        adi_int_CECUnhook(pInst->nDMAErrorIVG, EPPI6565IDMAErrorInterruptHandler, (void *)pInst);
        
        g_aEPPIspec[pInst->cEPPInr].bInUse = false;
    
        // free allocated memory
        free(pInst->pcFramebuffer[0]);
        free(pInst);
    }
    return erResult;
    
}


void EPPI656IstartEPPI(T_EPPI656I_HANDLE pa_tHndl) {
    
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST*) pa_tHndl;
    if(pInst) {
        *(g_aEPPIspec[pInst->cEPPInr].pDMA0config) |= 0x0001;
        *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol) |= 0x0001;
    }
}




void EPPI656IstopEPPI(T_EPPI656I_HANDLE pa_tHndl) {
    
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST*) pa_tHndl;
    
    if(pInst) {
        *(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol) &= ~0x0001; 
        *(g_aEPPIspec[pInst->cEPPInr].pDMA0config) &= ~0x0001;
    }
}



int EPPI656IframesAvailable(T_EPPI656I_HANDLE pa_tHndl) {
	T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
	if(pInst) {
        return pInst->cFramesInBuffer;
	} else {
	    return ERR_GENERIC;
	}
}



T_ERROR_CODE EPPI656IfreeFrame(T_EPPI656I_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    
    if(pInst) {
        // a EPPI656IDequeueFrame must be perform first
    	if (pInst->bDequeueInProgress) {
    		if (++pInst->cActiveFrameOut >= pInst->cNumberOfFrames) {
    		    pInst->cActiveFrameOut = 0;
    		}
		
    		pInst->cFramesInBuffer --;
		
    		pInst->bDequeueInProgress = false;
		
    		// check if the EPPI 
    		if((pInst->bGrabbingStarted) && !(*(g_aEPPIspec[pInst->cEPPInr].pEPPIcontrol) & 0x1)) {
    			EPPI656IstartEPPI(pa_tHndl);
    		}
		
    		erResult =  ERR_NONE;
    	} else {
    	    erResult =  ERR_GENERIC;
    	}
    }
    return erResult;
}



ifrm_t *EPPI656IDequeueFrame(T_EPPI656I_HANDLE pa_tHndl) {
    
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    
    if(pInst) {
        // check if dequeue is always in progress
        if((!pInst->bDequeueInProgress) && (pInst->cFramesInBuffer)) {
            pInst->bDequeueInProgress = true;
        
            return(&pInst->atFrames[pInst->cActiveFrameOut]);
        } 
        else {
            return(NULL);   
        }
    } else {
        return NULL;
    }
}



/**
 *	@public
 *	@brief		Gets the current FrameRate of the Device
 *
 *	@param		pa_tHndl	Handle to a EPPI656In instance
 *	
 *	@return		Returns the FrameRate
 *	
 **/
float EPPI656IgetFramerate(T_EPPI656I_HANDLE pa_tHndl) {
	T_EPPI656I_INST* pInst = (T_EPPI656I_INST *)pa_tHndl;
	if(pInst) {
    	switch(pInst->tAVformat) {
    	    case(AV_PAL):
    	    case(AV_PAL_BGH): {
    	        return(25.0);
    	    }
	    
    	    case(AV_NTSC): {
    	        return(30.0);
    	    }
	    
    	    default:
    	        return 0;   
    	}
	} else {
	    return 0;
	}
}


/**
 *    @public
 *    @brief        Initiates data transfers over the EPPI.
 *
 *    @param        pa_tHndl    Handle to a Camera Resource
 *
 *    
 **/
void EPPI656IstartGrabbing(T_EPPI656I_HANDLE pa_tHndl) {
	T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    if(pa_tHndl) {
    	pInst->bGrabbingStarted = true;
        EPPI656IstartEPPI(pa_tHndl);
    } 
}


/**
 *    @public
 *    @brief        Stops grbbing frames over the EPPI.
 *
 *    @param        pa_tHndl    Handle to a EPPI656In instance
 *
 *    
 **/
void EPPI656IstopGrabbing(T_EPPI656I_HANDLE pa_tHndl) {
	T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    if(pa_tHndl) {
    	pInst->bGrabbingStarted = false;
        EPPI656IstopEPPI(pa_tHndl); 
        pInst->cFramesInBuffer = 0;
        pInst->cActiveFrameIn = 0;
        pInst->cActiveFrameOut = pInst->cNumberOfFrames - 1;
        pInst->nTransmittedLines = 0;
        pInst->nTimeStampCount = 0;
        pInst->bDequeueInProgress = false;
    } 
}



/**
 *	@public
 *	@brief		Writes back a T_FG_VIDEO_DEVICE_INFO stucture to a provided buffer. @see T_FG_VIDEO_DEVICE_INFO 
 *
 *	@param		pa_tHndl		Handle to a EPPI656In Resource
 *	@param		pa_tInfoBlock	Provide the address of a T_FG_VIDEO_DEVICE_INFO structure.
 *	
 *	@return		ERR_NONE on sucess, ERR_GENERIC on timeout or failure.
 *	
 **/
void EPPI656IgetDeviceInfo(T_EPPI656I_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock) {
    
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    
    if(pInst) {
        pa_tInfoBlock->tFramerate = (T_FG_FRAMERATE)EPPI656IgetFramerate(pa_tHndl);
    
        pa_tInfoBlock->nXres = pInst->tAVstandard.nWidth;
        pa_tInfoBlock->nYres = pInst->tAVstandard.nHeight;
        pa_tInfoBlock->tColorMode = FG_YUV422;
    }
}


/**
 *	@public
 *	@brief		Resets the specified EPPI656 device
 *
 *	@param		pa_tHndl	Handle to a EPPI656In Resource
 *	
 *	@return		ERR_NONE on sucess
 **/
T_ERROR_CODE EPPI656IresetDevice(T_EPPI656I_HANDLE pa_tHndl) {
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    if(pInst) {
        pInst->cFramesInBuffer = 0;      ///< number of valid frames in buffer
        pInst->cActiveFrameIn = 0;    
        pInst->cActiveFrameOut = 0;
        pInst->bDequeueInProgress = false;            ///< flag is set if dequeue is in progress
       	pInst->nTransmittedLines = 0;;         ///< Number of transmitted lines for the active frame
       	pInst->nTimeStampCount = 0;           ///< Counter for the time stamp
       	pInst->bGrabbingStarted = false;;                   ///< is TRUE if a grabbing process was started, otherwise FALSE
    }
    return ERR_NONE;
}


/**
 *	@public
 *	@brief		Configures the specified EPPI656In resource
 *				
 *
 *	@param		pa_tHndl	Handle to a EPPI656In Resource
 *	@param		pa_tCmd		EPPI656In Commands @see frameGrabber.h
 *	@param		pa_tArg		Argument for command @see T_FG_ARG (can only use the pa_tArg.nNofFramesToHold element).
 *	
 *	@return		ERR_NONE on sucess.
 *	
 **/
T_ERROR_CODE EPPI656IconfigDevice(T_EPPI656I_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg) {
    T_EPPI656I_INST *pInst = (T_EPPI656I_INST *)pa_tHndl;
    if(pInst) {
    	switch(pa_tCmd){
    	    case FG_CMD_CLOSE:{
    	        if(pInst->fnHwDevCloseDev && pInst->hHwDevHndl){
    	            pInst->fnHwDevCloseDev(pInst->hHwDevHndl);
    	        }
    	        EPPI656IstopGrabbing(pa_tHndl);
    	        EPPI656IstopEPPI(pa_tHndl);
    	        EPPI656Iclose(pa_tHndl);
    	        break;
    	    }
    	    case FG_CMD_GET_DEVICE_NAME:{
    	        sprintf(pa_tArg->cDeviceName, "EPPI656In%d", pInst->cEPPInr);
    	        break;
    	    }
    	    default:{
    	        return ERR_GENERIC;
    	    }
    	}
    }
    return ERR_NONE;
}
