#include "environment.h"
#include <stdio.h>
#include <conio.h>
#include <services/services.h>
#include <drivers/adi_dev.h>
#include <cycle_count_bf.h>
#include <stdlib.h>
#include <string.h>
#include "../../../../../driver/src/common/GPIOconfig.h"
#include "../../../../../driver/src/common/UART/UARTconfig.h"
#include "../../../../../driver/src/common/EPPIconfig.h"
#include "../../../../../driver/src/common/SCCBconfig.h"
#include "../../../../../driver/src/common/camera/OVxxxx/OVxxxx.h"
#include "../../../../../driver/src/common/mmio/EPPIcameraIn/EPPIcameraIn.h"
#include "../../../../../driver/src/common/camera/camera.h"
#include "../../../../../blacksheep/common/framegrabber/frameGrabber.h"

#include "../../../../../blacksheep/common/term/xmodemUART-buf.h"

#ifdef _USE_HITTX09D_
    #include "../../../../../driver/src/common/display/TX09D70VM1CCA/HitachiTX09D.h"   
    
    #if HITTX09D_USE_BACKGROUND_IMAGE
        #include "../../../../../BLACKSheep/common/images/BS_Logo320x240_888.h"
        extern unsigned char g_BackgroundImage[];
    #endif
#endif    


unsigned long long cur[20];

unsigned short g_nReadIndex;

// prototypes
void EPPIHandler(T_EPPI_CAMERA_IN_CALLBACK_ARG pa_stBufferStatus);
void imageConvertYUVToRGB (unsigned long startAddressYUV, unsigned long startAddressRGB, unsigned short xWidth, unsigned short yWidth);
void imageConvertYUVToRGB32Bit (unsigned long startAddressYUV, unsigned long startAddressRGB, unsigned short xWidth, unsigned short yWidth);
void imageConvertRGB565ToYUV422 (unsigned long *pa_pRGBbuffer, unsigned long *pa_pYUVbuffer, unsigned short pa_nXsize, unsigned short pa_nYsize);
void imageConvertRaw10BitOrder (unsigned long *pa_pRawBuffer, unsigned short pa_nRawXsize, unsigned short pa_nRawYsize);
unsigned long imageConvertRGBToWindowsBmp (unsigned short xstart, unsigned short ystart, unsigned short xlength, unsigned short ylength, unsigned long pa_nBuffer, unsigned long pa_nOutBuffer);
void imageConvertRaw10ToYUV422 (unsigned long *pa_pRawBuffer, unsigned long *pa_pYUVBuffer, unsigned short pa_nRawXsize, unsigned short pa_nRawYsize);
void resizeImage(unsigned long *pa_pResizeBuffer, unsigned long *pa_pRGBbuffer, unsigned short pa_nXsize, unsigned short pa_nYsize, unsigned short pa_nFactor);


void RGB24toRGB565 ( unsigned char *pa_pcBits24, unsigned int pa_nWidth, unsigned int pa_nHeight, unsigned short *pa_pnPixel16 )
{
  
  int i                    = 0;
  unsigned char red        = 0;
  unsigned char green      = 0;
  unsigned char blue       = 0;
  unsigned short RGB2Bytes = 0;
  
  unsigned int size = pa_nWidth * pa_nHeight;
  
   
  for(i = 0 ; i < size ; i++) {      
      red   = *pa_pcBits24++ >> 3;  // 5-bit red;
      green = *pa_pcBits24++ >> 2;  // 6-bit green;
      blue  = *pa_pcBits24++ >> 3;  // 5-bit blue; 
      //TFT requires big endian
      RGB2Bytes = red | (green<<5) | (blue<<(5+6));
      
      *pa_pnPixel16++ = RGB2Bytes;
  }
  
}


// memory for the adi interrupt manager
#pragma alignment_region (4)
static unsigned char s_cDevMngStorage[ADI_DEV_BASE_MEMORY + (ADI_DEV_DEVICE_MEMORY * 5)];
static unsigned char s_cIntMngStorage[ADI_INT_SECONDARY_MEMORY * BS_INT_MNG_NOF_SECONDARY_HANDLER * BS_NOF_USER_IVGS];                         ///< Pointer to an area of memory used by the interrupt manager
static u8 DMAMgrData[ADI_DMA_BASE_MEMORY + (ADI_DMA_CHANNEL_MEMORY * 4)];
#pragma alignment_region_end
ADI_DEV_MANAGER_HANDLE g_hADIDevMng;                     // ADI Device Manager Handle

void main (void) {
    
	
	T_ERROR_CODE erResult = ERR_NONE;
	char sMessage[255];
	unsigned long vco, cclk, sclk;
#ifdef USE_CAM1
    bool bCAM1deactivate = false;
	T_SCCB_HANDLE hSCCBhndl1;
	T_OVXXXX_CONF stOVcamConfig1;
	T_OVXXXX_HANDLE hOVcamHndl1;
	T_EPPI_CAMERA_IN_CONF stEPPIcamInConfig1;
	T_EPPI_CAMERA_IN_HANDLE hEPPIcamInHndl1;
	ifrm_t *stFrameBufferCAM1;
	unsigned short nFactorCAM1;
	void *pRGBbufferCAM1 = 0;
	void *pResizeBufferCAM1 = 0;
	void *pBMPbufferCAM1 = 0;
	unsigned long nFileSizeCAM1 = 0;
#endif
	unsigned short i;
	
	

    unsigned long nResult = ADISSLinit(&g_hADIDevMng, 
                                       s_cIntMngStorage, 
                                       sizeof(s_cIntMngStorage),
                                       s_cDevMngStorage,
                                       sizeof(s_cDevMngStorage));

    cclk = getCoreClockFrequency();
    sclk = getSystemClockFrequency();
	
	g_nReadIndex = 0;
	
	// open the uart	
	T_UART_HANDLE hUART = uart_open(0, sclk, 1024, 1024, 0);
	uart_setMode(hUART, UART_BAUD_RATE, UART_NONE, 8, 1);
	
	uart_writeString(hUART, "\xd\xa\xd\xa");
	sprintf(sMessage, "Core Clock:\t%u Hz\n\rSystem Clock:\t%u Hz\n\r\n\r", cclk, sclk);
	uart_writeString(hUART, (unsigned char *)sMessage);
	
#ifdef _USE_HITTX09D_	
    T_HITTX09_CONFIG tHITTC09DConf;
    tHITTC09DConf.cPPI = 1;
    tHITTC09DConf.nCoreClk = cclk;
    tHITTC09DConf.nSystemClk = sclk;
    tHITTC09DConf.cBytesPerPixel = 2;       // use drivers default value
    tHITTC09DConf.cPWMtimer = HITTX09_PWM_TIMER;
    tHITTC09DConf.cDTMGshiftTimer = HITTX09_DTMG_SHIFT_TIMER;
    tHITTC09DConf.bCreateDCLK = true;
    tHITTC09DConf.cDCLKtimer = HITTX09_DCLK_TIMER;
    tHITTC09DConf.cPCIflag = HITTX09_PCI_PIN;
    tHITTC09DConf.cNofPlanes = HITTX09_NOF_PLANES;
    tHITTC09DConf.nIVG = HITTX09_PERIPHERAL_IVG;
    tHITTC09DConf.nErrIVG = HITTX09_PERIPHERAL_ERROR_IVG;         
    tHITTC09DConf.nDmaErrIVG = HITTX09_DMA_ERROR_IVG;
    tHITTC09DConf.bOverlayCapability = false;
    tHITTC09DConf.bAutoUpdate = true;
    tHITTC09DConf.nBckGrndImage = NULL;
    tHITTC09DConf.nDefColor = HITTX09D_DEFAULT_BACKGROUND_COLOR;

#if HITTX09D_USE_BACKGROUND_IMAGE
    tHITTC09DConf.nBckGrndImage = (unsigned short *)malloc (HITTX09_XSIZE * HITTX09_YSIZE * HITTX09_DEFAULT_BYTE_PER_PIXEL);

    RGB24toRGB565((unsigned char *)g_BackgroundImage, 
                  HITTX09_XSIZE, 
                  HITTX09_YSIZE, 
                  (unsigned short *)tHITTC09DConf.nBckGrndImage);
#endif  
    unsigned char **pcPlaneBuffer;
    void **pfGraficFunctions;

    T_HITTX09_HNDL tHndl = HITTX09open (&tHITTC09DConf, 
                                        (unsigned char ***)&pcPlaneBuffer, 
                                        (void ***)&pfGraficFunctions, 
                                        0, 
                                        0);
    while(1);                                        
                                        
#endif                
	
#ifdef USE_CAM1	
	uart_writeString(hUART, "CAM1: Open SCCB connection to the camera... ");
	
	//open sccb connection
	erResult = sccb_open(&hSCCBhndl1, CAM_SIO_C1_PIN, CAM_SIO_D1_PIN, CAM_PWDN1_PIN, 0, cclk);
	
	if(erResult != ERR_NONE) {
	    uart_writeString(hUART, "Error\xd\xa");
	    while(1) {
	        ;
	    }
	} else {
	    uart_writeString(hUART, "done\xd\xa");
	}
	
#endif
#ifdef USE_CAM2	
	uart_writeString(hUART, "CAM2: Open SCCB connection to the camera... ");
	
	//open sccb connection
	erResult = sccb_open(&hSCCBhndl2, CAM_SIO_C2_PIN, CAM_SIO_D2_PIN, CAM_PWDN2_PIN, 0, cclk);
	
	if(erResult != ERR_NONE) {
	    uart_writeString(hUART, "Error\xd\xa");
	    while(1) {
	        ;
	    }
	} else {
	    uart_writeString(hUART, "done\xd\xa");
	}
#endif	
#ifdef USE_CAM1	
	// configuration of camera open
	stOVcamConfig1.nColorMode = DEFAULT_COLOR_MODE_CAM1;
	stOVcamConfig1.nCameraInterface = OVT_SCCB;
	stOVcamConfig1.nResolution = DEFAULT_RESOLUTION_CAM1;
	stOVcamConfig1.nCameraModel = DEFAULT_CAMERA_CAM1;
	stOVcamConfig1.nCameraID = 0;
	stOVcamConfig1.hCamInterfaceHandle = hSCCBhndl1;
	stOVcamConfig1.fFramerate = DEFAULT_FRAMERATE_CAM1;
	
	uart_writeString(hUART, "CAM1: Configuring the camera... ");
	Sleep(50);
	hOVcamHndl1 = OVxxxx_Open(&stOVcamConfig1);
	
	if(!hOVcamHndl1) {
	    sprintf (sMessage, "Error: %d\xd\xa", stOVcamConfig1.erError);
		uart_writeString (hUART, (unsigned char *)sMessage);
#ifdef USE_CAM2
        bCAM1deactivate = true;
#else
		while(1) {
		    ;
		}
#endif
	} else {
	    uart_writeString(hUART, "done\xd\xa");
	    sprintf (sMessage, "CAM1: Camera OV%x found and initialized\xd\xa", stOVcamConfig1.nCameraID);
		uart_writeString (hUART, (unsigned char *)sMessage);
	}
#endif
#ifdef USE_CAM2	
	// configuration of camera open
	stOVcamConfig2.nColorMode = DEFAULT_COLOR_MODE_CAM2;
	stOVcamConfig2.nCameraInterface = OVT_SCCB;
	stOVcamConfig2.nResolution = DEFAULT_RESOLUTION_CAM2;
	stOVcamConfig2.nCameraModel = DEFAULT_CAMERA_CAM2;
	stOVcamConfig2.nCameraID = 0;
	stOVcamConfig2.hCamInterfaceHandle = hSCCBhndl2;
	stOVcamConfig2.fFramerate = DEFAULT_FRAMERATE_CAM2;
	
	uart_writeString(hUART, "CAM2: Configuring the camera... ");
	Sleep(50);
	hOVcamHndl2 = OVxxxx_Open(&stOVcamConfig2);
	
	if(!hOVcamHndl2) {
	    sprintf (sMessage, "Error: %d\xd\xa", stOVcamConfig2.erError);
		uart_writeString (hUART, (unsigned char *)sMessage);
		
#ifdef USE_CAM1
        bCAM2deactivate = true;
#else
		while(1) {
		    ;
		}
#endif
	} else {
	    uart_writeString(hUART, "done\xd\xa");
	    sprintf (sMessage, "CAM2: Camera OV%x found and initialized\xd\xa", stOVcamConfig2.nCameraID);
		uart_writeString (hUART, (unsigned char *)sMessage);
	}
#endif
#ifdef USE_CAM1	
    if(!bCAM1deactivate) {
    	// configuration of data interface (EPPI)
    	stEPPIcamInConfig1.nBufferMode = DEFAULT_BUFFER_MODE_CAM1;
    	stEPPIcamInConfig1.fnCallback = EPPIHandler;
    	stEPPIcamInConfig1.fnErrorCallback = 0;
    	stEPPIcamInConfig1.nDataInterfaceMode = stOVcamConfig1.nDataInterfaceMode;
    	stEPPIcamInConfig1.cEPPI = DEFAULT_EPPI_CAM1;
    	stEPPIcamInConfig1.nXres = stOVcamConfig1.nXres;
    	stEPPIcamInConfig1.nYres = stOVcamConfig1.nYres;
    	stEPPIcamInConfig1.nPerIVG = 0;
    	stEPPIcamInConfig1.nPerErrorIVG = 0;
    	stEPPIcamInConfig1.fFramerate = stOVcamConfig1.fFramerate;
    	stEPPIcamInConfig1.cSDRAMsize = DEFAULT_SDRAM_MEMORY_CAM1;
    	stEPPIcamInConfig1.nMaxFrames = DEFAULT_MAX_FRAMES_CAM1;
    	stEPPIcamInConfig1.nDmaErrorIVG = 0;
    	stEPPIcamInConfig1.fnGetTimestamp = NULL;
	
    	uart_writeString(hUART, "CAM1: Configuring the Data Interface... ");
	
    	hEPPIcamInHndl1 = EPPIcameraIn_open(&stEPPIcamInConfig1);
	
    	if(!hEPPIcamInHndl1) {
    	    sprintf (sMessage, "Error during EPPI camera In initialization: %d\xd\xa", stEPPIcamInConfig1.erError);
    		uart_writeString (hUART, (unsigned char *)sMessage);
    		while(1) {
    		    ;
    		}
    	} else {
    	    sprintf (sMessage, "EPPI%d as Data Interface initialized\xd\xa", stEPPIcamInConfig1.cEPPI);
    		uart_writeString (hUART, (unsigned char *)sMessage);
    	}
    }
#endif

    uart_writeString (hUART, "Give camera time to stabilize...");
	Sleep(1000);
	uart_writeString (hUART, "finished \xd\xa");
#ifdef USE_CAM1	
    if(!bCAM1deactivate) {
    	EPPIcameraIn_start(hEPPIcamInHndl1);
    	uart_writeString(hUART, "CAM1: Start image capturing...\xd\xa");
    }
#endif

//    while(1);
#if defined (USE_CAM1) && defined (USE_CAM2)
    uart_writeString (hUART, "Give cameras time to capture frames...");
#else
    uart_writeString (hUART, "Give camera time to capture frames...");
#endif
	Sleep(1000);                            // give camera time to capture frames
	uart_writeString (hUART, "finished \xd\xa");
	
	unsigned short *pBufferTest[10];
//	ifmr_t stFrameBuffer;
	
	

//	T_EPPI_CAMERA_IN_CALLBACK_ARG stFrameBuffer = EPPIcameraIn_getNextFrame(hEPPIcamInHndl1);
#ifdef USE_CAM1    
    if(!bCAM1deactivate) {
        while(!EPPIcameraIn_framesAvailable(hEPPIcamInHndl1)) {
            ;
        }
	
        stFrameBufferCAM1 = EPPIcameraIn_getNextFrame(hEPPIcamInHndl1);
	
        EPPIcameraIn_freeFrame(hEPPIcamInHndl1);
        EPPIcameraIn_stop(hEPPIcamInHndl1);
    
    	sprintf (sMessage, "CAM1: image capturing finished\xd\xa");
    	uart_writeString (hUART, (unsigned char *)sMessage);
    }
#endif	
	Sleep(100);
#ifdef 	USE_CAM1
    if(!bCAM1deactivate) {
    	pRGBbufferCAM1 = malloc(stEPPIcamInConfig1.nXres * stEPPIcamInConfig1.nYres * 4);
	
    	if(!pRGBbufferCAM1) {
    	    uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	    while(1) {
    	        ;
    	    }
    	}
    	
    	if(stOVcamConfig1.nColorMode == CAM_RAW) {
    	    imageConvertRaw10BitOrder ((unsigned long *)stFrameBufferCAM1->pucFramePtr, stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres);
    	    imageConvertRaw10ToYUV422 ((unsigned long *)stFrameBufferCAM1->pucFramePtr, (unsigned long *)stFrameBufferCAM1->pucFramePtr, stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres);
    	    
    	    stEPPIcamInConfig1.nXres = stEPPIcamInConfig1.nXres / 2;
    	    stEPPIcamInConfig1.nYres = stEPPIcamInConfig1.nYres / 2;
    	} else if(stOVcamConfig1.nColorMode == CAM_RGB_565) {
    	    imageConvertRGB565ToYUV422 ((unsigned long *)stFrameBufferCAM1->pucFramePtr, (unsigned long *)stFrameBufferCAM1->pucFramePtr, stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres);
    	}
    	
    	uart_writeString(hUART, "CAM1: Converting to RGB32bit... ");
	
    	imageConvertYUVToRGB32Bit ((unsigned long)stFrameBufferCAM1->pucFramePtr, (unsigned long)pRGBbufferCAM1, stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres);
	
    	uart_writeString (hUART, "done\xd\xa");	
    }
#endif
#ifdef 	USE_CAM2
    if(!bCAM2deactivate) {
    	pRGBbufferCAM2 = malloc(stEPPIcamInConfig2.nXres * stEPPIcamInConfig2.nYres * 4);
	
    	if(!pRGBbufferCAM2) {
    	    uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	    while(1) {
    	        ;
    	    }
    	}
    	
	    if(stOVcamConfig2.nColorMode == CAM_RAW) {
    	    imageConvertRaw10BitOrder ((unsigned long *)stFrameBufferCAM2->pucFramePtr, stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres);
    	    imageConvertRaw10ToYUV422 ((unsigned long *)stFrameBufferCAM2->pucFramePtr, (unsigned long *)stFrameBufferCAM2->pucFramePtr, stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres);
    	    
    	    stEPPIcamInConfig2.nXres = stEPPIcamInConfig2.nXres / 2;
    	    stEPPIcamInConfig2.nYres = stEPPIcamInConfig2.nYres / 2;
    	} else if(stOVcamConfig2.nColorMode == CAM_RGB_565) {
    	    imageConvertRGB565ToYUV422 ((unsigned long *)stFrameBufferCAM2->pucFramePtr, (unsigned long *)stFrameBufferCAM2->pucFramePtr, stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres);
    	}
    	
    	uart_writeString(hUART, "CAM2: Converting to RGB32bit... ");
	
    	imageConvertYUVToRGB32Bit ((unsigned long)stFrameBufferCAM2->pucFramePtr, (unsigned long)pRGBbufferCAM2, stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres);
	
    	uart_writeString (hUART, "done\xd\xa");	
    }
#endif
#ifdef USE_CAM1	
    if(!bCAM1deactivate) {
    	nFactorCAM1 = stEPPIcamInConfig1.nXres / RESIZE_X_VALUE_CAM1;
    	pResizeBufferCAM1;
	
    	if(nFactorCAM1 > 1) {
    	    uart_writeString(hUART, "CAM1: Resizing image... ");
    	    pResizeBufferCAM1 = malloc((stEPPIcamInConfig1.nXres * stEPPIcamInConfig1.nYres * 4) / nFactorCAM1);
	    
    	    if(!pResizeBufferCAM1) {
    	        uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	        while(1) {
    	            ;
    	        }
    	    }
	
    	    resizeImage(pResizeBufferCAM1, (unsigned long *)pRGBbufferCAM1, stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres, nFactorCAM1);
    	    stEPPIcamInConfig1.nXres = stEPPIcamInConfig1.nXres / nFactorCAM1;
    	    stEPPIcamInConfig1.nYres = stEPPIcamInConfig1.nYres / nFactorCAM1;
	    
    	    uart_writeString (hUART, "done\xd\xa");
	    
    	} else {
    	    pResizeBufferCAM1 = malloc((stEPPIcamInConfig1.nXres * stEPPIcamInConfig1.nYres * 4));
	    
    	    if(!pResizeBufferCAM1) {
    	        uart_writeString(hUART, "CAM1: ERROR! not enough memory for buffer allocating!\xd\xa");
    	        while(1) {
    	            ;
    	        }
    	    }
	    
    	    memcpy((void *)pResizeBufferCAM1, (void *)pRGBbufferCAM1, (stEPPIcamInConfig1.nXres * stEPPIcamInConfig1.nYres * 4));
    	}
	
    	free(pRGBbufferCAM1);
    	sprintf (sMessage, "CAM1: image size: %d x %d\xd\xa", stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres);
    	uart_writeString (hUART, (unsigned char *)sMessage);
    }
#endif
#ifdef USE_CAM2	
    if(!bCAM2deactivate) {
    	nFactorCAM2 = stEPPIcamInConfig2.nXres / RESIZE_X_VALUE_CAM2;
    	pResizeBufferCAM2;
	
    	if(nFactorCAM2 > 1) {
    	    uart_writeString(hUART, "CAM2: Resizing image... ");
    	    pResizeBufferCAM2 = malloc((stEPPIcamInConfig2.nXres * stEPPIcamInConfig2.nYres * 4) / nFactorCAM2);
	    
    	    if(!pResizeBufferCAM2) {
    	        uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	        while(1) {
    	            ;
    	        }
    	    }
	
    	    resizeImage(pResizeBufferCAM2, (unsigned long *)pRGBbufferCAM2, stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres, nFactorCAM2);
    	    stEPPIcamInConfig2.nXres = stEPPIcamInConfig2.nXres / nFactorCAM2;
    	    stEPPIcamInConfig2.nYres = stEPPIcamInConfig2.nYres / nFactorCAM2;
	    
    	    uart_writeString (hUART, "done\xd\xa");
	    
    	} else {
    	    pResizeBufferCAM2 = malloc((stEPPIcamInConfig2.nXres * stEPPIcamInConfig2.nYres * 4));
	    
    	    if(!pResizeBufferCAM2) {
    	        uart_writeString(hUART, "CAM1: ERROR! not enough memory for buffer allocating!\xd\xa");
    	        while(1) {
    	            ;
    	        }
    	    }
	    
    	    memcpy((void *)pResizeBufferCAM2, (void *)pRGBbufferCAM2, (stEPPIcamInConfig2.nXres * stEPPIcamInConfig2.nYres * 4));
    	}
	
    	free(pRGBbufferCAM2);
    	sprintf (sMessage, "CAM2: image size: %d x %d\xd\xa", stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres);
    	uart_writeString (hUART, (unsigned char *)sMessage);
    }
#endif
#ifdef USE_CAM1
    if(!bCAM1deactivate) {
    	uart_writeString(hUART, "CAM1: Converting to Windows BMP file format... ");
	
    	pBMPbufferCAM1 = malloc((stEPPIcamInConfig1.nXres * stEPPIcamInConfig1.nYres * 4));
	
    	if(!pBMPbufferCAM1) {
    	    uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	    while(1) {
    	        ;
    	    }
    	}
	
    	nFileSizeCAM1 = imageConvertRGBToWindowsBmp(0, 0, stEPPIcamInConfig1.nXres, stEPPIcamInConfig1.nYres, (unsigned long)pResizeBufferCAM1, (unsigned long)pBMPbufferCAM1);
    	free(pResizeBufferCAM1);
    	uart_writeString(hUART, "done\xd\xa");
    }
#endif
#ifdef USE_CAM2
    if(!bCAM2deactivate) {
    	uart_writeString(hUART, "CAM2: Converting to Windows BMP file format... ");
	
    	pBMPbufferCAM2 = malloc((stEPPIcamInConfig2.nXres * stEPPIcamInConfig2.nYres * 4));
	
    	if(!pBMPbufferCAM2) {
    	    uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	    while(1) {
    	        ;
    	    }
    	}
	
    	nFileSizeCAM2 = imageConvertRGBToWindowsBmp(0, 0, stEPPIcamInConfig2.nXres, stEPPIcamInConfig2.nYres, (unsigned long)pResizeBufferCAM2, (unsigned long)pBMPbufferCAM2);
    	free(pResizeBufferCAM2);
    	uart_writeString(hUART, "done\xd\xa");
    }
#endif
#ifdef USE_CAM1
    if(!bCAM1deactivate) {
    	uart_writeString (hUART, "CAM1: Start file receiving on your terminal now. Protocol \"xmodem\".\xd\xa");
    	uart_writeString (hUART, "CAM1: Save file on host pc as \"*.bmp\"...");
	
    	long nBytesTransmittetCAM1 = XM_TransmitFile (hUART, (unsigned char *)pBMPbufferCAM1, nFileSizeCAM1);
    	if (nBytesTransmittetCAM1 > 0) {
    		sprintf (sMessage, "successfully done. %u bytes transmittet.\xd\xa", nBytesTransmittetCAM1);
    		uart_writeString (hUART, (unsigned char *)sMessage);
    		uart_writeString (hUART, "CAM1: You can open the file with an image viewer.\xd\xa");
    	} else {
    		uart_writeString (hUART, "failed.\xd\xa");
    	}
	
    	free(pBMPbufferCAM1);
    	EPPIcameraIn_close(hEPPIcamInHndl1);
    	OVxxxx_Close(hOVcamHndl1);
    }
#endif	
#ifdef USE_CAM2
    if(!bCAM2deactivate) {
    	uart_writeString (hUART, "CAM2: Start file receiving on your terminal now. Protocol \"xmodem\".\xd\xa");
    	uart_writeString (hUART, "CAM2: Save file on host pc as \"*.bmp\"...");
	
    	long nBytesTransmittetCAM2 = XM_TransmitFile (hUART, (unsigned char *)pBMPbufferCAM2, nFileSizeCAM2);
    	if (nBytesTransmittetCAM2 > 0) {
    		sprintf (sMessage, "successfully done. %u bytes transmittet.\xd\xa", nBytesTransmittetCAM2);
    		uart_writeString (hUART, (unsigned char *)sMessage);
    		uart_writeString (hUART, "CAM2: You can open the file with an image viewer.\xd\xa");
    	} else {
    		uart_writeString (hUART, "failed.\xd\xa");
    	}
	
    	free(pBMPbufferCAM2);
    	EPPIcameraIn_close(hEPPIcamInHndl2);
    	OVxxxx_Close(hOVcamHndl2);
    }
#endif		
	while(1) {
	    ;
	}
}

// interrupt handler for EPPI interrupt
void EPPIHandler(T_EPPI_CAMERA_IN_CALLBACK_ARG pa_stBufferStatus) {
	// set flag to signal end of image transfer+
	
}


void resizeImage(unsigned long *pa_pResizeBuffer, unsigned long *pa_pImageBuffer, unsigned short pa_nXsize, unsigned short pa_nYsize, unsigned short pa_nFactor) {
    
    unsigned short i, k;
    
    for(i=0; i<pa_nYsize; i+=pa_nFactor) {
        for(k=0; k<pa_nXsize; k+=pa_nFactor) {
            *(pa_pResizeBuffer++) = *(pa_pImageBuffer + ((i * pa_nXsize) + k));
        }
        //pa_pImageBuffer += (pa_nFactor * pa_nXsize);
    }
}

// converts the image from yuv4:2:2 to rgb 32 bit format
void imageConvertYUVToRGB (unsigned long startAddressYUV, unsigned long startAddressRGB, unsigned short xWidth, unsigned short yWidth) {
	register unsigned char *memIndexYUV;
	register unsigned char yChannel;
	register unsigned long *memIndexRGB;
	register unsigned long count;
	register unsigned long i;
	register signed long int Red, Green, Blue;
	register unsigned char u = 0;
	register unsigned char v = 0;
	
	
	memIndexYUV = (unsigned char *)(startAddressYUV + 1);
	memIndexRGB = (unsigned long *)(startAddressRGB);
	
	count = (unsigned long)((xWidth * yWidth) / 2);
	
	for (i=0; i<count; i++) {
		yChannel = *memIndexYUV;
		v = *(memIndexYUV+1);
		u = *(memIndexYUV-1);

		Red = v - 128 + yChannel;
		if (Red > 255) Red = 255;
		if (Red < 0  ) Red = 0;
		Blue = u - 128 + yChannel;
		if (Blue > 255) Blue = 255;
		if (Blue < 0  ) Blue = 0;
		Green = (169 * yChannel - 52 * Red - 19 * Blue) / 100;
		if (Green > 255) Green = 255;
		if (Green < 0  ) Green = 0;
		*memIndexRGB = (Red << 16) + (Green << 8) + Blue;
		memIndexRGB ++;
	
		memIndexYUV+=2;
		yChannel = *memIndexYUV;
	
		Red = v - 128 + yChannel;
		if (Red > 255) Red = 255;
		if (Red < 0  ) Red = 0;
		Blue = u - 128 + yChannel;
		if (Blue > 255) Blue = 255;
		if (Blue < 0  ) Blue = 0;
		Green = (169 * yChannel - 52 * Red - 19 * Blue) / 100;
		if (Green > 255) Green = 255;
		if (Green < 0  ) Green = 0;

		*memIndexRGB = (Red << 16) + (Green << 8) + Blue;
		memIndexRGB ++;

		memIndexYUV+=2;
	}
}

void imageConvertYUVToRGB32Bit (unsigned long startAddressYUV, unsigned long startAddressRGB, unsigned short xWidth, unsigned short yWidth) {
	imageConvertYUVToRGB (startAddressYUV, startAddressRGB, xWidth, yWidth);
}

void imageConvertRGB565ToYUV422 (unsigned long *pa_pRGBbuffer, unsigned long *pa_pYUVbuffer, unsigned short pa_nXsize, unsigned short pa_nYsize) {
    register unsigned long nPixelCount = pa_nXsize * pa_nYsize;
    register unsigned long nRGBvalue;
    register signed short nBlue, nRed, nGreen;
    register unsigned char nY1, nY2;
    register signed short nU1, nV1, nU2, nV2;
    
    while (nPixelCount) {
        nRGBvalue = *pa_pRGBbuffer;
        nBlue = (nRGBvalue & 0x1f) << 3;
        nGreen = ((nRGBvalue >> 5) & 0x3f) << 2;
        nRed = ((nRGBvalue >> 11) & 0x1f) << 3;
        nY1 =(unsigned char)((1225 * nRed + 2404 * nGreen + 467 * nBlue) >> 12);
        nU1 = (signed short)((nBlue - nY1) + 128);
        nV1 = (signed short)((nRed - nY1) + 128);
        nBlue = ((nRGBvalue >> 16) & 0x1f) << 3;
        nGreen = ((nRGBvalue >> 21) & 0x3f) << 2;
        nRed = ((nRGBvalue >> 27) & 0x1f) << 3;
        nY2 =(unsigned char)((1225 * nRed + 2404 * nGreen + 467 * nBlue) >> 12);
        nU2 = (signed short)((nBlue - nY2) + 128);
        nV2 = (signed short)((nRed - nY2) + 128);
        nU1 = (nU1 + nU2) >> 1;
        nV1 = (nV1 + nV2) >> 1;
        *pa_pYUVbuffer = (nY2 << 24) | ((nV1 & 0xff) << 16) | (nY1 << 8) | (nU1 & 0xff);
        pa_pRGBbuffer ++;
        pa_pYUVbuffer ++;
        nPixelCount -= 2;
    }   
}

// image conversion, from imageConversion.c
void imageConvertRaw10BitOrder (unsigned long *pa_pRawBuffer, unsigned short pa_nRawXsize, unsigned short pa_nRawYsize) {
    register unsigned long nPixelCount = (pa_nRawXsize * pa_nRawYsize) >> 1;
    register unsigned long nValue;
    register unsigned short nValueHigh;
    register unsigned short nValueLow;
    
    
    while (nPixelCount) {
        nValue = *pa_pRawBuffer;
        nValueLow = nValue & 0xffff;
		nValueLow = ((nValueLow << 2) & 0x3fc) | ((nValueLow >> 9) & 0x0001)| ((nValueLow >> 7) & 0x0002);
        nValueHigh = (nValue >> 16) & 0xffff;
		nValueHigh = ((nValueHigh << 2) & 0x3fc) | ((nValueHigh >> 9) & 0x0001)| ((nValueHigh >> 7) & 0x0002);
		*pa_pRawBuffer = (nValueHigh << 16) | nValueLow;
        pa_pRawBuffer++;
        nPixelCount--;
    }
}

// creates a windows bmp compatible format from an rgb 24 bit image
unsigned long imageConvertRGBToWindowsBmp (unsigned short xstart, unsigned short ystart, unsigned short xlength, unsigned short ylength, unsigned long pa_nBuffer, unsigned long pa_nOutBuffer) {
	// creating bmp header
	char headerbyte[54] = {0};
	headerbyte[0]	= 'B';
	headerbyte[1]	= 'M';
	headerbyte[10]	= 54;
	headerbyte[11]	= 0;
	headerbyte[14]	= 40;
	headerbyte[26]	= 1;
	headerbyte[28]	= 24;
	headerbyte[38]	= 1;
	headerbyte[39]	= 126;
	headerbyte[40]	= 8;
	headerbyte[41]	= 48;
	headerbyte[42]	= 66;
	headerbyte[44]	= 100;
	headerbyte[47]	= 1;

	headerbyte[18]  = (char)(xlength & 0x00ff);
	headerbyte[19]  = (char)((xlength >> 8) & 0x00ff); 
	headerbyte[22]  = (char)(ylength & 0x00ff);
	headerbyte[23]  = (char)((ylength >> 8) & 0x00ff);

	unsigned short nRest = xlength % 4;
	unsigned long filesize = 54 + xlength * ylength * 3 + ylength * nRest;
		
	headerbyte[2] = (char)(filesize & 0xff);
	headerbyte[3] = (char)((filesize >> 8) & 0xff);
	headerbyte[4] = (char)((filesize >> 16) & 0xff);
	headerbyte[5] = (char)((filesize >> 24) & 0xff);

	// writing header to out buffer
	unsigned char *pcIndex = (unsigned char *)pa_nOutBuffer;
	unsigned short i = 0;
	while (i < 54) {
		*pcIndex++ = headerbyte[i++];
	}
		
	// writing image data to outbuffer
	unsigned short x = 0;
	unsigned short nDiff = 2 * xlength;
	unsigned long *memIndexRGB = (unsigned long *)(pa_nBuffer + (ylength - 1) * xlength * 4);
	unsigned long nColor = 0;
	while ((unsigned long)memIndexRGB >= pa_nBuffer) {
		x = 0;
		while(x<xlength) {
			nColor = *memIndexRGB++;
			*pcIndex++ = nColor & 0xff;
			*pcIndex++ = (nColor >> 8) & 0xff;
			*pcIndex++ = (nColor >> 16) & 0xff;
			x++;
		}
		i = 0;
		while (i++ < nRest) {
			*pcIndex++ = 0;
		}
		memIndexRGB -= nDiff;
	}		
	
	return filesize;
}

/**

	Converts an image of 10 bit bayer pattern into YUV422 (UYVY).
	Resolution of YUV image is half in x and y as RAW image.
	
**/
void imageConvertRaw10ToYUV422 (unsigned long *pa_pRawBuffer, unsigned long *pa_pYUVBuffer, unsigned short pa_nRawXsize, unsigned short pa_nRawYsize) {
	register signed short nX = 0;
	register signed short nY = 0;
	register signed short nRawXsize = pa_nRawXsize;
	register unsigned long nYUVValue = 0;
	register unsigned long nYvalue = 0;
	register unsigned long nRawValueUpperLine = 0;
	register unsigned long nRawValueLowerLine = 0;
	
	signed char U = 0;
	signed char V = 0;
	while (nY < pa_nRawYsize) {
		while (nX < pa_nRawXsize) {
			nRawValueUpperLine = *pa_pRawBuffer;
			nRawValueLowerLine = *(pa_pRawBuffer + nRawXsize / 2);			
			
			// calculate Y1
			nYvalue = (59 * ((((nRawValueUpperLine >> 16) & 0x0000ffff) +
			(nRawValueLowerLine & 0x0000ffff)) >> 1) +
			30 * (((nRawValueLowerLine >> 16) & 0x0000ffff)) +
			11 * (nRawValueUpperLine & 0x0000ffff)) / 400;
/*			nYvalue = (((((nRawValueUpperLine >> 16) & 0x0000ffff) +
						(nRawValueLowerLine & 0x0000ffff)) >> 3) & 0x000000ff);*/
			nYUVValue = nYvalue << 8;		// maybe 2 shifts not needed
			// calculate U=B-Y
			nYUVValue |= ((((nRawValueUpperLine & 0x0000ffff)>>2) - nYvalue) + 128)    & 0x000000ff;
			// calculate V=R-Y
			nYUVValue |= ((((((nRawValueLowerLine >> 16) & 0x0000ffff) >> 2) - nYvalue) + 128) << 16) & 0x00ff0000;
			// calculate Y1
			nRawValueUpperLine = *(pa_pRawBuffer + 1);
			nRawValueLowerLine = *(pa_pRawBuffer + 1 + nRawXsize / 2);			
					
			nYvalue = (59 * ((((nRawValueUpperLine >> 16) & 0x0000ffff) +
			(nRawValueLowerLine & 0x0000ffff)) >> 1) +
			30 * (((nRawValueLowerLine >> 16) & 0x0000ffff)) +
			11 * (nRawValueUpperLine & 0x0000ffff)) / 400;
			/*nYvalue = ((((nRawValueUpperLine >> 16) & 0x0000ffff) +
						(nRawValueLowerLine & 0x0000ffff)) >> 3) & 0x000000ff;*/
			nYUVValue |= nYvalue << 24;
			*pa_pYUVBuffer = nYUVValue;
			pa_pYUVBuffer++;
			pa_pRawBuffer += 2;
			nX += 4;	// 4 raw values processed
		}
		pa_pRawBuffer += nRawXsize / 2;
		nX = 0;
		nY += 2;	// 2 raw lines processed
	}
}



