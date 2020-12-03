/**
    @file PPI656In.c
    @ingroup video
    @brief 
    
    
    
    BLT_DISCLAIMER(TBD)
    @author Thomas Maier
    @version 1.0
    @date 30.06.2009
    
    @startcond Changelog
    
    @endcond
**/

#include "PPI656In.h"
#include "../../PPI.h"

extern T_AV_STANDARD_DESCR g_tPALstd;               ///< declared in analogVideo.c
extern T_AV_STANDARD_DESCR g_tNTSCstd;              ///< declared in analogVideo.c
extern unsigned int g_nPPIcount;                    ///< declared in PPIglobal.c
extern T_PPI_SPEC g_aPPIspec[];                     ///< declared in PPIglobal.c

//extern unsigned long g_nPPI656InErrorCount;
//extern unsigned long g_nPPI656InErrorCountDeep;

int PPI656IframesAvailable(T_PPI656I_HANDLE pa_tHndl);
void PPI656IstartGrabbing(T_PPI656I_HANDLE pa_tHndl);
float PPI656IgetFramerate(T_PPI656I_HANDLE pa_tHndl);
void PPI656IstopGrabbing(T_PPI656I_HANDLE pa_tHndl) ;
T_ERROR_CODE PPI656IfreeFrame(T_PPI656I_HANDLE pa_tHndl);
void PPI656IstartPPI(T_PPI656I_HANDLE pa_tHndl) ;
void PPI656IstopPPI(T_PPI656I_HANDLE pa_tHndl);
ifrm_t *PPI656IDequeueFrame(T_PPI656I_HANDLE pa_tHndl);


//unsigned short *g_anPPI656IDMAdesc[PPI656I_MAX_DEVICES];
static T_PPI656I_HANDLE g_atPPI656Itable[PPI656I_MAX_DEVICES];
static unsigned char g_cPPI656InoDevices = 0;              ///< number of initialized devices

#pragma section ("L1_data_a")
unsigned short g_anPPI656IDMAdesc[PPI656I_MAX_DEVICES][2 * PPI656I_DMA_DESC_SIZE * 10];

// interrupt handler for PPI interrupt
#pragma section ("L1_code")
ADI_INT_HANDLER_RESULT PPI656IintHandler(void *pa_pClientArg) {
    bool bRestartPPI = true;
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_pClientArg;
     
    // check flag   
    if((*g_aPPIspec[pInst->cPPInr].pDMAirqStat) & DMA_DONE) {    
                
        // check if a full frame was transfered
        pInst->nTransmittedLines += pInst->tAVstandard.nHeight / 2;
        if(pInst->nTransmittedLines == pInst->tAVstandard.nHeight )
        {           
            pInst->nTransmittedLines = 0;
                                    
            // save time stamp in frame structure
            if(pInst->fnGetTimeStamp) {
                pInst->atFrames[pInst->cActiveFrameIn].ulTS = ((T_PPI656I_GET_TIME_STAMP)(pInst->fnGetTimeStamp))(pa_pClientArg);
            }
            else {
                pInst->atFrames[pInst->cActiveFrameIn].ulTS = pInst->nTimeStampCount++;
            }
            
            if(pInst->cFramesInBuffer < pInst->cNumberOfFrames) {
                pInst->cFramesInBuffer++;
            }
            
            // set next active frame
            switch(pInst->tBufferMode) {
                case(PPI656I_BUFFER_OVERWRITE): {
                    if(pInst->cFramesInBuffer == pInst->cNumberOfFrames) {
                        // do not update the active frame
                        break;
                    }
                    else {  // some frames are free
                        ++pInst->cActiveFrameIn;
                    }
                    break;
                }
                case(PPI656I_BUFFER_NOT_OVERWRITE): {
                    // there's no free frame
                    if(pInst->cFramesInBuffer == (pInst->cNumberOfFrames)) {
                        
                        // stop the frame transfer on PPI
                        PPI656IstopPPI((T_PPI656I_HANDLE)pa_pClientArg);  
                        ++pInst->cActiveFrameIn;               
                        bRestartPPI = false;            
                    }
                    else {
                        ++pInst->cActiveFrameIn;
                    }
                    break;
                }
                default:
                    return ADI_INT_RESULT_NOT_PROCESSED;
            }
            
            // check the range of ActiveFrameIn
            if(pInst->cActiveFrameIn >= pInst->cNumberOfFrames) {
                pInst->cActiveFrameIn = 0;
            }
             
            PPI656IstopPPI((T_PPI656I_HANDLE)pa_pClientArg);
            *(g_aPPIspec[pInst->cPPInr].pDMAnextDescrPtr) = (void *)((unsigned long)&g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * pInst->cActiveFrameIn]);            
            
            if(bRestartPPI) {
                PPI656IstartPPI((T_PPI656I_HANDLE)pa_pClientArg); 
            }
            
            // execute callback function
            if(pInst->fnTransferCallback) {
            	pInst->fnTransferCallback(pa_pClientArg);
            }            
        }

        // clear IRQ request        
        *( g_aPPIspec[pInst->cPPInr].pDMAirqStat ) |= DMA_DONE;
        int nDummy = *(g_aPPIspec[pInst->cPPInr].pPPIstatus);
                
		return ADI_INT_RESULT_PROCESSED;
	} 
	else {	    
		return ADI_INT_RESULT_NOT_PROCESSED;
	}       
}



#pragma section ("L1_code")
static ADI_INT_HANDLER_RESULT PPI656IerrorHandler(void *pa_pClientArg) {
  
	T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_pClientArg;
	int nDummy;
		
	// clear status register
	nDummy = *(g_aPPIspec[pInst->cPPInr].pPPIstatus);
	
	if (nDummy & 0xFB00) {
		
        PPI656IstopPPI((T_PPI656I_HANDLE)pa_pClientArg);
        
        *(g_aPPIspec[pInst->cPPInr].pPPIstatus) |= nDummy;
        //g_nPPI656InErrorCount++;
	    
	    if (pInst->fnErrorCallback) {										// Check if a Callback function was hooked
		    pInst->fnErrorCallback(pa_pClientArg);						// If so, execute it!
		}
		
		
        PPI656IstartPPI((T_PPI656I_HANDLE)pa_pClientArg);
	        
		return ADI_INT_RESULT_PROCESSED;			// Processed the interrupt, so notify ADI's ISR routine.
	} 
	else {    	    
		return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't processed, so notify ADI's ISR routine.
	}
}


#pragma section ("L1_code")

ADI_INT_HANDLER_RESULT PPI6565IDMAErrorInterruptHandler(void *pa_pClientArg) {
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_pClientArg;
            
    if (pInst) {	
    	if (*(g_aPPIspec[pInst->cPPInr].pDMAirqStat) & DMA_ERR) {
            PPI656IstopPPI((T_PPI656I_HANDLE)pa_pClientArg);
            
    	    //g_nPPI656InErrorCount++;
    	    
    		*(g_aPPIspec[pInst->cPPInr].pDMAirqStat) |= DMA_ERR;
    		
            PPI656IstartPPI((T_PPI656I_HANDLE)pa_pClientArg);
                        
    		return ADI_INT_RESULT_PROCESSED;
    	}
    }
        
    return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
}



/**
    @brief save handle in tbale 
**/
char PPI656IsetHandle(T_PPI656I_HANDLE pa_tHndl) {
    unsigned char i;
    
    for(i = 0; i < PPI656I_MAX_DEVICES ; i++) {
        if(g_atPPI656Itable[i] == 0) {
            g_atPPI656Itable[i] = pa_tHndl;
            return i;
        }
    }
    return -1;
} 

/**
    @brief remove handle from table
**/
void PPI656IremoveHandle(T_PPI656I_HANDLE pa_tHndl) {
    unsigned char i;
    
    for(i = 0; i < PPI656I_MAX_DEVICES ; i++) {
        if(g_atPPI656Itable[i] == pa_tHndl) {
            g_atPPI656Itable[i] = 0;
        }
    }
}



/**
 *	@private
 *	@brief		Initialize the array of frame structures
 *
 *	@param		pa_pClientArg	Handle to a PPI656In 
 *	
 *	@return		NONE
 *	
 **/
void PPI656IinitializeFrameStructure(T_PPI656I_HANDLE pa_pClientArg) {
    
    unsigned short nIndex;
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_pClientArg;
    
    for(nIndex = 0; nIndex < (pInst->cNumberOfFrames - 1); nIndex++) {
        pInst->atFrames[nIndex].ulTS = 0;
        pInst->atFrames[nIndex].pucFramePtr = pInst->pcFramebuffer[nIndex];
        pInst->atFrames[nIndex].nxt = &pInst->atFrames[nIndex + 1];
    }
    
    pInst->atFrames[nIndex].ulTS = 0;
    pInst->atFrames[nIndex].pucFramePtr = pInst->pcFramebuffer[nIndex];
    pInst->atFrames[nIndex].nxt = NULL;
}



T_PPI656I_HANDLE PPI656Iopen(T_PPI656I_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_tError, void **pa_tFGfunctions) {
    
    *pa_tError = ERR_NONE;
    T_PPI656I_INST *pInst = (T_PPI656I_INST*) malloc(sizeof(T_PPI656I_INST));
    
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
                *pa_tError = ERR_PPI656I_UNKNWON_AV_STD;
                free(pInst);
                return 0;
            }
        }
        
        //check for max devices
        char cIndex = PPI656IsetHandle((T_PPI656I_HANDLE)pInst);
        
        if(cIndex < 0) {
            *pa_tError = ERR_PPI656I_MAX_DEVICES;
        } else {
             pInst->cDeviceNr = cIndex;
        }
        
        //check for PPI
        if (g_aPPIspec[pa_ptConfig->cPPInr].bInUse) {
                    *pa_tError = ERR_PPI656I_PPI_NOT_AVAILABLE;
                }
        
        if(*pa_tError == ERR_NONE) {
        
            if(pa_ptConfig->cNumberOfFrames == 0) {
                pa_ptConfig->cNumberOfFrames = 1;
            }
        
            if(pa_ptConfig->cNumberOfFrames > FG_MAX_NOF_BUFFERED_FRAMES) {
                pa_ptConfig->cNumberOfFrames = FG_MAX_NOF_BUFFERED_FRAMES;
            }
            
            //malloc space for frame buffers
            pInst->pcFramebuffer[0] = (unsigned char*)malloc(pa_ptConfig->cNumberOfFrames * 
                                                             pInst->tAVstandard.nWidth * 
                                                             pInst->tAVstandard.nHeight * 
                                                             sizeof(unsigned short));
        
            if(pInst->pcFramebuffer[0]) {
                unsigned short i;
                //fill framebuffer pointer array
                for (i = 1; i < pa_ptConfig->cNumberOfFrames; i++) {
                    pInst->pcFramebuffer[i] = pInst->pcFramebuffer[0] + 
                                              i * pInst->tAVstandard.nWidth * 2 * pInst->tAVstandard.nHeight;  
                }
                
                g_cPPI656InoDevices ++;
                //setup ppi & dma
                
                pInst->cNumberOfFrames = pa_ptConfig->cNumberOfFrames;
                pInst->cPPInr = pa_ptConfig->cPPInr;
                pInst->nPPIIVG= pa_ptConfig->nPPIIVG;
                pInst->nErrorIVG = pa_ptConfig->nErrorIVG;
                pInst->tAVformat = pa_ptConfig->tAVformat;
                pInst->tBufferMode = (pa_ptConfig->cNumberOfFrames == 1) ? PPI656I_BUFFER_OVERWRITE : pa_ptConfig->tBufferMode;
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
                pa_tFGfunctions[FG_DEVICE_RESET] 			= (void *)PPI656IresetDevice;
                pa_tFGfunctions[FG_FREE_FRAME] 				= (void *)PPI656IfreeFrame;
                pa_tFGfunctions[FG_FRAME_AVAILABLE] 		= (void *)PPI656IframesAvailable;
                pa_tFGfunctions[FG_DEQUEUE_FRAME] 			= (void *)PPI656IDequeueFrame;
                pa_tFGfunctions[FG_GET_FRAMERATE] 			= (void *)PPI656IgetFramerate;
                pa_tFGfunctions[FG_GET_VIDEO_DEVICE_INFO] 	= (void *)PPI656IgetDeviceInfo;
                pa_tFGfunctions[FG_START_GRABBING] 			= (void *)PPI656IstartGrabbing;
                pa_tFGfunctions[FG_STOP_GRABBING] 			= (void *)PPI656IstopGrabbing;
                pa_tFGfunctions[FG_CONFIG_DEVICE] 			= (void *)PPI656IconfigDevice;
                
                if( ppi_platformInit(pInst->cPPInr, 0x00c0) ) {
                         
                                       
                #ifdef __ADSPBF561__ 
                    // 32 Bit transfer
                    // initialize the PPI interface
                    *(g_aPPIspec[pInst->cPPInr].pDMAxcount)    = pInst->tAVstandard.nWidth / 2;
                    *(g_aPPIspec[pInst->cPPInr].pDMAxmodify)   = 2 * pInst->tAVstandard.nPixelStride;
                    
                    *(g_aPPIspec[pInst->cPPInr].pDMAycount)    = pInst->tAVstandard.nHeight / 2;
                    *(g_aPPIspec[pInst->cPPInr].pDMAymodify)   = 2 * pInst->tAVstandard.nWidth + 4;
                
                    *(g_aPPIspec[pInst->cPPInr].pDMAconfig)    = 0x74BA;
                    *(g_aPPIspec[pInst->cPPInr].pPPIcontrol)   = 0x01C0;
                    
                #else
                    // 16 Bit transfer
                    // initialize the PPI interface
                    *(g_aPPIspec[pInst->cPPInr].pDMAxcount)    = pInst->tAVstandard.nWidth;
                    *(g_aPPIspec[pInst->cPPInr].pDMAxmodify)   = pInst->tAVstandard.nPixelStride;
                    
                    *(g_aPPIspec[pInst->cPPInr].pDMAycount)    = pInst->tAVstandard.nHeight / 2;
                    *(g_aPPIspec[pInst->cPPInr].pDMAymodify)   = 2 * pInst->tAVstandard.nWidth + 2;
                    
                    *(g_aPPIspec[pInst->cPPInr].pDMAconfig)    = 0x7496;
                    *(g_aPPIspec[pInst->cPPInr].pPPIcontrol)   = 0x00C0;
                #endif    
                
                    // set ppi registers
                    *(g_aPPIspec[pInst->cPPInr].pPPIdelay) = 0;
                    *(g_aPPIspec[pInst->cPPInr].pPPIframe) = pInst->tAVstandard.nHeight;
                    
                    
                    //fill descriptor array for each plane
                    unsigned short i;                
                    
                    for(i=0; i<pInst->cNumberOfFrames; i++) {
                        
                        unsigned long nNextStartAddr = (unsigned long)pInst->pcFramebuffer[i];// + 2 * pInst->tAVstandard.nWidth;
                        
                        // NEXT_DESC_PTR LSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i] = (unsigned long)(&g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 4]);                        
                        // NEXT_DESC_PTR HSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 1] = (unsigned long)(&g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 4]) >> 16;
                        
                        // START_ADDR LSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 2] = nNextStartAddr;
                        // START_ADDR HSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 3] = nNextStartAddr >> 16;                        
                        
                        nNextStartAddr +=  2 * pInst->tAVstandard.nWidth;
                        // NEXT_DESC_PTR LSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 4] = (unsigned long)(&g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 8]);
                        // NEXT_DESC_PTR LSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 5] = (unsigned long)(&g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 8]) >> 16;
                        // START_ADDR LSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 6] = nNextStartAddr;
                        // START_ADDR LSBs
                        g_anPPI656IDMAdesc[pInst->cDeviceNr][8 * i + 7] = nNextStartAddr >> 16;                        
                        
                    }

                    //the last one points to the first
                    g_anPPI656IDMAdesc[pInst->cDeviceNr][(pInst->cNumberOfFrames - 1) * 8 + 4] = (unsigned long)g_anPPI656IDMAdesc[pInst->cDeviceNr];          	            
                    //the last one points to the first
                    g_anPPI656IDMAdesc[pInst->cDeviceNr][(pInst->cNumberOfFrames - 1) * 8 + 5] = (unsigned long)g_anPPI656IDMAdesc[pInst->cDeviceNr] >> 16;          	            
            	    // Initialize the array of frames structure
            	    
            	    PPI656IinitializeFrameStructure((T_PPI656I_HANDLE)pInst);        
            	            

            	    // initialize frame buffer
            	    //memset(pInst->pcFramebuffer[0],0,pInst->tAVstandard.nWidth*pInst->tAVstandard.nHeight*2*pInst->cNumberOfFrames);

            	    
                    *(g_aPPIspec[pInst->cPPInr].pDMAnextDescrPtr) = (void *)((unsigned long)&g_anPPI656IDMAdesc[pInst->cDeviceNr][0]);
                    
                    //hook interrupts
                 	if(!pa_ptConfig->nPPIIVG) {
    				    adi_int_SICGetIVG(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId, &pInst->nPPIIVG);
    				} else {
    				    adi_int_SICSetIVG(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId, pa_ptConfig->nPPIIVG);
    				    pInst->nPPIIVG = pa_ptConfig->nPPIIVG; //use custom ivg
    				}
					
    				// hook the interrupt
    				if(adi_int_CECHook(pInst->nPPIIVG, PPI656IintHandler, (void *)pInst, true) == ADI_INT_RESULT_SUCCESS) {
    					adi_int_SICWakeup(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId, TRUE);
    					adi_int_SICEnable(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId);
    				} else {
    					*pa_tError = ERR_PPI656I_HOOK_INTERRUPT;
    				}
    				
                 	if(!pa_ptConfig->nErrorIVG) {
    				    adi_int_SICGetIVG(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId, &pInst->nErrorIVG);
    				} else {
    				    adi_int_SICSetIVG(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId, pa_ptConfig->nErrorIVG);
    				    pInst->nErrorIVG = pa_ptConfig->nErrorIVG; //use custom ivg
    				}    				
    				//hook error interupt 							
    				if(adi_int_CECHook(pInst->nErrorIVG, PPI656IerrorHandler, (void *)pInst, true) == ADI_INT_RESULT_SUCCESS) {
    					adi_int_SICWakeup(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId, TRUE);
    					adi_int_SICEnable(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId);
    				} else {
    					*pa_tError = ERR_PPI656I_HOOK_INTERRUPT;
    				}	
    				
                    if(!pa_ptConfig->nDMAErrorIVG) {
                        adi_int_SICGetIVG(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId, &pInst->nDMAErrorIVG);
                    } else {
                        adi_int_SICSetIVG(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId, pa_ptConfig->nDMAErrorIVG);
                        pInst->nDMAErrorIVG = pa_ptConfig->nDMAErrorIVG; //use custom ivg
                    }       
                    // hook DMA error interrupt
                    if(adi_int_CECHook(pInst->nDMAErrorIVG, PPI6565IDMAErrorInterruptHandler, (void *)pInst, true) == ADI_INT_RESULT_SUCCESS) {
                        adi_int_SICWakeup(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId, TRUE);
                        adi_int_SICEnable(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId);
                    } else {
                        *pa_tError = ERR_PPI656I_HOOK_INTERRUPT;
                    }
				       				
                } else {
                    *pa_tError = ERR_PPI6565I_PLATFORM_INIT; 
                    free(pInst->pcFramebuffer[0]);
                    free(pInst);
                }
            } else {
                *pa_tError = ERR_PPI656I_OUT_OF_MEM;
                free(pInst);
            }
        
        } 
    } else {
        
        *pa_tError = ERR_PPI656I_OUT_OF_MEM;      
    }
    
    if(*pa_tError == ERR_NONE) {
        return ((T_PPI656I_HANDLE)pInst);
    }
    else {
        return 0;   
    }
}



T_ERROR_CODE PPI656Iclose(T_PPI656I_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    T_PPI656I_INST *pInst = (T_PPI656I_INST*) pa_tHndl;
    
    // stop the ppi
    PPI656IstopPPI(pa_tHndl);
    
    // unhook the interrupts
    adi_int_CECUnhook(pInst->nPPIIVG, PPI656IintHandler, (void *)pInst);
    adi_int_CECUnhook(pInst->nErrorIVG, PPI656IerrorHandler, (void *)pInst);
    adi_int_CECUnhook(pInst->nDMAErrorIVG, PPI6565IDMAErrorInterruptHandler, (void *)pInst);
    
    // free allocated memory
    free(pInst->pcFramebuffer[0]);
    free(g_anPPI656IDMAdesc[pInst->cDeviceNr]);
    free(pInst);
    return erResult;
    
}


void PPI656IstartPPI(T_PPI656I_HANDLE pa_tHndl) {
    
    T_PPI656I_INST *pInst = (T_PPI656I_INST*) pa_tHndl;
    
    // first check if there are free frames
    if((pInst->tBufferMode == PPI656I_BUFFER_OVERWRITE) || (pInst->cFramesInBuffer < (pInst->cNumberOfFrames - 1))) {
//        *(g_aPPIspec[pInst->cPPInr].pDMAstartaddr) = (void *)((g_anPPI656IDMAdesc[0][3] << 16) | g_anPPI656IDMAdesc[0][2]);
        
        *(g_aPPIspec[pInst->cPPInr].pDMAconfig) |= 0x0001;
        *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) |= 0x0001; 
    }
}




void PPI656IstopPPI(T_PPI656I_HANDLE pa_tHndl) {
    
    T_PPI656I_INST *pInst = (T_PPI656I_INST*) pa_tHndl;
    
    *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) &= ~0x0001; 
    *(g_aPPIspec[pInst->cPPInr].pDMAconfig) &= ~0x0001;
}



int PPI656IframesAvailable(T_PPI656I_HANDLE pa_tHndl) {
    
	T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
	
    return pInst->cFramesInBuffer;
}



/**
 *    @public
 *    @brief        Initiates data transfers over the PPI.
 *
 *    @param        pa_tHndl    Handle to a Camera Resource
 *
 *    
 **/
void PPI656IstartGrabbing(T_PPI656I_HANDLE pa_tHndl) {
	
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
	
    if(pa_tHndl) {
    	pInst->bGrabbingStarted = true;
        PPI656IstartPPI(pa_tHndl);
    } 
}



T_ERROR_CODE PPI656IfreeFrame(T_PPI656I_HANDLE pa_tHndl) {
    
    T_ERROR_CODE erResult = ERR_NONE;
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
    
    // a PPI656IDequeueFrame must be perform first
	if (pInst->bDequeueInProgress) {
		if (++pInst->cActiveFrameOut == pInst->cNumberOfFrames) {
		    pInst->cActiveFrameOut = 0;
		}
		
		pInst->cFramesInBuffer --;
		
		pInst->bDequeueInProgress = false;
		
		// check if the PPI 
		if((pInst->bGrabbingStarted) && !(*(g_aPPIspec[pInst->cPPInr].pPPIcontrol) & 0x1)) {
		    //*(g_aPPIspec[pInst->cPPInr].pDMAnextDescrPtr) = (void *)((unsigned long)&g_anPPI656IDMAdesc[pInst->cDeviceNr][0]);
			PPI656IstartPPI(pa_tHndl);
		}
		
		erResult =  ERR_NONE;
	} else {
	    erResult =  ERR_GENERIC;
	}
    
    return erResult;
}



ifrm_t *PPI656IDequeueFrame(T_PPI656I_HANDLE pa_tHndl) {
    
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
    
    // check if dequeue is always in progress
    if((!pInst->bDequeueInProgress) && (pInst->cFramesInBuffer)) {
        pInst->bDequeueInProgress = true;
        
        return(&pInst->atFrames[pInst->cActiveFrameOut]);
    } 
    else {
        return(NULL);   
    }
}



/**
 *	@public
 *	@brief		Gets the current FrameRate of the Device
 *
 *	@param		pa_tHndl	Handle to a PPI656In instance
 *	
 *	@return		Returns the FrameRate
 *	
 **/
float PPI656IgetFramerate(T_PPI656I_HANDLE pa_tHndl) {
	T_PPI656I_INST* pInst = (T_PPI656I_INST *)pa_tHndl;
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
}


/**
 *    @public
 *    @brief        Stops grbbing frames over the PPI.
 *
 *    @param        pa_tHndl    Handle to a PPI656In instance
 *
 *    
 **/
void PPI656IstopGrabbing(T_PPI656I_HANDLE pa_tHndl) {
	
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
	
    if(pa_tHndl) {
    	pInst->bGrabbingStarted = false;
    	    	
        PPI656IstopPPI(pa_tHndl); 
    } 
}    



void PPI656Isetup() {

}
void PPI656Icleanup() {

}



/**
 *	@public
 *	@brief		Writes back a T_FG_VIDEO_DEVICE_INFO stucture to a provided buffer. @see T_FG_VIDEO_DEVICE_INFO 
 *
 *	@param		pa_tHndl		Handle to a PPI656In Resource
 *	@param		pa_tInfoBlock	Provide the address of a T_FG_VIDEO_DEVICE_INFO structure.
 *	
 *	@return		ERR_NONE on sucess, ERR_GENERIC on timeout or failure.
 *	
 **/
void PPI656IgetDeviceInfo(T_PPI656I_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock) {
    
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
    
    pa_tInfoBlock->tFramerate = (T_FG_FRAMERATE)PPI656IgetFramerate(pa_tHndl);
    
    pa_tInfoBlock->nXres = pInst->tAVstandard.nWidth;
    pa_tInfoBlock->nYres = pInst->tAVstandard.nHeight;
    pa_tInfoBlock->tColorMode = FG_YUV422;
}


/**
 *	@public
 *	@brief		Resets the specified PPI656 device
 *
 *	@param		pa_tHndl	Handle to a PPI656In Resource
 *	
 *	@return		ERR_NONE on sucess
 **/
T_ERROR_CODE PPI656IresetDevice(T_PPI656I_HANDLE pa_tHndl) {
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;
    
    pInst->cFramesInBuffer = 0;      ///< number of valid frames in buffer
    pInst->cActiveFrameIn = 0;    
    pInst->cActiveFrameOut = 0;
    pInst->bDequeueInProgress = false;            ///< flag is set if dequeue is in progress
   	pInst->nTransmittedLines = 0;;         ///< Number of transmitted lines for the active frame
   	pInst->nTimeStampCount = 0;           ///< Counter for the time stamp
   	pInst->bGrabbingStarted = false;;                   ///< is TRUE if a grabbing process was started, otherwise FALSE

    return ERR_NONE;
}


/**
 *	@public
 *	@brief		Configures the specified PPI656In resource
 *				
 *
 *	@param		pa_tHndl	Handle to a PPI656In Resource
 *	@param		pa_tCmd		PPI656In Commands @see frameGrabber.h
 *	@param		pa_tArg		Argument for command @see T_FG_ARG (can only use the pa_tArg.nNofFramesToHold element).
 *	
 *	@return		ERR_NONE on sucess.
 *	
 **/
T_ERROR_CODE PPI656IconfigDevice(T_PPI656I_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg) {
    T_PPI656I_INST *pInst = (T_PPI656I_INST *)pa_tHndl;

	switch(pa_tCmd){
	    case FG_CMD_CLOSE:{
	        if(pInst->fnHwDevCloseDev && pInst->hHwDevHndl){
	            pInst->fnHwDevCloseDev(pInst->hHwDevHndl);
	        }
	        PPI656IstopGrabbing(pa_tHndl);
	        PPI656IstopPPI(pa_tHndl);
	        PPI656Iclose(pa_tHndl);
	        break;
	    }
	    case FG_CMD_GET_DEVICE_NAME:{
	        sprintf(pa_tArg->cDeviceName, "PPI656In%d", pInst->cPPInr);
	        break;
	    }
	    default:{
	        return ERR_GENERIC;
	    }
	}
	
    return ERR_NONE;
}
