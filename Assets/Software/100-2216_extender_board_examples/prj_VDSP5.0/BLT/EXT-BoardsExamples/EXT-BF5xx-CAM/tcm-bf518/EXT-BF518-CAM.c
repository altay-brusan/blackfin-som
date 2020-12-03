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
#include "../../../../../driver/src/common/PPIconfig.h"
#include "../../../../../driver/src/common/SCCBconfig.h"
#include "../../../../../driver/src/common/camera/OVxxxx/OVxxxx.h"
#include "../../../../../driver/src/common/mmio/PPIcameraIn/PPIcameraIn.h"
#include "../../../../../driver/src/common/camera/camera.h"
#include "../../../../../driver/src/common/pwrmngt/clockquery.h"

#include "../../../../../blacksheep/common/framegrabber/frameGrabber.h"
#include "../../../../../blacksheep/common/term/xmodemUART-buf.h"

#include <gpio_global.h>
#include <adi_ssl_init.h>

#pragma alignment_region (4)
static unsigned char s_cDevMngStorage[ADI_DEV_BASE_MEMORY + (ADI_DEV_DEVICE_MEMORY * 5)];
static unsigned char s_cIntMngStorage[ADI_INT_SECONDARY_MEMORY * BS_INT_MNG_NOF_SECONDARY_HANDLER * BS_NOF_USER_IVGS];                         ///< Pointer to an area of memory used by the interrupt manager
static u8 DMAMgrData[ADI_DMA_BASE_MEMORY + (ADI_DMA_CHANNEL_MEMORY * 4)];
#pragma alignment_region_end

ADI_DEV_MANAGER_HANDLE g_hADIDevMng;                     // ADI Device Manager Handle

unsigned long long cur[20];

unsigned short g_nReadIndex;

typedef struct {
	T_GPIO_MASK nSIO_C;
	T_GPIO_MASK nSIO_D;
	T_GPIO_MASK nPWDN;
	unsigned char cDevAddr;
} T_SCCB_ASSIGMENT;

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

// prototypes
//void PPIHandler(T_PPI_CAMERA_IN_CALLBACK_ARG pa_stBufferStatus);
void imageConvertYUVToRGB (unsigned long startAddressYUV, unsigned long startAddressRGB, unsigned short xWidth, unsigned short yWidth);
void imageConvertYUVToRGB32Bit (unsigned long startAddressYUV, unsigned long startAddressRGB, unsigned short xWidth, unsigned short yWidth);
void imageConvertRGB565ToYUV422 (unsigned long *pa_pRGBbuffer, unsigned long *pa_pYUVbuffer, unsigned short pa_nXsize, unsigned short pa_nYsize);
void imageConvertRaw10BitOrder (unsigned long *pa_pRawBuffer, unsigned short pa_nRawXsize, unsigned short pa_nRawYsize);
unsigned long imageConvertRGBToWindowsBmp (unsigned short xstart, unsigned short ystart, unsigned short xlength, unsigned short ylength, unsigned long pa_nBuffer, unsigned long pa_nOutBuffer);
void imageConvertRaw10ToYUV422 (unsigned long *pa_pRawBuffer, unsigned long *pa_pYUVBuffer, unsigned short pa_nRawXsize, unsigned short pa_nRawYsize);
void resizeImage(unsigned long *pa_pResizeBuffer, unsigned long *pa_pRGBbuffer, unsigned short pa_nXsize, unsigned short pa_nYsize, unsigned short pa_nFactor);


// memory for the adi interrupt manager
#pragma align(4)
static unsigned char intmgr_storage[(ADI_INT_SECONDARY_MEMORY * 8)];

void main (void) {
    
	
	T_ERROR_CODE erResult = ERR_NONE;
	char sMessage[255];
	unsigned long vco, cclk, sclk;
    bool bCAM1deactivate = false;
	T_SCCB_HANDLE hSCCBhndl1;
	ifrm_t *stFrameBufferCAM1;
	unsigned short nFactorCAM1;
	void *pRGBbufferCAM1 = 0;
	void *pResizeBufferCAM1 = 0;
	void *pBMPbufferCAM1 = 0;
	unsigned long nFileSizeCAM1 = 0;
	unsigned short i;

    // initialize the ADI Service System Libraries
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
	
	
	PPIcameraIn_setup();
	
	uart_writeString(hUART, "\xd\xa\xd\xa");
	
    T_SCCB_HANDLE hSCCBhndl;
    T_OVXXXX_CONF CamConfig;
    T_OVXXXX_HANDLE hCamera;
    
    T_PPI_CAMERA_IN_CONF DataIntConfig;
    DataIntConfig.nBufferMode = PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE;//PPI_CAMERA_IN_DOUBLE_BUFFER_NO_OVERWRITE;//PPI_CAMERA_IN_MULTI_FRAME_BUFFER_NO_OVERWRITE;
    T_PPI_CAMERA_IN_HANDLE hDataIntHnl;

    DataIntConfig.nMaxFrames = 2;   // double buffering
    DataIntConfig.cSDRAMsize = 0;

    CamConfig.nResolution = CAM_QVGA;
    CamConfig.fFramerate = 25;
    CamConfig.nCameraModel = OV264X;
    CamConfig.nColorMode = CAM_YUV_422;
    DataIntConfig.nPerIVG = OVXXXX_IVG_PERIPHERAL;
    DataIntConfig.nPerErrorIVG = OVXXXX_IVG_PERIPHERAL_ERROR;
    DataIntConfig.nDmaErrorIVG = OVXXXX_IVG_DMA_ERROR;
    DataIntConfig.fnCallback = NULL;
    DataIntConfig.pCallbackArg = NULL;
    DataIntConfig.fnErrorCallback = NULL;
    DataIntConfig.fnDmaErrorCallback = NULL;
    
    DataIntConfig.fnGetTimestamp = 0;//(PPI_CAMERA_IN_FN_GET_TIMESTAMP)getTimestamp;
    DataIntConfig.cPPI = 0;

    T_SCCB_ASSIGMENT tSCCB;
    tSCCB.nSIO_C = CAM_SIO_C1_PIN;
    tSCCB.nSIO_D = CAM_SIO_D1_PIN;
    tSCCB.nPWDN = CAM_PWDN1_PIN;

    CamConfig.nCameraInterface = OVT_SCCB;

    CamConfig.nResolution = DEFAULT_RESOLUTION_CAM1;
    CamConfig.nColorMode = DEFAULT_COLOR_MODE_CAM1;


    DataIntConfig.cPPI = 0;    
    CamConfig.nCameraModel = OV264X;
    

	//printf (" Open SCCB connection...");
    sccb_open(&hSCCBhndl, tSCCB.nSIO_C, tSCCB.nSIO_D, tSCCB.nPWDN,NULL, cclk);

	CamConfig.hCamInterfaceHandle = hSCCBhndl;
	hCamera = OVxxxx_Open(&CamConfig);

	//printf (" Open PPI%u...", DataIntConfig.cPPI);
	
	DataIntConfig.nDataInterfaceMode = CamConfig.nDataInterfaceMode;
	DataIntConfig.nColorMode = CamConfig.nColorMode;
	DataIntConfig.nXres = CamConfig.nXres;
	DataIntConfig.nYres = CamConfig.nYres;
	DataIntConfig.fFramerate = CamConfig.fFramerate;
	if(CamConfig.pHwDevCloseDev){
	    DataIntConfig.pHwDevCloseDev = CamConfig.pHwDevCloseDev;
	    DataIntConfig.hHwDevHndl = hCamera;
	}
	
	hDataIntHnl = PPIcameraIn_open(&DataIntConfig);
	
	//printf(" Give camera time to stabilize... Wait %d ms.\n");
	Sleep(1000);
	
    uart_writeString (hUART, "Give camera time to stabilize...");
	Sleep(5000);
	uart_writeString (hUART, "finished \xd\xa");
    if(!bCAM1deactivate) {
    	PPIcameraIn_start(hDataIntHnl);
    	uart_writeString(hUART, "CAM1: Start image capturing...\xd\xa");
    }

    uart_writeString (hUART, "Give camera time to capture frames...");
	Sleep(1000);                            // give camera time to capture frames
	uart_writeString (hUART, "finished \xd\xa");
//	Sleep(5000);
	
	unsigned short *pBufferTest[10];

    if(!bCAM1deactivate) {
        while(!PPIcameraIn_framesAvailable(hDataIntHnl)) {
            ;
        }
	
        stFrameBufferCAM1 = PPIcameraIn_getNextFrame(hDataIntHnl);
	
        PPIcameraIn_freeFrame(hDataIntHnl);
        PPIcameraIn_stop(hDataIntHnl);
    
    	sprintf (sMessage, "CAM1: image capturing finished\xd\xa");
    	uart_writeString (hUART, (unsigned char *)sMessage);
    }
	Sleep(100);
    if(!bCAM1deactivate) {
    	pRGBbufferCAM1 = malloc(DataIntConfig.nXres * DataIntConfig.nYres * 4);
	
    	if(!pRGBbufferCAM1) {
    	    uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	    while(1) {
    	        ;
    	    }
    	}
    	
/*    	if(stOVcamConfig1.nColorMode == CAM_RAW) {
    	    imageConvertRaw10BitOrder ((unsigned long *)stFrameBufferCAM1->pucFramePtr, DataIntConfig.nXres, DataIntConfig.nYres);
    	    imageConvertRaw10ToYUV422 ((unsigned long *)stFrameBufferCAM1->pucFramePtr, (unsigned long *)stFrameBufferCAM1->pucFramePtr, DataIntConfig.nXres, DataIntConfig.nYres);
    	    
    	    DataIntConfig.nXres = DataIntConfig.nXres / 2;
    	    DataIntConfig.nYres = DataIntConfig.nYres / 2;
    	} else if(stOVcamConfig1.nColorMode == CAM_RGB_565) {
    	    imageConvertRGB565ToYUV422 ((unsigned long *)stFrameBufferCAM1->pucFramePtr, (unsigned long *)stFrameBufferCAM1->pucFramePtr, DataIntConfig.nXres, DataIntConfig.nYres);
    	}
  */  	
    	uart_writeString(hUART, "CAM1: Converting to RGB32bit... ");
	
    	imageConvertYUVToRGB32Bit ((unsigned long)stFrameBufferCAM1->pucFramePtr, (unsigned long)pRGBbufferCAM1, DataIntConfig.nXres, DataIntConfig.nYres);
	
    	uart_writeString (hUART, "done\xd\xa");	
    }
    if(!bCAM1deactivate) {
    	nFactorCAM1 = DataIntConfig.nXres / RESIZE_X_VALUE_CAM1;
    	pResizeBufferCAM1;
	
    	if(nFactorCAM1 > 1) {
    	    uart_writeString(hUART, "CAM1: Resizing image... ");
    	    pResizeBufferCAM1 = malloc((DataIntConfig.nXres * DataIntConfig.nYres * 4) / nFactorCAM1);
	    
    	    if(!pResizeBufferCAM1) {
    	        uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	        while(1) {
    	            ;
    	        }
    	    }
	
    	    resizeImage(pResizeBufferCAM1, (unsigned long *)pRGBbufferCAM1, DataIntConfig.nXres, DataIntConfig.nYres, nFactorCAM1);
    	    DataIntConfig.nXres = DataIntConfig.nXres / nFactorCAM1;
    	    DataIntConfig.nYres = DataIntConfig.nYres / nFactorCAM1;
	    
    	    uart_writeString (hUART, "done\xd\xa");
	    
    	} else {
    	    pResizeBufferCAM1 = malloc((DataIntConfig.nXres * DataIntConfig.nYres * 4));
	    
    	    if(!pResizeBufferCAM1) {
    	        uart_writeString(hUART, "CAM1: ERROR! not enough memory for buffer allocating!\xd\xa");
    	        while(1) {
    	            ;
    	        }
    	    }
	    
    	    memcpy((void *)pResizeBufferCAM1, (void *)pRGBbufferCAM1, (DataIntConfig.nXres * DataIntConfig.nYres * 4));
    	}
	
    	free(pRGBbufferCAM1);
    	sprintf (sMessage, "CAM1: image size: %d x %d\xd\xa", DataIntConfig.nXres, DataIntConfig.nYres);
    	uart_writeString (hUART, (unsigned char *)sMessage);
    }
    if(!bCAM1deactivate) {
    	uart_writeString(hUART, "CAM1: Converting to Windows BMP file format... ");
	
    	pBMPbufferCAM1 = malloc((DataIntConfig.nXres * DataIntConfig.nYres * 4));
	
    	if(!pBMPbufferCAM1) {
    	    uart_writeString(hUART, "ERROR! not enough memory for buffer allocating!\xd\xa");
    	    while(1) {
    	        ;
    	    }
    	}
	
    	nFileSizeCAM1 = imageConvertRGBToWindowsBmp(0, 0, DataIntConfig.nXres, DataIntConfig.nYres, (unsigned long)pResizeBufferCAM1, (unsigned long)pBMPbufferCAM1);
    	free(pResizeBufferCAM1);
    	uart_writeString(hUART, "done\xd\xa");
    }
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
	
		sccb_close(hSCCBhndl);
    	free(pBMPbufferCAM1);
    	PPIcameraIn_close(hDataIntHnl);
    	OVxxxx_Close(hCamera);
    }
    
    PPIcameraIn_cleanup();
    
	while(1) {
	    ;
	}
}

// interrupt handler for PPI interrupt
/*void PPIHandler(T_PPI_CAMERA_IN_CALLBACK_ARG pa_stBufferStatus) {
	// set flag to signal end of image transfer+
	
}*/


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



