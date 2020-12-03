/**
    @file PPI656Out.c
    @ingroup video
    @brief 
    
    
    
    BLT_DISCLAIMER(TBD)
    @author Thomas Maier
    @version 1.0
    @date 30.06.2009
    
    @startcond Changelog
    
    @endcond
**/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <Environment.h>
#ifdef _USE_VDK_
    #include <vdk.h>
#endif
#include "PPI656Out.h"
#include "../../mdma/MDMAconfig.h"
#include "../../PPIconfig.h"
#include "../../PPI.h"
#include "../../misc/delay.h"
//#include "../../analog_video/analogVideo.h"



extern unsigned int g_nMDMAcount;                   ///< declared in mdma_global.c
extern T_MDMA_SPEC g_aMDMAspec[];                   ///< declared in mdma_global.c
extern T_AV_STANDARD_DESCR g_tPALstd;              ///< declared in analogVideo.c
extern T_AV_STANDARD_DESCR g_tNTSCstd;             ///< declared in analogVideo.c
extern unsigned int g_nPPIcount;
extern T_PPI_SPEC g_aPPIspec[];
//extern unsigned long g_nPPI656OutErrorCount;
extern T_MDMA_SPEC g_aMDMAspec[];


unsigned short g_nPPI656OutActivePlane = 0;

#ifdef __ADSPBF561__
    #ifdef __COREA__
        section ("L1_data_a") 
        unsigned short g_anPPI656ODMAdesc[PPI656O_DMA_DESC_SIZE  * PPI656O_MAX_DEVICES * VDM_MAX_NOF_PLANES];                                // descriptor size = 9, nof decsriptors = 2

    #elif defined (__COREB__)
        section ("L1_data_b") 
        unsigned short g_anPPI656ODMAdesc[PPI656O_DMA_DESC_SIZE  * PPI656O_MAX_DEVICES * VDM_MAX_NOF_PLANES];                                // descriptor size = 9, nof decsriptors = 2

    #else
        #error "You must specify a core!"
    #endif    
#else
    section ("L1_data") 
    unsigned short g_anPPI656ODMAdesc[PPI656O_DMA_DESC_SIZE  * PPI656O_MAX_DEVICES * VDM_MAX_NOF_PLANES];                                // descriptor size = 9, nof decsriptors = 2
#endif


static T_PPI656O_HANDLE g_atPPI656Otable[PPI656O_MAX_DEVICES];      ///< record handles
static unsigned char g_cPPI656OnoDevices = 0;                      ///< number of initialized devices

void AVinitITU656Matrix ( unsigned char *pa_pcBuffer, T_AV_STANDARD_DESCR *pa_pStd );
T_ERROR_CODE AV_CopyYUV422toITUR656Frame(T_AV_FRAME_BUFFER_DESC* pa_tDestBuffDesc, 
                                 unsigned short pa_nXOffset,
                                 unsigned short pa_nYOffset,
                                 char* pa_pcYUV422Start, 
                                 unsigned short pa_nWidth, 
                                 unsigned short pa_nHeight, 
                                 unsigned char pa_cMdmaChannel);

#pragma section ("L1_code")

                                 
ADI_INT_HANDLER_RESULT MDMAErrorInterruptHandler(void *pa_pClientArg) { 
    T_PPI656O_INST *pInst = (T_PPI656O_INST *)pa_pClientArg;
    
    if((*g_aMDMAspec[pInst->cMDMAChannel].pSourceIrqStat) & DMA_DONE) {
        *g_aMDMAspec[pInst->cMDMAChannel].pSourceIrqStat |= DMA_DONE;
        return ADI_INT_RESULT_PROCESSED;        
    }
    return ADI_INT_RESULT_NOT_PROCESSED;
       
}
                                 
                                 
#pragma section ("L1_code")

ADI_INT_HANDLER_RESULT PPI6565OinterruptHandler(void *pa_pClientArg) {
    
    T_PPI656O_INST *pInst = (T_PPI656O_INST *)pa_pClientArg;
    // check if peripheral interrupt occures
    if((*g_aPPIspec[pInst->cPPInr].pDMAirqStat) & DMA_DONE) {
        
        //plane switch request by VDM?
        if(pInst->bPlaneSwitchRequest) {
            // switch plane
            pInst->cActivePlane = pInst->cNextPlane;
            pInst->bPlaneSwitchRequest = false;
            
            // update free number of planes
            if(pInst->nFreePlanesCount < pInst->cNumberOfPlanes) {
                pInst->nFreePlanesCount++;     
            }
        } 
        else {
            if (pInst->bAutoUpdate){
                //use buffermode strategy
                pInst->cActivePlane++;
                if(pInst->cActivePlane >= pInst->cNumberOfPlanes) {
                    pInst->cActivePlane = 0;
                }
            
                g_anPPI656ODMAdesc[pInst->cDeviceNr *  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 2] = 
                        (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cActivePlane] & 0xffff);    // SAL: start address frame buffer 0
                    
                g_anPPI656ODMAdesc[pInst->cDeviceNr *  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 3] = 
                        (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pInst->cActivePlane] >> 16);        // SAH: start address frame buffer 0
                
                // update free number of planes                         
                if(pInst->nFreePlanesCount < pInst->cNumberOfPlanes) {
                    pInst->nFreePlanesCount++;     
                }                                              
            }
        }

        pInst->bVSYNC = true;
        #ifdef _USE_VDK_
            VDK_C_ISR_PostSemaphore(pInst->tFsyncSem);
        #endif
        
        g_nPPI656OutActivePlane = pInst->cActivePlane;
        
        // clear the interrupt
        *( g_aPPIspec[pInst->cPPInr].pDMAirqStat ) |= DMA_DONE;
              
        return ADI_INT_RESULT_PROCESSED;
    }     
    return ADI_INT_RESULT_NOT_PROCESSED;
}


#pragma section ("L1_code")
static ADI_INT_HANDLER_RESULT PPI656OerrorHandler(void *pa_pClientArg) {
  
	T_PPI656O_INST *pInst = (T_PPI656O_INST*)pa_pClientArg;
		
    // check if peripheral interrupt occures
    if((*g_aPPIspec[pInst->cPPInr].pPPIstatus) & 0x7000) {
        
        *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) &= ~0x0001;     
        *(g_aPPIspec[pInst->cPPInr].pDMAconfig) &= ~0x0001;

        *g_aPPIspec[pInst->cPPInr].pPPIstatus |= 0x7000;
        //g_nPPI656OutErrorCount++;
        
        if (pInst->fnErrorCallback) {                                        // Check if a Callback function was hooked
            pInst->fnErrorCallback(pa_pClientArg);                // If so, execute it!
        }

        *(g_aPPIspec[pInst->cPPInr].pDMAconfig) |= 0x0001;  
        *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) |= 0x0001;     
              
        return ADI_INT_RESULT_PROCESSED;            // Processed the interrupt, so notify ADI's ISR routine.
    } 
    
    else {
        return ADI_INT_RESULT_NOT_PROCESSED;    // Interrupt wasn't processed, so notify ADI's ISR routine.
    }
}



#pragma section ("L1_code")

ADI_INT_HANDLER_RESULT PPI6565ODMAErrorInterruptHandler(void *pa_pClientArg) {
        
    T_PPI656O_INST *pInst = (T_PPI656O_INST *)pa_pClientArg;
    
    if (pInst) {	
        
    	if (*(g_aPPIspec[pInst->cPPInr].pDMAirqStat) & DMA_ERR) {
            *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) &= ~0x0001;     
            *(g_aPPIspec[pInst->cPPInr].pDMAconfig) &= ~0x0001;
    	    
            //g_nPPI656OutErrorCount++;
            
    		*(g_aPPIspec[pInst->cPPInr].pDMAirqStat) |= DMA_ERR;
    		
            *(g_aPPIspec[pInst->cPPInr].pDMAconfig) |= 0x0001;  
            *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) |= 0x0001;     
                        
    		return ADI_INT_RESULT_PROCESSED;
    	}
    }
        
    return ADI_INT_RESULT_NOT_PROCESSED;	// Interrupt wasn't for us, notify ADI's ISR that we didn't process it!
}



/**
    @brief Open an ITU656 over PPI output device
**/
T_PPI656O_HANDLE PPI656Oopen(T_PPI656O_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_tError, void ***pfGraficFunctions) {
    T_PPI656O_HANDLE tHndl = 0;
    
    T_PPI656O_INST *pInst = (T_PPI656O_INST*) malloc (sizeof(T_PPI656O_INST));
    *pa_tError = ERR_NONE;
    
    if(pInst) {
        
        memset(pInst, 0, sizeof(T_PPI656O_INST));
        
        //get video standard fill standard descriptor
        switch(pa_ptConfig->tAVformat) {
            case AV_PAL: {
                pInst->tAVstandard = g_tPALstd;
                break;
            }
            case AV_NTSC: {
                pInst->tAVstandard = g_tNTSCstd;
                break;
            }
            
            default: {
                *pa_tError = ERR_PPI656O_UNKNWON_AV_STD;
                tHndl = 0;
                break;
            }
        }
        
        if(*pa_tError == ERR_NONE) {
        
            if(pa_ptConfig->cNumberOfPlanes == 0) {
                pa_ptConfig->cNumberOfPlanes = 1;
            }
        
            if(pa_ptConfig->cNumberOfPlanes > VDM_MAX_NOF_PLANES) {
                pa_ptConfig->cNumberOfPlanes = VDM_MAX_NOF_PLANES;
            }
        
            //malloc space for planebuffers
            pInst->pcPlaneBuffer[0] = (unsigned char*) malloc(pa_ptConfig->cNumberOfPlanes * pInst->tAVstandard.nLineStride * pInst->tAVstandard.nTotalLines );
        
            if(pInst->pcPlaneBuffer[0]) {
            
                unsigned short i;
            
                pInst->cDeviceNr = g_cPPI656OnoDevices;
                g_cPPI656OnoDevices++;
            
                pInst->pcFrameBuffer = pInst->pcPlaneBuffer[0];
            
                #if _PPI656O_DEBUG_LVL_ > 0
                    printf("(T_PPI656Oopen): Planebuffer 0 @ 0x%x\n", pInst->pcFrameBuffer);
                #endif
            
                //init planebuffers
                for(i = 1; i < pa_ptConfig->cNumberOfPlanes; i++) {
                    pInst->pcPlaneBuffer[i] = pInst->pcPlaneBuffer[i-1] + pInst->tAVstandard.nLineStride * pInst->tAVstandard.nTotalLines;
                    #if _PPI656O_DEBUG_LVL_ > 0
                        printf("(T_PPI656Oopen): Planebuffer %d @ 0x%x\n", i, pInst->pcPlaneBuffer[i]);
                    #endif
                }
            
                
                for(i = 0; i < pa_ptConfig->cNumberOfPlanes; i++) {
                    AVinitITU656Matrix(pInst->pcPlaneBuffer[i], &pInst->tAVstandard);
                }
            
                pInst->pHwDevHndl = pa_ptConfig->pHwDevHndl;
                pInst->cNumberOfPlanes = pa_ptConfig->cNumberOfPlanes;
                pInst->bOverwriteActivePlaneAllowed = (pInst->cNumberOfPlanes == 1) ? true : 
                                                      pa_ptConfig->bOverwriteActivePlaneAllowed; 
                pInst->cActivePlane = 0;
                if(pInst->cNumberOfPlanes > 1) {
                    pInst->cNextPlane = 1;
                } else {
                    pInst->cNextPlane = 0;
                }
                pInst->nFreePlanesCount = (pInst->cNumberOfPlanes == 1) ? 1 : -1;//  (pInst->cNumberOfPlanes - 1 + ((pInst->bOverwriteActivePlaneAllowed) ? 1 : 0));
                pInst->tAVformat = pa_ptConfig->tAVformat;
                pInst->cPPInr = pa_ptConfig->cPPInr;
                pInst->nPPIIVG = pa_ptConfig->nPPIIVG;            ///< PPI IVG 0 = standard
                pInst->nErrorIVG =  pa_ptConfig->nErrorIVG;          ///< PPI Error IVG 0 = standard
                pInst->nDMAErrorIVG =  pa_ptConfig->nDMAErrorIVG;
                pInst->bAutoUpdate =  pa_ptConfig->bAutoUpdate;
                pInst->cMDMAChannel = pa_ptConfig->cMDMAChannel;
                if(pa_ptConfig->fnDeviceClose) {
                    pInst->fnDeviceClose = pa_ptConfig->fnDeviceClose;
                }
                else {
                    pInst->fnDeviceClose = 0;
                }
                //fill functions for VDM
                pInst->pfGraficFunctions[VD_SET_PIXEL] = (void *)PPI656OsetPixel;
                pInst->pfGraficFunctions[VD_GET_PIXEL] = (void *)PPI656OgetPixel;
                pInst->pfGraficFunctions[VD_BLEND_PIXEL] = (void *)PPI656OblendPixel;
                pInst->pfGraficFunctions[VD_GET_ACTIVE_PLANE] = (void *)PPI656OgetActivePlane;
                pInst->pfGraficFunctions[VD_SET_ACTIVE_PLANE] = (void *)PPI656OsetActivePlane;
                pInst->pfGraficFunctions[VD_FSYNC_WAIT] = (void*)PPI656OwaitForFrameSync;
                pInst->pfGraficFunctions[VD_SHOW_FRAME] = (void *)PPI656OshowFrame;
                pInst->pfGraficFunctions[VD_GET_NEXT_PLANE] = (void *)PPI656OgetNextPlaneNum;
                *pfGraficFunctions = pInst->pfGraficFunctions;
                #ifdef _USE_VDK_
                //create semaphore for fsync
                pInst->tFsyncSem = VDK_CreateSemaphore(0, 1, 1, 0);
                #endif

                //setup DMA descriptor
                g_anPPI656ODMAdesc[(g_cPPI656OnoDevices -1)*  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 0] = (unsigned short)((unsigned long)&g_anPPI656ODMAdesc[(g_cPPI656OnoDevices -1)*  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 0] & 0xffff);            // NDPL
                g_anPPI656ODMAdesc[(g_cPPI656OnoDevices -1)*  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 1] = (unsigned short)((unsigned long)&g_anPPI656ODMAdesc[(g_cPPI656OnoDevices -1)*  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 0] >> 16);            // NDPH
                g_anPPI656ODMAdesc[(g_cPPI656OnoDevices -1)*  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 2] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[0] & 0xffff);    // SAL: start address frame buffer 0
                g_anPPI656ODMAdesc[(g_cPPI656OnoDevices -1)*  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 3] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[0] >> 16);        // SAH: start address frame buffer 0
            
                //setup ppi 
                //void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
            
                if (g_aPPIspec[pa_ptConfig->cPPInr].bInUse) {
                    *pa_tError = ERR_ALREADY_INITIALIZED;
                }
                //adi_int_ExitCriticalRegion(pExitCriticalArg);
        
                if (*pa_tError == ERR_NONE) {
                    // no error so far - continue      
                            
                        
#ifdef __ADSPBF561__    
                    // 32 Bit transfer
                    // set dma registers
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAxcount) = pInst->tAVstandard.nLineStride / 4;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAycount) = pInst->tAVstandard.nTotalLines;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAxmodify) =  2 * pInst->tAVstandard.nPixelStride;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAymodify) =  2 * pInst->tAVstandard.nPixelStride;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAnextDescrPtr) = (void *)&g_anPPI656ODMAdesc[VDM_MAX_NOF_PLANES * (g_cPPI656OnoDevices - 1) * PPI656O_DMA_DESC_SIZE];
                    
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAconfig) = 0x7498;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pPPIcontrol) = 0x01c6;
#else
                    // 16 Bit transfer
                    // set dma registers
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAxcount) = pInst->tAVstandard.nLineStride / 2;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAycount) = pInst->tAVstandard.nTotalLines;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAxmodify) =  pInst->tAVstandard.nPixelStride;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAymodify) =  pInst->tAVstandard.nPixelStride;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAnextDescrPtr) = (void *)&g_anPPI656ODMAdesc[VDM_MAX_NOF_PLANES * (g_cPPI656OnoDevices - 1) * PPI656O_DMA_DESC_SIZE];
                    
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAconfig) = 0x7494;
                    *(g_aPPIspec[pa_ptConfig->cPPInr].pPPIcontrol) = 0x00c6;
#endif
                
                    if ( ppi_platformInit(pa_ptConfig->cPPInr, *(g_aPPIspec[pa_ptConfig->cPPInr].pPPIcontrol)) ) {
   
                        if(!pa_ptConfig->nPPIIVG) {
                        
                            adi_int_SICGetIVG(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId, &pInst->nPPIIVG);
                        } else {
                            adi_int_SICSetIVG(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId, pa_ptConfig->nPPIIVG);
                            pInst->nPPIIVG = pa_ptConfig->nPPIIVG; //use custom ivg
                        }
                       
                        // hook the interrupt
                        if(adi_int_CECHook(pInst->nPPIIVG, PPI6565OinterruptHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                            adi_int_SICWakeup(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId, FALSE);
                            adi_int_SICEnable(g_aPPIspec[pa_ptConfig->cPPInr].peripheralIntId);
                        } else {
                            *pa_tError = ERR_PPI656O_HOOK_INTERRUPT;
                        }
                        
                        if(!pa_ptConfig->nErrorIVG) {
                            adi_int_SICGetIVG(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId, &pInst->nErrorIVG);
                        } else {
                            adi_int_SICSetIVG(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId, pa_ptConfig->nErrorIVG);
                            pInst->nErrorIVG = pa_ptConfig->nErrorIVG; //use custom ivg
                        }                        
                        //hook error interupt                             
                        if(adi_int_CECHook(pInst->nErrorIVG, PPI656OerrorHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                            adi_int_SICWakeup(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId, FALSE);
                            adi_int_SICEnable(g_aPPIspec[pa_ptConfig->cPPInr].errorIntId);
                        } else {
                            *pa_tError = ERR_PPI656O_HOOK_INTERRUPT;
                        }    

                        if(!pa_ptConfig->nDMAErrorIVG) {
                            adi_int_SICGetIVG(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId, &pInst->nDMAErrorIVG);
                        } else {
                            adi_int_SICSetIVG(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId, pa_ptConfig->nDMAErrorIVG);
                            pInst->nDMAErrorIVG = pa_ptConfig->nDMAErrorIVG; //use custom ivg
                        }       
                        // hook DMA error interrupt
                        if(adi_int_CECHook(pInst->nDMAErrorIVG, PPI6565ODMAErrorInterruptHandler, (void *)pInst, false) == ADI_INT_RESULT_SUCCESS) {
                            adi_int_SICWakeup(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId, FALSE);
                            adi_int_SICEnable(g_aPPIspec[pa_ptConfig->cPPInr].DMAerrorIntId);
                        } else {
                            *pa_tError = ERR_PPI656O_HOOK_INTERRUPT;
                        }
                        
                        
                        
                        
                        
                        
                        
                        
            //adi_int_SICSetIVG(g_aMDMAspec[pInst->cMDMAChannel].peripheralIntId, 15);
            //pInst->nErrorIVG = pa_ptConfig->nErrorIVG; //use custom ivg
            /*
            //hook error interupt                             
            if(adi_int_CECHook(15, MDMAErrorInterruptHandler, (void *)pInst, true) == ADI_INT_RESULT_SUCCESS) {
                adi_int_SICWakeup(g_aMDMAspec[pInst->cMDMAChannel].peripheralIntId, TRUE);
                adi_int_SICEnable(g_aMDMAspec[pInst->cMDMAChannel].peripheralIntId);
            } else {
                *pa_tError = ERR_PPI656O_HOOK_INTERRUPT;
            }    


            */
            
            
                                    
                        //enable ppi
                        *(g_aPPIspec[pa_ptConfig->cPPInr].pDMAconfig) |= 0x0001;
                        *(g_aPPIspec[pa_ptConfig->cPPInr].pPPIcontrol) |= 0x0001;
                    
                        tHndl = (T_PPI656O_HANDLE) pInst; 
                   } else {
                        // error the platform init function failed
                        *pa_tError = ERR_PPI6565O_PLATFORM_INIT;
                        free(pInst->pcPlaneBuffer[0]);
                        free(pInst);
                        tHndl = 0;
                    }
                } else {
                    // ppi is already initialized    
                       // free plane buffer and instances
                       free(pInst->pcPlaneBuffer[0]);
                       free(pInst);
                       *pa_tError =ERR_PPI656O_PPI_NOT_AVAILABLE;
                       tHndl = 0;
                       }      
                   
            } else {
                free(pInst);
                tHndl = 0;
                *pa_tError = ERR_PPI656O_OUT_OF_MEM;
            }
        } else {
            free(pInst);
            tHndl = 0;
            *pa_tError = ERR_PPI656O_UNKNWON_AV_STD;
        }
    } else {
        tHndl = 0;
        *pa_tError = ERR_PPI656O_OUT_OF_MEM;
    }    
    return tHndl;
}

/**

**/
T_ERROR_CODE T_PPI656Oclose(T_PPI656O_HANDLE pa_tHndl) {
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_PPI656O_INST *pInst = (T_PPI656O_INST *)pa_tHndl;
        unsigned short i;
        
        Sleep(1);
        
        // unhook the interrupts
        adi_int_CECUnhook(pInst->nPPIIVG, PPI6565OinterruptHandler, (void *)pInst);
        adi_int_CECUnhook(pInst->nErrorIVG, PPI656OerrorHandler, (void *)pInst);
        adi_int_CECUnhook(pInst->nDMAErrorIVG, PPI6565ODMAErrorInterruptHandler, (void *)pInst);
        // disable ppi
        *(g_aPPIspec[pInst->cPPInr].pDMAconfig) = 0xFFFE;
        *(g_aPPIspec[pInst->cPPInr].pPPIcontrol) = 0xFFFE;
        ssync();

        // close device
        if(pInst->fnDeviceClose && pInst->pHwDevHndl) {
            pInst->fnDeviceClose(pInst->pHwDevHndl);
        }
        
        free(pInst->pcPlaneBuffer[0]);
        
        #ifdef _USE_VDK_
            //destroy semaphore for fsync
            VDK_DestroySemaphore(pInst->tFsyncSem);
        #endif
        free(pInst);
        g_cPPI656OnoDevices--;
    } else {
        erResult = ERR_PPI656O_INVALID_HANDLE;
    }
    
    return erResult;
}

/**
    @brief set a pixel in itu 656 frame
**/
//#pragma section ("L1_code")
void PPI656OsetPixel (void* pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor) {
    register T_PPI656O_INST *pInst;
    register unsigned long nIndex;
    unsigned long nYUVcolor, nFlagValue;
    unsigned short i, nStartLine;
    unsigned char cU, cV, cY, cRed, cBlue, cGreen, cYnew, cUnew, cVnew;
    
    pInst = (T_PPI656O_INST*) pa_tHndl;
    
    //calculate position in frame
    //even or odd line
    if(y & 0x1 ) {
        nFlagValue = AV_FIELD_2;

    } else {
        nFlagValue = AV_FIELD_1;
    }
    
    //cylce trough fields to get start of active video
    for(i = 0; i < AV_FLAG_NUM ; i++) {
        if(pInst->tAVstandard.tFlags[i].nFlags == nFlagValue) {
            nStartLine = pInst->tAVstandard.tFlags[i].nLine;
            break;
        }
    } 
    
    //start of Active video
    nIndex = (unsigned long)pInst->pcPlaneBuffer[pa_cPlane] + nStartLine * pInst->tAVstandard.nLineStride; 
    nIndex += (y/2) * pInst->tAVstandard.nLineStride;   // move to correct line
    nIndex += pInst->tAVstandard.nBlankLineSize + x * pInst->tAVstandard.nPixelStride + 8 ;  //move to xpos
   
    if ((nIndex & 0x3) == 0) {
        // we are on a 4 byte boundary
        nYUVcolor = *(unsigned long *)nIndex;
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff;
        cY = (nYUVcolor >> 24) & 0xff;
        cRed = pa_nColor & 0xff;
        cGreen = (pa_nColor >> 8) & 0xff; 
        cBlue = (pa_nColor >> 16) & 0xff;
        cYnew = (unsigned char)((2449 * cRed + 4809 * cGreen + 943 * cBlue) >> 13);
        cUnew = cBlue - cYnew + 128;
        cVnew = cRed - cYnew + 128;
        // calc average of both crominanz values
        //cUnew = (cUnew + cU) >> 1;
        //cVnew = (cVnew + cV) >> 1;        
        // set new value
        *(unsigned long *)nIndex = (cY << 24) | (cVnew << 16) | (cYnew << 8) | cUnew;
    } else {
        // we are on a 2 byte boundary    
        nYUVcolor = *(unsigned long *)(nIndex - 2);
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff; 
        cY = (nYUVcolor >> 8) & 0xff;
        cRed = pa_nColor & 0xff;
        cGreen = (pa_nColor >> 8) & 0xff; 
        cBlue = (pa_nColor >> 16) & 0xff;
        cYnew = (unsigned char)((2449 * cRed + 4809 * cGreen + 943 * cBlue) >> 13);
        cUnew = cBlue - cYnew + 128;
        cVnew = cRed - cYnew + 128;
        // calc average of both crominanz values
        //cUnew = (cUnew + cU) >> 1;
        //cVnew = (cVnew + cV) >> 1;        
        // set new value
        *(unsigned long *)(nIndex - 2) = (cYnew << 24) | (cVnew << 16) | (cY << 8) | cUnew;        
    }
}


//#pragma section ("L1_code")
T_VD_COLOR PPI656OgetPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y) {
    register T_PPI656O_INST *pInst;
    register unsigned long nIndex ;
    unsigned long nYUVcolor, nFlagValue;
    unsigned short i, nStartLine;
    unsigned char cU, cV, cY;
    signed short nRed, nBlue, nGreen;
    unsigned long nVDcolor;
    
    
    pInst = (T_PPI656O_INST*) pa_tHndl;
    
    //calculate position in frame
    //even or odd line
    if(y & 0x1 ) {
        nFlagValue = AV_FIELD_2;

    } else {
        nFlagValue = AV_FIELD_1;
    }
    
    //cylce trough fields to get start of active video
    for(i = 0; i < AV_FLAG_NUM ; i++) {
        if(pInst->tAVstandard.tFlags[i].nFlags == nFlagValue) {
            nStartLine = pInst->tAVstandard.tFlags[i].nLine;
            break;
        }
    } 
    
    //start of Active video
    nIndex = (unsigned long)pInst->pcPlaneBuffer[pa_cPlane] + nStartLine * pInst->tAVstandard.nLineStride; 
    nIndex += (y/2) * pInst->tAVstandard.nLineStride;   // move to correct line
    nIndex += pInst->tAVstandard.nBlankLineSize + x * pInst->tAVstandard.nPixelStride + 6 ;  //move to xpos
    
    if ((nIndex & 0x3) == 0) {
        // we are on a 4 byte boundary
        nYUVcolor = *(unsigned long *)nIndex;
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff;
        cY = (nYUVcolor >> 24) & 0xff;
        nRed = cV - 128 + cY;
        if (nRed > 255)
            nRed = 255;
        if (nRed < 0  )
            nRed = 0;
        nBlue = cU - 128 + cY;
        if (nBlue > 255)
            nBlue = 255;
        if (nBlue < 0 ) 
            nBlue = 0;
        nGreen = (6922 * cY - 2130 * nRed - 778 * nBlue) >> 12;
        if (nGreen > 255) 
            nGreen = 255;
        if (nGreen < 0 ) 
            nGreen = 0; 
        nVDcolor = (nBlue << 16) | (nGreen << 8) | nRed;
    } else {
        // we are on a 2 byte boundary    
        nYUVcolor = *(unsigned long *)(nIndex - 2);
        cU = nYUVcolor & 0xff;
        cV = (nYUVcolor >> 16) & 0xff; 
        cY = (nYUVcolor >> 8) & 0xff;
        nRed = cV - 128 + cY;
        if (nRed > 255)
            nRed = 255;
        if (nRed < 0  )
            nRed = 0;
        nBlue = cU - 128 + cY;
        if (nBlue > 255)
            nBlue = 255;
        if (nBlue < 0 ) 
            nBlue = 0;
        nGreen = (6922 * cY - 2130 * nRed - 778 * nBlue) >> 12;
        if (nGreen > 255) 
            nGreen = 255;
        if (nGreen < 0 ) 
            nGreen = 0;
        nVDcolor = (nBlue << 16) | (nGreen << 8) | nRed;
    }
    
    return nVDcolor;
}


//#pragma section ("L1_code")
void PPI656OblendPixel(void *pa_tHndl, unsigned char pa_cPlane, unsigned short x, unsigned short y, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha) {
    unsigned char cRed;
    unsigned char cGreen;
    unsigned char cBlue;
    // fetch the current pixel color
    T_VD_COLOR nColor = PPI656OgetPixel(pa_tHndl, pa_cPlane, x, y);
    // calc the blend colors
    cRed = ((pa_nColor & 0xff) * pa_cAlpha + (nColor & 0xff) * (255 - pa_cAlpha)) >> 8;
    cGreen = (((pa_nColor & 0xff00) >> 8) * pa_cAlpha + ((nColor & 0xff00) >> 8) * (255 - pa_cAlpha)) >> 8;
    cBlue = (((pa_nColor & 0xff0000) >> 16) * pa_cAlpha + ((nColor & 0xff0000) >> 16) * (255 - pa_cAlpha)) >> 8;
    // set the new pixel color
    PPI656OsetPixel(pa_tHndl, pa_cPlane, x, y, cRed | (cGreen << 8) | (cBlue << 16));
}



signed char  PPI656OgetActivePlane(void *pa_tHndl){
    T_PPI656O_INST *pInst;
    
    if(pa_tHndl) {
        pInst = (T_PPI656O_INST*) pa_tHndl;
        return pInst->cActivePlane;
    } else {
        return (signed char)ERR_PPI656O_INVALID_HANDLE;
    }
    
    
}





T_ERROR_CODE PPI656OsetActivePlane(void *pa_tHndl, unsigned char pa_cPlane){
    T_PPI656O_INST *pInst;
    
    if(pa_tHndl) {
        pInst = (T_PPI656O_INST*) pa_tHndl;
        
        g_anPPI656ODMAdesc[pInst->cDeviceNr *  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 2] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pa_cPlane] & 0xffff);    // SAL: start address frame buffer 0
        g_anPPI656ODMAdesc[pInst->cDeviceNr *  VDM_MAX_NOF_PLANES * PPI656O_DMA_DESC_SIZE + 3] = (unsigned short)((unsigned long)pInst->pcPlaneBuffer[pa_cPlane] >> 16);        // SAH: start address frame buffer 0
           
        pInst->cNextPlane = pa_cPlane;
        pInst->bPlaneSwitchRequest = true;
        
        return ERR_NONE;
    } else {
        return ERR_PPI656O_INVALID_HANDLE;
    }
    
    
}



T_ERROR_CODE PPI656OwaitForFrameSync(void *pa_tHndl) {
    T_PPI656O_INST *pInst;
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        pInst = (T_PPI656O_INST*) pa_tHndl;
        
        pInst->bVSYNC = false;
        unsigned short nTimeout = PPI656O_FSYNC_TIMEOUT;
        
        while ((!pInst->bVSYNC) && nTimeout) {
            Sleep(1);
            nTimeout --;
        }
        
        if(!nTimeout) {
            erResult = ERR_PPI656O_FSYNC_TIMEOUT;
        }
      
    } else {
        erResult = ERR_PPI656O_INVALID_HANDLE;
    }
    
    return erResult;
}


unsigned char PPI656OgetNextPlaneNum(void *pa_tHndl) {
    
    T_PPI656O_INST *pInst = (T_PPI656O_INST*) pa_tHndl;
    unsigned char cNewValue = pInst->cActivePlane + 1;
    
    return((cNewValue >= pInst->cNumberOfPlanes) ? 0 : cNewValue);
}



T_ERROR_CODE PPI656OshowFrame(void *pa_tHndl, 
                              unsigned char *pa_pcFrameBuffer, 
                              unsigned short pa_nXoffset, 
                              unsigned short pa_nYoffset, 
                              unsigned short pa_nXsize, 
                              unsigned short pa_nYsize) {
                                  
    T_ERROR_CODE erResult = ERR_NONE;
    
    if(pa_tHndl) {
        T_PPI656O_INST *pInst = (T_PPI656O_INST*) pa_tHndl;
        
        if((pInst->nFreePlanesCount || pInst->bOverwriteActivePlaneAllowed) && (pa_nXsize == pInst->tAVstandard.nWidth)) {
            unsigned short nNextInputPlane;   
            T_AV_FRAME_BUFFER_DESC tFb;
                
            // build a frame buffer with the used video standard
            tFb.width = pInst->tAVstandard.nWidth; 
            tFb.height = pInst->tAVstandard.nHeight ;
            tFb.offset = pInst->tAVstandard.nActiveVideo1Offset;
            tFb.pixel_stride = pInst->tAVstandard.nPixelStride;
            tFb.line_stride = pInst->tAVstandard.nLineStride;
            tFb.field_stride = pInst->tAVstandard.nFieldStride;
            tFb.interlaced = 1;
        
            if(pInst->nFreePlanesCount < 0) {
                if(pInst->cNumberOfPlanes > 1) {
                    nNextInputPlane = 1;
                } else {
                    nNextInputPlane = 0;
                }
                pInst->nFreePlanesCount = pInst->cNumberOfPlanes;
            }
            else {
                if(pInst->nFreePlanesCount == 0) {
                    if(pInst->bOverwriteActivePlaneAllowed) {
                        nNextInputPlane = pInst->cActivePlane;
                    }
                    else {
                        return(ERR_PPI656O_NO_FREE_PLANE);
                    }
                }
                else {
                    nNextInputPlane = pInst->cActivePlane + pInst->cNumberOfPlanes - pInst->nFreePlanesCount;
                    if(nNextInputPlane >= pInst->cNumberOfPlanes) {
                        nNextInputPlane -= pInst->cNumberOfPlanes;
                    }
            
                    // all planes are free
                    if((pInst->nFreePlanesCount == pInst->cNumberOfPlanes) && (pInst->cNumberOfPlanes != 1)) {
                        // get the first plane after the active plane
                        if((++nNextInputPlane) >= pInst->cNumberOfPlanes) {
                            nNextInputPlane -= pInst->cNumberOfPlanes;
                        }
                    }
                }
            }
            
            tFb.start_addr = pInst->pcPlaneBuffer[nNextInputPlane];
                    
            // check Ysize of the frame to be show
            if(pa_nYsize > tFb.height) {
                pa_nYsize = tFb.height;
            }
                    
            //copy frame
            erResult = AV_CopyYUV422toITUR656Frame(&tFb, 
                                     pa_nXoffset,
                                     pa_nYoffset,
                                     (char *)pa_pcFrameBuffer, 
                                     pa_nXsize, 
                                     pa_nYsize, 
                                     pInst->cMDMAChannel);

            if(pInst->nFreePlanesCount) {                                     
                if((pInst->cNumberOfPlanes == 1) && pInst->bOverwriteActivePlaneAllowed) {
                    pInst->nFreePlanesCount = 1;
                }
                else {
                    pInst->nFreePlanesCount--;
                }
            }
            
            //pInst->cNextInputPlane = PPI656OgetNewNextInputPlane(pa_tHndl);
        }  
        else {
            erResult = ERR_PPI656O_NO_FREE_PLANE;
        }      
    } 
    else {
        erResult = ERR_PPI656O_INVALID_HANDLE;
    }
    
    return erResult;
}
