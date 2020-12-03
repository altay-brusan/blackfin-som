/**
	@file OVxxxx.c
	@ingroup camera
	@brief Driver for OmniVision Cameras
	
	
	
	BLT_DISCLAIMER(TBD)
	@author Alexander Froemel
	@version 1.0
	@date 13.07.2009
	
	@startcond Changelog
	
	@endcond
**/
#include <stdio.h>
#include <conio.h>
#include <cycle_count_bf.h>
#include "../../SCCBconfig.h"
#include "../../i2c/I2CMan.h"
#include "../../i2c/I2CEmu.h"
#include "OVxxxx.h"
#include "../camera.h"
#ifdef _USE_VDK_
	#include <VDK.h>
#endif


// supported camera modules
T_OVXXXX_CAMERA_TYPE g_tCamera[] = {
    {
        OMNIVISION,
        OVT_SCCB,
        OV_UNKNOWN,
        0x7660,
        0x42
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV263X,
        0x2631,
        0x60
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV263X,
        0x2633,
        0x60
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV264X,
        0x2641,
        0x60
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV264X,
        0x2642,
        0x60
    },
    {
        OMNIVISION,
        OVT_SCCB_16BIT,
        OV265X,
        0x2656,
        0x60,
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV_UNKNOWN,
        0x3610,
        0x60
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV_UNKNOWN,
        0x3620,
        0x60
    },
    {
        OMNIVISION,
        OVT_SCCB,
        OV_UNKNOWN,
        0x5611,
        0x60
    }        
};

// specifiy number of supported cameras
unsigned short g_nNofCams = sizeof(g_tCamera) / sizeof(T_OVXXXX_CAMERA_TYPE);


// methods to assign camera config arrays the appropriate default parameters
void OVxxxx_paramSet_30_QVGA_YUV_8(unsigned short *pa_pParameter);
void OVxxxx_paramSet_30_QVGA_RGB_16(unsigned short *pa_pParameter);
void OVxxxx_paramSet_15_QVGA_YUV_8(unsigned short *pa_pParameter);
void OVxxxx_paramSet_15_VGA_YUV_8(unsigned short *pa_pParameter);
void OVxxxx_paramSet_60_QVGA_YUV_8(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2630_SVGA_30fps_RAW10(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2630_UXGA_15fps_RAW10(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2630_Init(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_RAW10(unsigned short *pa_pParameter);	
unsigned short OVxxxx_paramSet_OV2641_RGB565(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_YUV(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_Init(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_SVGA_28fps_YUV(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_QVGA_25fps_YUV(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_SVGA_12_5fps_YUV(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2641_VGA_12_5fps_YUV(unsigned short *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2655_VGA_YUV422_25FPS(unsigned long *pa_pParameter);
unsigned short OVxxxx_paramSet_OV2655_SVGA_YUV422_25FPS(unsigned long *pa_pParameter);


T_OVXXXX_HANDLE OVxxxx_Open(T_OVXXXX_CONF *pa_pCameraConfig) {
    
    T_OVXXXX_SPEC *pCameraSpec = malloc(sizeof(T_OVXXXX_SPEC));
    
    if(pCameraSpec == NULL){
        pa_pCameraConfig->erError = ERR_OVXXXX_NOT_ENOUGH_MEMORY;
        return 0;
    }
    
    unsigned short pConfigParams[255];
    unsigned long pConfigParams16bit[512];
    unsigned short nNumParams = 0;
    unsigned short i;
    
    pa_pCameraConfig->erError = ERR_NONE;
    
    for(i=0; i<=g_nNofCams; i++) {
        if((i<g_nNofCams) && (g_tCamera[i].nCameraModel == pa_pCameraConfig->nCameraModel)) {
            pCameraSpec->cDeviceAddr = g_tCamera[i].cDeviceAddress;
            pCameraSpec->nCameraInterface = pa_pCameraConfig->nCameraInterface = g_tCamera[i].nInterfaceType;
            pCameraSpec->nCameraID = g_tCamera[i].nDeviceID;
            i=g_nNofCams + 1;
        }else if(i == g_nNofCams) {
            pa_pCameraConfig->erError = ERR_UNKNOWN_CAMERA;
            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
            return 0;
        }
    }
    
    pa_pCameraConfig->pHwDevCloseDev = (void *)OVxxxx_closeDevice;
    
    if((pa_pCameraConfig->hCamInterfaceHandle) != 0) {
        switch(pCameraSpec->nCameraInterface) {
            case OVT_SCCB: {
                
                pa_pCameraConfig->erError = sccb_writeByte8bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, 0xff, 0x01);
                pa_pCameraConfig->erError |= sccb_writeByte8bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_RESET, OVXXXX_MASK_RESET_BIT);
                
                if(pa_pCameraConfig->erError != ERR_NONE) {
                    pa_pCameraConfig->erError = ERR_CONFIG_CAM;
                    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                    return 0;
                }
                
                Sleep(300);
                
                unsigned short nCamID = 0;
                
                nCamID = sccb_readByte8bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_CAM_ID_HI, &(pa_pCameraConfig->erError)) << 8;
				nCamID |= sccb_readByte8bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_CAM_ID_LO, &(pa_pCameraConfig->erError));
				/*
				if(pa_pCameraConfig->nCameraID != nCamID) {
				    pa_pCameraConfig->erError = ERR_OVXXXX_WRONG_CAMERA_CONNECTED;
				    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
				    return 0;
				}
				*/
				
				pa_pCameraConfig->nCameraID = nCamID;
                break;
            }
            case OVT_SCCB_16BIT:{
                
                //pa_pCameraConfig->erError = sccb_writeByte16bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, 0xffff, 0x01);
                pa_pCameraConfig->erError |= sccb_writeByte16bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_RESET_16BIT, OVXXXX_MASK_RESET_BIT);
                
                if(pa_pCameraConfig->erError != ERR_NONE) {
                    pa_pCameraConfig->erError = ERR_CONFIG_CAM;
                    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                    return 0;
                }
                
                Sleep(300);
                
                unsigned short nCamID = 0;
                
                nCamID = sccb_readByte16bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_CAM_ID_HI_16BIT, &(pa_pCameraConfig->erError)) << 8;
				nCamID |= sccb_readByte16bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_CAM_ID_LO_16BIT, &(pa_pCameraConfig->erError));
				/*
				if(pa_pCameraConfig->nCameraID != nCamID) {
				    pa_pCameraConfig->erError = ERR_OVXXXX_WRONG_CAMERA_CONNECTED;
				    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
				    return 0;
				}
				*/
				
				pa_pCameraConfig->nCameraID = nCamID;
                break;
            }
/*            case OVT_I2C_16BIT: {
                T_OVXXXX_I2C_CONFIG_PARAMS tParam;
                tParam.cDevReg = 0xff;
                tParam.cDevRegValue[0] = 0xff;
                tParam.cDevRegValue[1] = 0x01;
                pa_pCameraConfig->erError = I2CwriteReg(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, tParam.cDevReg, tParam.cDevRegValue, OVXXXX_I2C_REGSIZE, NULL);
                tParam.cDevReg = OVXXXX_REG_RESET_I2C_HI;
                tParam.cDevRegValue[0] = OVXXXX_REG_RESET_I2C_LO;
                tParam.cDevRegValue[1] = OVXXXX_MASK_RESET_BIT;
                pa_pCameraConfig->erError |= I2CwriteReg(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, tParam.cDevReg, tParam.cDevRegValue, OVXXXX_I2C_REGSIZE, NULL);
                
                if(pa_pCameraConfig->erError != ERR_NONE) {
                    pa_pCameraConfig->erError = ERR_CONFIG_CAM;
                    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                    return 0;
                }
                
                Sleep(300);
                
                unsigned short nCamID = 0;
                
//                nCamID = I2CreadReg(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_CAM_ID_HI_16BIT, &(pa_pCameraConfig->erError)) << 8;
//				nCamID |= (I2CreadReg(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, OVXXXX_REG_CAM_ID_LO_16BIT, &(pa_pCameraConfig->erError)) & 0xff);
				
				pa_pCameraConfig->nCameraID = nCamID;
                break;
            }*/
            default: {
                pa_pCameraConfig->erError = ERR_OVXXXX_UNKNOWN_INTERFACE;
                OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                return 0;
            }
        }
    } else {
        pa_pCameraConfig->erError = ERR_OVXXXX_INVALID_INT_HNDL;
        OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
        return 0;
    }
    
    if(pa_pCameraConfig->nCameraID != 0xffff) {
        pa_pCameraConfig->erError = ERR_UNKNOWN_CAMERA;
        unsigned short i;
        for(i=0; i<g_nNofCams; i++) {
#ifdef OVXXXX_DEV_ID_IGNORE_LAST_NIBBLE
            if((pa_pCameraConfig->nCameraID & 0xfff0) == (g_tCamera[i].nDeviceID & 0xfff0)) {
                if(pa_pCameraConfig->nCameraModel == g_tCamera[i].nCameraModel) {
                    pa_pCameraConfig->erError = ERR_NONE;
                    i=g_nNofCams;
                }
            }
#else
            if(pa_pCameraConfig->nCameraID == g_tCamera[i].nDeviceID) {
                if(pa_pCameraConfig->nCameraModel == g_tCamera[i].nCameraModel) {
                    pa_pCameraConfig->erError = ERR_NONE;
                    i=g_nNofCams;
                }
            }
#endif            
        }
        
        if(pa_pCameraConfig->erError != ERR_NONE) {
            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
            return 0;
        }
    } else {
        pa_pCameraConfig->erError = ERR_NO_CAMERA_FOUND;
        OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
        return 0;
    }
    
    switch(pa_pCameraConfig->nCameraModel) {
        
        case OV264X: {                                                                              // camera OV2640
        
            nNumParams += OVxxxx_paramSet_OV2641_Init(pConfigParams);
            
            switch(pa_pCameraConfig->nColorMode) {
                case CAM_YUV_422: {
                    
                    pa_pCameraConfig->nDataInterfaceMode = CAM_8BIT_HVSYNC;
                    
                    switch(pa_pCameraConfig->nResolution) {
                        case CAM_QVGA: {
                             pa_pCameraConfig->nXres = 320;
                            pa_pCameraConfig->nYres = 240;
                            pa_pCameraConfig->fFramerate = 25;
                            nNumParams += OVxxxx_paramSet_OV2641_QVGA_25fps_YUV(&pConfigParams[nNumParams]);
                            break;
                        }
                        case CAM_VGA: {
                            pa_pCameraConfig->nXres = 640;
                            pa_pCameraConfig->nYres = 480;
                            pa_pCameraConfig->fFramerate = 12.5;
                            nNumParams += OVxxxx_paramSet_OV2641_VGA_12_5fps_YUV(&pConfigParams[nNumParams]);
                            break;
                        }
                        case CAM_SVGA: {
                            pa_pCameraConfig->nXres = 800;
                            pa_pCameraConfig->nYres = 600;
                            
                            if(pa_pCameraConfig->fFramerate < 20.0) {
                                pa_pCameraConfig->fFramerate = 12.5;
                                nNumParams += OVxxxx_paramSet_OV2641_SVGA_12_5fps_YUV(&pConfigParams[nNumParams]);
                            } else {
                                pa_pCameraConfig->fFramerate = 28;
                                nNumParams += OVxxxx_paramSet_OV2641_SVGA_28fps_YUV(&pConfigParams[nNumParams]);
                            }
                            
                            break;
                        }
                        case CAM_UXGA: {
                            pa_pCameraConfig->nXres = 1600;
                            pa_pCameraConfig->nYres = 1200;
                            pa_pCameraConfig->fFramerate = 7.5;
                            //nNumParams += OVxxxx_paramSet_OV2641_YUV(&pConfigParams[nNumParams]);
                            nNumParams += OVxxxx_paramSet_OV2641_RAW10(&pConfigParams[nNumParams]);
                            nNumParams += OVxxxx_paramSet_OV2641_YUV(&pConfigParams[nNumParams]);
                            break;
                        }
                        
                        default: {
                            pa_pCameraConfig->erError = ERR_OVXXXX_RESOLUTION_NOT_SUPPORTED;
                            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                            return 0;
                        }
                    }
                    
                    break;
                }
                
                case CAM_RGB_565: {
                    pa_pCameraConfig->nDataInterfaceMode = CAM_8BIT_HVSYNC;
                    
                    switch(pa_pCameraConfig->nResolution) {
                        case CAM_UXGA: {
                            pa_pCameraConfig->nXres = 1600;
                            pa_pCameraConfig->nYres = 1200;
                            pa_pCameraConfig->fFramerate = 7.5;
                            nNumParams += OVxxxx_paramSet_OV2641_RGB565(&pConfigParams[nNumParams]);
                            break;
                        }
                        
                        default: {
                            pa_pCameraConfig->erError = ERR_OVXXXX_RESOLUTION_NOT_SUPPORTED;
                            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                            return 0;
                        }
                    }
                    
                    break;
                }
                
                case CAM_RAW: {
                    pa_pCameraConfig->nDataInterfaceMode = CAM_10BIT_HVSYNC;
                    
                    switch(pa_pCameraConfig->nResolution) {
                        case CAM_UXGA: {
                            pa_pCameraConfig->nXres = 1600;
                            pa_pCameraConfig->nYres = 1200;
                            pa_pCameraConfig->fFramerate = 7.5;
                            nNumParams += OVxxxx_paramSet_OV2641_RAW10(&pConfigParams[nNumParams]);
                            break;
                        }
                        
                        default: {
                            pa_pCameraConfig->erError = ERR_OVXXXX_RESOLUTION_NOT_SUPPORTED;
                            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                            return 0;
                        }
                    }
                    
                    break;
                }
                    
                default : {
                    pa_pCameraConfig->erError = ERR_OVXXXX_COLOR_MODE_NOT_SUPPORTED;
                    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                    return 0;
                }
            }
            
            break;
        }
        
        case OV265X:{
            switch(pa_pCameraConfig->nColorMode){
                case CAM_YUV_422:{
                    
                    pa_pCameraConfig->nDataInterfaceMode = CAM_8BIT_HVSYNC;
                    
                    switch(pa_pCameraConfig->nResolution) {
/*                        case CAM_QVGA: {
                            pa_pCameraConfig->nXres = 320;
                            pa_pCameraConfig->nYres = 240;
                            
                            break;
                        }*/
                        case CAM_VGA: {
                            pa_pCameraConfig->nXres = 640;
                            pa_pCameraConfig->nYres = 480;
                            pa_pCameraConfig->fFramerate = 25;
                            nNumParams += OVxxxx_paramSet_OV2655_VGA_YUV422_25FPS(&pConfigParams16bit[nNumParams]);
                            
                            break;
                        }
                        case CAM_SVGA: {
                            pa_pCameraConfig->nXres = 800;
                            pa_pCameraConfig->nYres = 600;
                            pa_pCameraConfig->fFramerate = 25;
                            nNumParams += OVxxxx_paramSet_OV2655_SVGA_YUV422_25FPS(&pConfigParams16bit[nNumParams]);
                            
                            break;
                        }
/*                        case CAM_UXGA: {
                            pa_pCameraConfig->nXres = 1600;
                            pa_pCameraConfig->nYres = 1200;
                            
                            break;
                        }*/
                        
                        default: {
                            pa_pCameraConfig->erError = ERR_OVXXXX_RESOLUTION_NOT_SUPPORTED;
                            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                            return 0;
                        }
                    }
                    break;
                }
                default:{
                    pa_pCameraConfig->erError = ERR_OVXXXX_COLOR_MODE_NOT_SUPPORTED;
                    OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                    return 0;
                }
            }
            break;
        }
        
        default:{
            pa_pCameraConfig->erError = ERR_UNKNOWN_CAMERA;
            OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
            return 0;
        }
    }
    
    
    unsigned short nParamIndex = 0;
    unsigned short nAddress;
    
    while((pa_pCameraConfig->erError == ERR_NONE) && nParamIndex < nNumParams) {                      // configure the camera
        
        switch(pCameraSpec->nCameraInterface) {
            case OVT_SCCB: {
                nAddress = (unsigned short)((pConfigParams[nParamIndex] >> 8) & 0xffff);
                pa_pCameraConfig->erError = sccb_writeByte8bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, (unsigned char)nAddress, (unsigned char)(pConfigParams[nParamIndex] & 0xff));
                break;
            }
            case OVT_SCCB_16BIT: {
                nAddress = (unsigned short)((pConfigParams16bit[nParamIndex] >> 8) & 0xffff);
                pa_pCameraConfig->erError = sccb_writeByte16bit(pa_pCameraConfig->hCamInterfaceHandle, pCameraSpec->cDeviceAddr, nAddress, (unsigned char)(pConfigParams16bit[nParamIndex] & 0xff));
                break;
            }
            default: {
                pa_pCameraConfig->erError = ERR_OVXXXX_UNKNOWN_INTERFACE;
                OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
                return 0;
            }
        }
        
        nParamIndex++;
    }
    
    if(pa_pCameraConfig->erError != ERR_NONE) {
        pa_pCameraConfig->erError = ERR_CONFIG_CAM;
        OVxxxx_Close((T_OVXXXX_HANDLE)&pCameraSpec);
        return 0;
    }
    
    pCameraSpec->nCameraModel = pa_pCameraConfig->nCameraModel;
    pCameraSpec->nCameraID = pa_pCameraConfig->nCameraID;
    pCameraSpec->hCamInterfaceHandle = pa_pCameraConfig->hCamInterfaceHandle;
    
//    Sleep(2000);                                                                // give the camera time to stabilize
    
    return (T_OVXXXX_HANDLE)pCameraSpec;
}


void OVxxxx_Close(T_OVXXXX_HANDLE pa_Handle) {
    T_OVXXXX_SPEC *pCameraSpec = (T_OVXXXX_SPEC *)pa_Handle;
    //OVxxxx_PWRdown(pa_Handle);
/*    if(pCameraSpec->hCamInterfaceHandle) {
        sccb_close(pCameraSpec->hCamInterfaceHandle);   
    }
  */  
    free(pCameraSpec);
    
}


void OVxxxx_setup(){
    //nothing to do
}


void OVxxxx_cleanup(){
    //nothing to do
}


void OVxxxx_closeDevice(T_OVXXXX_HANDLE pa_Handle){
    OVxxxx_PWRdown(pa_Handle);
    OVxxxx_Close(pa_Handle);
}


void OVxxxx_PWRdown(T_OVXXXX_HANDLE pa_Handle) {
    T_OVXXXX_SPEC *pCameraSpec = (T_OVXXXX_SPEC *)pa_Handle;
    
    switch(pCameraSpec->nCameraModel) {
        
        case OV264X: {
            if(pCameraSpec->pwdnMask) {
                gpio_set(pCameraSpec->pwdnMask);
            }
            break;
        }
        
        default: {
            break;
        }
    }
    
}


void OVxxxx_PWRup(T_OVXXXX_HANDLE pa_Handle) {
    T_OVXXXX_SPEC *pCameraSpec = (T_OVXXXX_SPEC *)pa_Handle;
    
    switch(pCameraSpec->nCameraModel) {
        
        case OV264X: {
            if(pCameraSpec->pwdnMask) {
                gpio_clear(pCameraSpec->pwdnMask);
            }
            break;
        }
        
        default: {
            break;
        }
    }
    
}



T_ERROR_CODE OVxxxx_getCamera(T_CAM_MANUFACTURER pa_nManufacturer, T_OVXXXX_CAMERA_MODEL pa_nCamera, T_OVXXXX_CAMERA_TYPE *pa_nCamType) {
    unsigned char i;
    
    for(i=0; i<g_nNofCams; i++) {
        if(g_tCamera[i].nCameraModel == pa_nCamera && g_tCamera[i].nManufacturer == pa_nManufacturer) {
            *pa_nCamType = g_tCamera[i];
            return ERR_NONE;
        }
    }
    
    return ERR_GENERIC;
}


// methods to assign camera config arrays the appropriate default parameters

void OVxxxx_paramSet_30_QVGA_YUV_8(unsigned short *pa_pParameter) {
	pa_pParameter[0] = 0x03a4;
	pa_pParameter[1] = 0x03a4;
	pa_pParameter[2] = 0x0430;
	pa_pParameter[3] = 0x0588;
	pa_pParameter[4] = 0x0600;
	pa_pParameter[5] = 0x1100; 	//Clockdivider: beinflusst Framerate
	pa_pParameter[6] = 0x1214;	//0x1215; //0x1215;  //ITU656 Output Format
	pa_pParameter[7] = 0x13b3; 	//0x13b3;  // 13b3 ITU656 Enable/Disable
	pa_pParameter[8] = 0x1424;  //VGA / QVGA
	pa_pParameter[9] = 0x1505;	//0x1504;  //1544 Clock format, Data Format
	pa_pParameter[10] = 0x1f40;
	pa_pParameter[11] = 0x20c0;	//Edge enhancement
	pa_pParameter[12] = 0x23de;
	pa_pParameter[13] = 0x24a0;
	pa_pParameter[14] = 0x2580;
	pa_pParameter[15] = 0x2632;
	pa_pParameter[16] = 0x27e2;
	pa_pParameter[17] = 0x2800; //Interleaced/progressive
	pa_pParameter[18] = 0x2a11;	//0x2a13;
	pa_pParameter[19] = 0x2b00;
	pa_pParameter[20] = 0x2d15; //ABE
	pa_pParameter[21] = 0x2f9c;
	pa_pParameter[22] = 0x3000;
	pa_pParameter[23] = 0x31c4;
	pa_pParameter[24] = 0x6086;
	pa_pParameter[25] = 0x61e0;
	pa_pParameter[26] = 0x6288;
	pa_pParameter[27] = 0x6311;
	pa_pParameter[28] = 0x6489;
	pa_pParameter[29] = 0x6500;
	pa_pParameter[30] = 0x6714;
	pa_pParameter[31] = 0x687a;
	pa_pParameter[32] = 0x6904;
	pa_pParameter[33] = 0x6c11;
	pa_pParameter[34] = 0x6d33;
	pa_pParameter[35] = 0x6e22;
	pa_pParameter[36] = 0x6f00;
	pa_pParameter[37] = 0x7180;
	pa_pParameter[38] = 0x7420;
	pa_pParameter[39] = 0x750e;
	pa_pParameter[40] = 0x7602;
	pa_pParameter[41] = 0x77c4;
	pa_pParameter[42] = 0xffff;
}

void OVxxxx_paramSet_30_QVGA_RGB_16(unsigned short *pa_pParameter) {
	pa_pParameter[0] = 0x03a4;
	pa_pParameter[1] = 0x03a4;
	pa_pParameter[2] = 0x0430;
	pa_pParameter[3] = 0x0588;
	pa_pParameter[4] = 0x0660;
	pa_pParameter[5] = 0x1100; 	//Clockdivider: beinflusst Framerate
	pa_pParameter[6] = 0x120d;	//0x1215; //0x1215;  //ITU656 Output Format
	pa_pParameter[7] = 0x13b3; 	//0x13b3;  // 13b3 ITU656 Enable/Disable
	pa_pParameter[8] = 0x1424;  //VGA / QVGA
	pa_pParameter[9] = 0x1504;	//0x1504;  //1544 Clock format, Data Format
	pa_pParameter[10] = 0x1f51;
	pa_pParameter[11] = 0x20d0;	//Edge enhancement
	pa_pParameter[12] = 0x23de;
	pa_pParameter[13] = 0x2480;
	pa_pParameter[14] = 0x2560;
	pa_pParameter[15] = 0x26c2;
	pa_pParameter[16] = 0x27e2;
	pa_pParameter[17] = 0x2800; //Interleaced/progressive
	pa_pParameter[18] = 0x2a11;	//Night Mode [2]
	pa_pParameter[19] = 0x2b00;
	pa_pParameter[20] = 0x2d05; //ABE [4]
	pa_pParameter[21] = 0x2f9c;
	pa_pParameter[22] = 0x3000;
	pa_pParameter[23] = 0x31c4;
	pa_pParameter[24] = 0x60a6;
	pa_pParameter[25] = 0x6168;
	pa_pParameter[26] = 0x6288;
	pa_pParameter[27] = 0x6311;
	pa_pParameter[28] = 0x6488;
	pa_pParameter[29] = 0x6500;
	pa_pParameter[30] = 0x6794;
	pa_pParameter[31] = 0x687a;
	pa_pParameter[32] = 0x6904;
	pa_pParameter[33] = 0x6c11;
	pa_pParameter[34] = 0x6d33;
	pa_pParameter[35] = 0x6e22;
	pa_pParameter[36] = 0x6f00;
	pa_pParameter[37] = 0x7420;
	pa_pParameter[38] = 0x750e;
	pa_pParameter[39] = 0x77c4;
	pa_pParameter[40] = 0xffff;
}

void OVxxxx_paramSet_15_QVGA_YUV_8(unsigned short *pa_pParameter) {
	pa_pParameter[0]	= 0x1210;
	pa_pParameter[1]	= 0x1210;
	pa_pParameter[2]	= 0x1181;
	pa_pParameter[3]	= 0x142a;
	pa_pParameter[4]	= 0x1502;
	pa_pParameter[5]	= 0x0441;
	pa_pParameter[6]	= 0x3300;
	pa_pParameter[7]	= 0x3407;
	pa_pParameter[8]	= 0x3d19;
	pa_pParameter[9]	= 0x6b0a;
	pa_pParameter[10]	= 0x2930;
	pa_pParameter[11]	= 0x3584;
	pa_pParameter[12]	= 0x3600;
	pa_pParameter[13]	= 0x3943;
	pa_pParameter[14]	= 0x3a0c;
	pa_pParameter[15]	= 0x3c6c;
	pa_pParameter[16]	= 0x8dcf;
	pa_pParameter[17]	= 0x4001;
	pa_pParameter[18]	= 0x0f6f;
	pa_pParameter[19]	= 0x13ff;
	pa_pParameter[20]	= 0x1e01;
	pa_pParameter[21]	= 0x1b01;
	pa_pParameter[22]	= 0x184b;
	pa_pParameter[23]	= 0x1723;
	pa_pParameter[24]	= 0x32bf;
	pa_pParameter[25]	= 0x1902;
	pa_pParameter[26]	= 0x1a7a;
	pa_pParameter[27]	= 0x2a00;
	pa_pParameter[28]	= 0x2b76;
	pa_pParameter[29]	= 0x4102;	//0x4122 for activating edge enhancment.
	pa_pParameter[30]	= 0x3f41;
	pa_pParameter[31]	= 0x9f9d;
	pa_pParameter[32]	= 0x6003;
	pa_pParameter[33]	= 0x3d99;
	pa_pParameter[34]	= 0xa108;
	pa_pParameter[35]	= 0x2468;
	pa_pParameter[36]	= 0x2558;
	pa_pParameter[37]	= 0x6980;
	pa_pParameter[38]	= 0x5cb9;
	pa_pParameter[39]	= 0x5d9b;
	pa_pParameter[40]	= 0x5e10;
	pa_pParameter[41]	= 0x59ba;
	pa_pParameter[42]	= 0x5a9a;
	pa_pParameter[43]	= 0x5b22;
	pa_pParameter[44]	= 0x43f0;
	pa_pParameter[45]	= 0x4410;
	pa_pParameter[46]	= 0x4578;
	pa_pParameter[47]	= 0x46a8;
	pa_pParameter[48]	= 0x4760;
	pa_pParameter[49]	= 0x4880;
	pa_pParameter[50]	= 0x5ff0;
	pa_pParameter[51]	= 0x6005;
	pa_pParameter[52]	= 0x6160;
	pa_pParameter[53]	= 0x9f5d;
	pa_pParameter[54]	= 0xa020;
	pa_pParameter[55]	= 0x4f46;
	pa_pParameter[56]	= 0x5036;
	pa_pParameter[57]	= 0x510f;
	pa_pParameter[58]	= 0x5217;
	pa_pParameter[59]	= 0x537f;
	pa_pParameter[60]	= 0x5496;
	pa_pParameter[61]	= 0x6c40;
	pa_pParameter[62]	= 0x6d30;
	pa_pParameter[63]	= 0x6e4b;			
	pa_pParameter[64]	= 0x6f60;
	pa_pParameter[65]	= 0x7070;
	pa_pParameter[66]	= 0x7170;
	pa_pParameter[67]	= 0x7270;
	pa_pParameter[68]	= 0x7370;
	pa_pParameter[69]	= 0x7460;
	pa_pParameter[70]	= 0x7560;
	pa_pParameter[71]	= 0x7650;
	pa_pParameter[72]	= 0x7748;
	pa_pParameter[73]	= 0x783a;
	pa_pParameter[74]	= 0x792e;
	pa_pParameter[75]	= 0x7a28;
	pa_pParameter[76]	= 0x7b22;
	pa_pParameter[77]	= 0x7c04;
	pa_pParameter[78]	= 0x7d07;
	pa_pParameter[79]	= 0x7e10;
	pa_pParameter[80]	= 0x7f28;
	pa_pParameter[81]	= 0x8036;
	pa_pParameter[82]	= 0x8144;
	pa_pParameter[83]	= 0x8252;
	pa_pParameter[84]	= 0x8360;
	pa_pParameter[85]	= 0x846c;
	pa_pParameter[86]	= 0x8578;
	pa_pParameter[87]	= 0x868c;
	pa_pParameter[88]	= 0x879e;
	pa_pParameter[89]	= 0x88bb;
	pa_pParameter[90]	= 0x89d2;
	pa_pParameter[91]	= 0x8ae6;
	pa_pParameter[92]	= 0x0e04;
	pa_pParameter[93]	= 0x1602;
	pa_pParameter[94]	= 0x3b0a;
	pa_pParameter[95]	= 0x8bcc;
	pa_pParameter[96]	= 0x8ccf;
	pa_pParameter[97]	= 0x9200;
	pa_pParameter[98]	= 0x9300;
	pa_pParameter[99]	= 0x2728;
	pa_pParameter[100]= 0x2828;
	pa_pParameter[101]= 0x2c28;
	pa_pParameter[102]= 0xffff;	
}

void OVxxxx_paramSet_15_VGA_YUV_8(unsigned short *pa_pParameter) {
	pa_pParameter[0]	= 0x1200; 
	pa_pParameter[1]	= 0x1200; 
	pa_pParameter[2]	= 0x1181;
	pa_pParameter[3]	= 0x142a;
	pa_pParameter[4]	= 0x1500;
	pa_pParameter[5]	= 0x0440;
	pa_pParameter[6]	= 0x3300;
	pa_pParameter[7]	= 0x3407; 
	pa_pParameter[8]	= 0x3d19; 
	pa_pParameter[9]	= 0x6b0a; 
	pa_pParameter[10]	= 0x2930; 
	pa_pParameter[11]	= 0x3584; 
	pa_pParameter[12]	= 0x3600; 
	pa_pParameter[13]	= 0x3943;
	pa_pParameter[14]	= 0x3a0c;
	pa_pParameter[15]	= 0x3c6c; 
	pa_pParameter[16]	= 0x8dcf;
	pa_pParameter[17]	= 0x4080;
	pa_pParameter[18]	= 0x0f6f; 
	pa_pParameter[19] = 0x13ff; 
	pa_pParameter[20]	= 0x1e01; 
	pa_pParameter[21]	= 0x1b01; 
	pa_pParameter[22]	= 0x1801; 
	pa_pParameter[23]	= 0x1713; 
	pa_pParameter[24]	= 0x3292;
	pa_pParameter[25]	= 0x1902;
	pa_pParameter[26]	= 0x1a7a; 
	pa_pParameter[27]	= 0x4120;	//0x4122; for activating edge enhancment
	pa_pParameter[28]	= 0x3f41; 
	pa_pParameter[29]	= 0x9f9d; 
	pa_pParameter[30]	= 0x6003; 
	pa_pParameter[31]	= 0x3d99; 
	pa_pParameter[32]	= 0xa108; 
	pa_pParameter[33]	= 0x2468; 
	pa_pParameter[34]	= 0x2558; 
	pa_pParameter[35]	= 0x6980; 
	pa_pParameter[36]	= 0x5cb9; 
	pa_pParameter[37]	= 0x5d9b; 
	pa_pParameter[38]	= 0x5e10; 
	pa_pParameter[39]	= 0x59ba; 
	pa_pParameter[40]	= 0x5a9a; 
	pa_pParameter[41]	= 0x5b22; 
	pa_pParameter[42]	= 0x43f0; 
	pa_pParameter[43]	= 0x4410; 
	pa_pParameter[44]	= 0x4578; 
	pa_pParameter[45]	= 0x46a8;
	pa_pParameter[46]	= 0x4760; 
	pa_pParameter[47]	= 0x4880; 
	pa_pParameter[48]	= 0x5ff0; 
	pa_pParameter[49]	= 0x6005; 
	pa_pParameter[50]	= 0x6160; 
	pa_pParameter[51]	= 0x9f5d; 
	pa_pParameter[52]	= 0xa020; 
	pa_pParameter[53]	= 0x4f46; 
	pa_pParameter[54]	= 0x5036; 
	pa_pParameter[55]	= 0x510f;
	pa_pParameter[56]	= 0x5217; 
	pa_pParameter[57]	= 0x537f; 
	pa_pParameter[58]	= 0x5496; 
	pa_pParameter[59]	= 0x6c40; 
	pa_pParameter[60]	= 0x6d30; 
	pa_pParameter[61]	= 0x6e4b;
	pa_pParameter[62]	= 0x6f60; 
	pa_pParameter[63]	= 0x7070; 
	pa_pParameter[64]	= 0x7170; 
	pa_pParameter[65]	= 0x7270; 
	pa_pParameter[66]	= 0x7370; 
	pa_pParameter[67]	= 0x7460; 
	pa_pParameter[68]	= 0x7560; 
	pa_pParameter[69]	= 0x7650; 
	pa_pParameter[70]	= 0x7748; 
	pa_pParameter[71]	= 0x783a; 
	pa_pParameter[72]	= 0x792e; 
	pa_pParameter[73]	= 0x7a28; 
	pa_pParameter[74]	= 0x7b22; 
	pa_pParameter[75]	= 0x7c04; 
	pa_pParameter[76]	= 0x7d07; 
	pa_pParameter[77]	= 0x7e10; 
	pa_pParameter[78]	= 0x7f28; 
	pa_pParameter[79]	= 0x8036; 
	pa_pParameter[80]	= 0x8144; 
	pa_pParameter[81]	= 0x8252; 
	pa_pParameter[82]	= 0x8360;
	pa_pParameter[83]	= 0x846c; 
	pa_pParameter[84]	= 0x8578; 
	pa_pParameter[85]	= 0x868c; 
	pa_pParameter[86]	= 0x879e; 
	pa_pParameter[87]	= 0x88bb; 
	pa_pParameter[88]	= 0x89d2; 
	pa_pParameter[89]	= 0x8ae6; 
	pa_pParameter[90]	= 0x0e04; 
	pa_pParameter[91]	= 0x1602;
	pa_pParameter[92]	= 0x3b02; 	//02 for 60Hz, 0a for 50Hz 
	pa_pParameter[93]	= 0x8bcc; 
	pa_pParameter[94]	= 0x8ccf;
	pa_pParameter[95]	= 0x9200;
	pa_pParameter[96]	= 0x9300; 
	pa_pParameter[97]	= 0x2728; 
	pa_pParameter[98]	= 0x2828; 
	pa_pParameter[99]	= 0x2c28;
	pa_pParameter[100]	= 0xffff;
}

void OVxxxx_paramSet_60_QVGA_YUV_8(unsigned short *pa_pParameter) {
	pa_pParameter[0]	= 0x1280; 
	pa_pParameter[1]	= 0x1210; 
	pa_pParameter[2]	= 0x1180; //80 for 60 fps
	pa_pParameter[3]	= 0x142a;
	pa_pParameter[4]	= 0x1502;
	pa_pParameter[5]	= 0x0441; //Enable ITU format (set bit 6)
	pa_pParameter[6]	= 0x3300; 
	pa_pParameter[7]	= 0x3407; 
	pa_pParameter[8]	= 0x3d19; 
	pa_pParameter[9]	= 0x6b0a; 
	pa_pParameter[10]	= 0x2930; 
	pa_pParameter[11]	= 0x3584; 
	pa_pParameter[12]	= 0x3600; 
	pa_pParameter[13]	= 0x3943;
	pa_pParameter[14]	= 0x3a0c; //Output sequence UYVY 
	pa_pParameter[15]	= 0x3c6c; 
	pa_pParameter[16]	= 0x8dcf; 
	pa_pParameter[17]	= 0x4081; //Set output range to 10-f0, for 01-fe set to 81
	pa_pParameter[18]	= 0x0f6f; 
	pa_pParameter[19]	= 0x13ff; 
	pa_pParameter[20]	= 0x1e01; 
	pa_pParameter[21]	= 0x1b01; 
	pa_pParameter[22]	= 0x184b; 
	pa_pParameter[23]	= 0x1723; 
	pa_pParameter[24]	= 0x32bf;
	pa_pParameter[25]	= 0x1902;
	pa_pParameter[26]	= 0x1a7a;
	pa_pParameter[27]	= 0x2a00; //Adjust for 27 MHz clock
	pa_pParameter[28]	= 0x2b76; //Adjust for 27 Mhz clock
	pa_pParameter[29]	= 0x4122; 
	pa_pParameter[30]	= 0x3f41; 
	pa_pParameter[31]	= 0x9f9d; 
	pa_pParameter[32]	= 0x6003; 
	pa_pParameter[33]	= 0x3d99; 
	pa_pParameter[34]	= 0xa108; 
	pa_pParameter[35]	= 0x2468; 
	pa_pParameter[36]	= 0x2558; 
	pa_pParameter[37]	= 0x6980; 
	pa_pParameter[38]	= 0x5cb9; 
	pa_pParameter[39]	= 0x5d9b; 
	pa_pParameter[40]	= 0x5e10; 
	pa_pParameter[41]	= 0x59ba; 
	pa_pParameter[42]	= 0x5a9a; 
	pa_pParameter[43]	= 0x5b22; 
	pa_pParameter[44]	= 0x43f0; 
	pa_pParameter[45]	= 0x4410; 
	pa_pParameter[46]	= 0x4578; 
	pa_pParameter[47]	= 0x46a8; 
	pa_pParameter[48]	= 0x4760; 
	pa_pParameter[49]	= 0x4880; 
	pa_pParameter[50]	= 0x5ff0; 
	pa_pParameter[51]	= 0x6005; 
	pa_pParameter[52]	= 0x6160; 
	pa_pParameter[53]	= 0x9f5d; 
	pa_pParameter[54]	= 0xa020; 
	pa_pParameter[55]	= 0x4f46; 
	pa_pParameter[56]	= 0x5036; 
	pa_pParameter[57]	= 0x510f; 
	pa_pParameter[58]	= 0x5217; 
	pa_pParameter[59]	= 0x537f; 
	pa_pParameter[60]	= 0x5496; 
	pa_pParameter[61]	= 0x6c40; 
	pa_pParameter[62]	= 0x6d30; 
	pa_pParameter[63]	= 0x6e4b; 
	pa_pParameter[64]	= 0x6f60; 
	pa_pParameter[65]	= 0x7070; 
	pa_pParameter[66]	= 0x7170; 
	pa_pParameter[67]	= 0x7270; 
	pa_pParameter[68]	= 0x7370; 
	pa_pParameter[69]	= 0x7460;
	pa_pParameter[70]	= 0x7560; 
	pa_pParameter[71]	= 0x7650; 
	pa_pParameter[72]	= 0x7748; 
	pa_pParameter[73]	= 0x783a; 
	pa_pParameter[74]	= 0x792e; 
	pa_pParameter[75]	= 0x7a28; 
	pa_pParameter[76]	= 0x7b22; 
	pa_pParameter[77]	= 0x7c04; 
	pa_pParameter[78]	= 0x7d07; 
	pa_pParameter[79]	= 0x7e10; 
	pa_pParameter[80]	= 0x7f28; 
	pa_pParameter[81]	= 0x8036; 
	pa_pParameter[82]	= 0x8144; 
	pa_pParameter[83]	= 0x8252; 
	pa_pParameter[84]	= 0x8360; 
	pa_pParameter[85]	= 0x846c; 
	pa_pParameter[86]	= 0x8578; 
	pa_pParameter[87]	= 0x868c; 
	pa_pParameter[88]	= 0x879e; 
	pa_pParameter[89]	= 0x88bb; 
	pa_pParameter[90]	= 0x89d2; 
	pa_pParameter[91]	= 0x8ae6; 
	pa_pParameter[92]	= 0x0e04; 
	pa_pParameter[93]	= 0x1602; 
	pa_pParameter[94]	= 0x3b0a; //02 for 60Hz, 0a for 50Hz 
	pa_pParameter[95]	= 0x8bcc; 
	pa_pParameter[96]	= 0x8ccf;
	pa_pParameter[97]	= 0x9200;
	pa_pParameter[98]	= 0x9300; 
	pa_pParameter[99]	= 0x2728; 
	pa_pParameter[100] 	= 0x2828; 
	pa_pParameter[101] 	= 0x2c28;
	pa_pParameter[101] 	= 0xffff;
}

unsigned short OVxxxx_paramSet_OV2641_Init(unsigned short *pa_pParameter) {
	pa_pParameter[0] 	= 0xff00;
	pa_pParameter[1] 	= 0x2cff;
	pa_pParameter[2] 	= 0x2edf;
	pa_pParameter[3] 	= 0xff01;
	pa_pParameter[4] 	= 0x3c32;
	pa_pParameter[5] 	= 0x1100;
	pa_pParameter[6] 	= 0x0902;
	pa_pParameter[7] 	= 0x0428;
	pa_pParameter[8] 	= 0x13e5;
	pa_pParameter[9] 	= 0x1448;
	pa_pParameter[10] 	= 0x2c0c;
	pa_pParameter[11] 	= 0x3378;
	pa_pParameter[12] 	= 0x3a33;
	pa_pParameter[13] 	= 0x3bfb;
	pa_pParameter[14] 	= 0x3e00;
	pa_pParameter[15] 	= 0x4311;
	pa_pParameter[16] 	= 0x1610;
	pa_pParameter[17] 	= 0x3902;
	pa_pParameter[18] 	= 0x3588;
	pa_pParameter[19] 	= 0x220a;
	pa_pParameter[20] 	= 0x3740;
	pa_pParameter[21] 	= 0x2300;
	pa_pParameter[22] 	= 0x34a0;
	pa_pParameter[23] 	= 0x361a;
	pa_pParameter[24] 	= 0x0602;
	pa_pParameter[25] 	= 0x07c0;
	pa_pParameter[26] 	= 0x0db7;
	pa_pParameter[27] 	= 0x0e01;
	pa_pParameter[28] 	= 0x4c00;
	pa_pParameter[29] 	= 0x4a81;
	pa_pParameter[30] 	= 0x2199;
	pa_pParameter[31] 	= 0x243a;
	pa_pParameter[32] 	= 0x2532;
	pa_pParameter[33] 	= 0x2682;
	pa_pParameter[34] 	= 0x5c00;
	pa_pParameter[35] 	= 0x6300;
	pa_pParameter[36] 	= 0x5d55;
	pa_pParameter[37] 	= 0x5e7d;
	pa_pParameter[38] 	= 0x5f7d;
	pa_pParameter[39] 	= 0x6055;
	pa_pParameter[40] 	= 0x6170;
	pa_pParameter[41] 	= 0x6280;
	pa_pParameter[42] 	= 0x7c05;
	pa_pParameter[43] 	= 0x2080;
	pa_pParameter[44] 	= 0x2830;
	pa_pParameter[45] 	= 0x6c00;
	pa_pParameter[46] 	= 0x6d80;
	pa_pParameter[47] 	= 0x6e00;
	pa_pParameter[48] 	= 0x7002;
	pa_pParameter[49] 	= 0x7194;
	pa_pParameter[50] 	= 0x73c1;
	pa_pParameter[51] 	= 0x3d34;
	pa_pParameter[52] 	= 0x5a57;
	pa_pParameter[53] 	= 0x4fbb;
	pa_pParameter[54] 	= 0x509c;
	pa_pParameter[55] 	= 0xff00;
	pa_pParameter[56] 	= 0xe57f;
	pa_pParameter[57] 	= 0xf9c0;
	pa_pParameter[58] 	= 0x4124;
	pa_pParameter[59] 	= 0xe014;
	pa_pParameter[60] 	= 0x76ff;
	pa_pParameter[61] 	= 0x33a0;
	pa_pParameter[62] 	= 0x4220;
	pa_pParameter[63] 	= 0x4318;
	pa_pParameter[64] 	= 0x4c00;
	pa_pParameter[65] 	= 0x87d0;
	pa_pParameter[66] 	= 0x883f;
	pa_pParameter[67] 	= 0xd703;
	pa_pParameter[68] 	= 0xd910;
	pa_pParameter[69] 	= 0xd382;
	pa_pParameter[70] 	= 0xc808;
	pa_pParameter[71] 	= 0xc980;
	pa_pParameter[72] 	= 0x7c00;
	pa_pParameter[73] 	= 0x7d02;
	pa_pParameter[74] 	= 0x7c03;
	pa_pParameter[75] 	= 0x7d48;
	pa_pParameter[76] 	= 0x7d48;
	pa_pParameter[77] 	= 0x7c08;
	pa_pParameter[78] 	= 0x7d20;
	pa_pParameter[79] 	= 0x7d10;
	pa_pParameter[80] 	= 0x7d0e;
	pa_pParameter[81] 	= 0x9000;
	pa_pParameter[82] 	= 0x910e;
	pa_pParameter[83] 	= 0x911a;
	pa_pParameter[84] 	= 0x9131;
	pa_pParameter[85] 	= 0x915a;
	pa_pParameter[86] 	= 0x9169;
	pa_pParameter[87] 	= 0x9175;
	pa_pParameter[88] 	= 0x917e;
	pa_pParameter[89] 	= 0x9188;
	pa_pParameter[90] 	= 0x918f;
	pa_pParameter[91] 	= 0x9196;
	pa_pParameter[92] 	= 0x91a3;
	pa_pParameter[93] 	= 0x91af;
	pa_pParameter[94] 	= 0x91c4;
	pa_pParameter[95] 	= 0x91d7;
	pa_pParameter[96] 	= 0x91e8;
	pa_pParameter[97] 	= 0x9120;
	pa_pParameter[98] 	= 0x9200;
	pa_pParameter[99] 	= 0x9306;
	pa_pParameter[100] 	= 0x93e3;
	pa_pParameter[101] 	= 0x9305;
	pa_pParameter[102] 	= 0x9305;
	pa_pParameter[103] 	= 0x9300;
	pa_pParameter[104] 	= 0x9302;
	pa_pParameter[105] 	= 0x9300;
	pa_pParameter[106] 	= 0x9300;
	pa_pParameter[107] 	= 0x9300;
	pa_pParameter[108] 	= 0x9300;
	pa_pParameter[109] 	= 0x9300;
	pa_pParameter[110] 	= 0x9300;
	pa_pParameter[111] 	= 0x9300;
	pa_pParameter[112] 	= 0x9600;
	pa_pParameter[113] 	= 0x9708;
	pa_pParameter[114] 	= 0x9719;
	pa_pParameter[115] 	= 0x9702;
	pa_pParameter[116] 	= 0x970c;
	pa_pParameter[117] 	= 0x9724;
	pa_pParameter[118] 	= 0x9730;
	pa_pParameter[119] 	= 0x9728;
	pa_pParameter[120] 	= 0x9726;
	pa_pParameter[121] 	= 0x9702;
	pa_pParameter[122] 	= 0x9798;
	pa_pParameter[123] 	= 0x9780;
	pa_pParameter[124] 	= 0x9700;
	pa_pParameter[125] 	= 0x9700;
	pa_pParameter[126] 	= 0xc3ed;
	pa_pParameter[127] 	= 0xa400;
	pa_pParameter[128] 	= 0xa800;
	pa_pParameter[129] 	= 0xc511;
	pa_pParameter[130] 	= 0xc651;
	pa_pParameter[131] 	= 0xbf80;
	pa_pParameter[132] 	= 0xc710;
	pa_pParameter[133] 	= 0xb666;
	pa_pParameter[134] 	= 0xb8a5;
	pa_pParameter[135] 	= 0xb764;
	pa_pParameter[136] 	= 0xb97c;
	pa_pParameter[137] 	= 0xb3af;
	pa_pParameter[138] 	= 0xb497;
	pa_pParameter[139] 	= 0xb5ff;
	pa_pParameter[140] 	= 0xb0c5;
	pa_pParameter[141] 	= 0xb194;
	pa_pParameter[142] 	= 0xb20f;
	pa_pParameter[143] 	= 0xc45c;
	pa_pParameter[144] 	= 0xc0c8;
	pa_pParameter[145] 	= 0xc196;
	pa_pParameter[146] 	= 0x861d;
	pa_pParameter[147] 	= 0x5000;
	pa_pParameter[148] 	= 0x5190;
	pa_pParameter[149] 	= 0x5218;
	pa_pParameter[150] 	= 0x5300;
	pa_pParameter[151] 	= 0x5400;
	pa_pParameter[152] 	= 0x5588;
	pa_pParameter[153] 	= 0x5700;
	pa_pParameter[154] 	= 0x5a90;
	pa_pParameter[155] 	= 0x5b18;
	pa_pParameter[156] 	= 0x5c05;
	pa_pParameter[157] 	= 0xc3ed;
	pa_pParameter[158] 	= 0x7f00;
	pa_pParameter[160] 	= 0xda04;
	pa_pParameter[161] 	= 0xe51f;
	pa_pParameter[162] 	= 0xe167;
	pa_pParameter[163] 	= 0xe000;
	pa_pParameter[164] 	= 0xdd7f;
	pa_pParameter[165] 	= 0x0500;
	pa_pParameter[166] 	= 0xff01;
	pa_pParameter[167] 	= 0x0901;		//increase fan out of output driver
	
	return 168;
}

unsigned short OVxxxx_paramSet_OV2641_YUV(unsigned short *pa_pParameter) {
	pa_pParameter[0] 	= 0xFF00;
	pa_pParameter[1] 	= 0xE004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	//Add 10/31/05
	pa_pParameter[4] 	= 0xDA01;
	pa_pParameter[5] 	= 0xD382;
	pa_pParameter[6] 	= 0xE000;
	pa_pParameter[7] 	= 0xffff;
	return 8;
}

unsigned short OVxxxx_paramSet_OV2641_SVGA_12_5fps_YUV(unsigned short *pa_pParameter) {
	pa_pParameter[0] 	= 0xff00;
	pa_pParameter[1] 	= 0xe004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	//Add 10/31/05
	pa_pParameter[4] 	= 0xDA01;
	pa_pParameter[5] 	= 0xc0c8;
	pa_pParameter[6] 	= 0xc196;
	pa_pParameter[7] 	= 0x8635;	//	;enable dcw 
	pa_pParameter[8] 	= 0x5089;
	pa_pParameter[9] 	= 0x5190;
	pa_pParameter[10] 	= 0x522c;
	pa_pParameter[11] 	= 0x5300;
	pa_pParameter[12] 	= 0x5400;
	pa_pParameter[13] 	= 0x5588;
	pa_pParameter[14] 	= 0x5700;
	pa_pParameter[15] 	= 0x5ac8;	// zoom width
	pa_pParameter[16] 	= 0x5b96;	// zoom high
	pa_pParameter[17] 	= 0x5c00;
	pa_pParameter[18] 	= 0xd304;
	pa_pParameter[19] 	= 0xe000;
	pa_pParameter[20] 	= 0xffff;
	return 21;
}

unsigned short OVxxxx_paramSet_OV2641_SVGA_28fps_YUV(unsigned short *pa_pParameter) {
	/*pa_pParameter[0] 	= 0xff01;
	pa_pParameter[1] 	= 0x1240;
	pa_pParameter[2] 	= 0x1711;
	pa_pParameter[3] 	= 0x1843;
	pa_pParameter[4] 	= 0x1900;
	pa_pParameter[5] 	= 0x1a4b;
	pa_pParameter[6] 	= 0x3209;
	pa_pParameter[7] 	= 0x4fca;
	pa_pParameter[8] 	= 0x50a8;
	pa_pParameter[9] 	= 0x5a23;
	pa_pParameter[10] 	= 0x6d00;
	pa_pParameter[11] 	= 0x3d38;
	pa_pParameter[12] 	= 0x3912;
	pa_pParameter[13] 	= 0x35da; 
	pa_pParameter[14] 	= 0x221a; 
	pa_pParameter[15] 	= 0x37c3; 
	pa_pParameter[16] 	= 0x2300;
	pa_pParameter[17] 	= 0x34c0; //;a0 ;c0-
	pa_pParameter[18] 	= 0x361a;
	pa_pParameter[19] 	= 0x0688; 
	pa_pParameter[20] 	= 0x07c0;
	pa_pParameter[21] 	= 0x0d87;
	pa_pParameter[22] 	= 0x0e41;
	pa_pParameter[23] 	= 0x4c00;
	pa_pParameter[24] 	= 0x4800;
	pa_pParameter[25] 	= 0x5B00;
	pa_pParameter[26] 	= 0x4203;
	pa_pParameter[27] 	= 0xff00;
	pa_pParameter[28] 	= 0xe004;
	pa_pParameter[29] 	= 0xE167;
	pa_pParameter[30] 	= 0xD701;	//Add 10/31/05
	pa_pParameter[31] 	= 0xDA01;
	pa_pParameter[32] 	= 0xc064;
	pa_pParameter[33] 	= 0xc14B;
	pa_pParameter[34] 	= 0x8c00;
	pa_pParameter[35] 	= 0x861D;
	pa_pParameter[36] 	= 0xd382;
	pa_pParameter[37] 	= 0xe000;
	pa_pParameter[38] 	= 0xffff;
	return 39;
	*/
	
	pa_pParameter[0] 	= 0xff00;
	pa_pParameter[1] 	= 0xe004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	//Add 10/31/05
	pa_pParameter[4] 	= 0xDA01;
	pa_pParameter[5] 	= 0xc0c8;
	pa_pParameter[6] 	= 0xc196;
	pa_pParameter[7] 	= 0x8635;	//	;enable dcw 
	pa_pParameter[8] 	= 0x5089;
	pa_pParameter[9] 	= 0x5190;
	pa_pParameter[10] 	= 0x522c;
	pa_pParameter[11] 	= 0x5300;
	pa_pParameter[12] 	= 0x5400;
	pa_pParameter[13] 	= 0x5588;
	pa_pParameter[14] 	= 0x5700;
	pa_pParameter[15] 	= 0x5ac8;	// zoom width
	pa_pParameter[16] 	= 0x5b96;	// zoom high
	pa_pParameter[17] 	= 0x5c00;
	pa_pParameter[18] 	= 0xd304;
	pa_pParameter[19] 	= 0xe000;
	pa_pParameter[20] 	= 0xff01;
	pa_pParameter[21] 	= 0x1180;
	pa_pParameter[22] 	= 0xffff;
	return 23;	
}

unsigned short OVxxxx_paramSet_OV2641_VGA_12_5fps_YUV(unsigned short *pa_pParameter) {    
	pa_pParameter[0] 	= 0xff00;
	pa_pParameter[1] 	= 0xe004;
   	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	//Add 10/31/05
	pa_pParameter[4] 	= 0xDA01;
	pa_pParameter[5] 	= 0xc0c8;
	pa_pParameter[6] 	= 0xc196;
	pa_pParameter[7] 	= 0x863d;	//;enable dcw 
	pa_pParameter[8] 	= 0x5089;
	pa_pParameter[9] 	= 0x5190;
	pa_pParameter[10] 	= 0x522c;
	pa_pParameter[11] 	= 0x5300;
	pa_pParameter[12] 	= 0x5400;
	pa_pParameter[13] 	= 0x5588;
	pa_pParameter[14] 	= 0x5700;
	pa_pParameter[15] 	= 0x5aa0;	// zoom width
	pa_pParameter[16] 	= 0x5b78;	// zoom high
	pa_pParameter[17] 	= 0x5c00;
	pa_pParameter[18] 	= 0xd304;
	pa_pParameter[19] 	= 0xe000;
	pa_pParameter[20] 	= 0xff01;
	pa_pParameter[21] 	= 0x1180;	
	pa_pParameter[22] 	= 0xffff;
	return 21;
}

unsigned short OVxxxx_paramSet_OV2641_QVGA_12_5fps_YUV(unsigned short *pa_pParameter) {    
	pa_pParameter[0] 	= 0xff00;
	pa_pParameter[1] 	= 0xe004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	// ;Add 10/31/05
	pa_pParameter[4] 	= 0xDA01;
	pa_pParameter[5] 	= 0xD382;
	pa_pParameter[6] 	= 0xc0c8;
	pa_pParameter[7] 	= 0xc196;
	pa_pParameter[8] 	= 0x8635;	//	;enable dcw 
	pa_pParameter[9] 	= 0x5092;
	pa_pParameter[10] 	= 0x5190;
	pa_pParameter[11] 	= 0x522c;
	pa_pParameter[12] 	= 0x5300;
	pa_pParameter[13] 	= 0x5400;
	pa_pParameter[14] 	= 0x5588;
	pa_pParameter[15] 	= 0x5700;
	pa_pParameter[16] 	= 0x5a50;
	pa_pParameter[17] 	= 0x5b3c;
	pa_pParameter[18] 	= 0x5c00;
	pa_pParameter[19] 	= 0xd308;	// pclk control
	pa_pParameter[20] 	= 0xe000;
	pa_pParameter[21] 	= 0xffff;
	return 22;
}

unsigned short OVxxxx_paramSet_OV2641_QVGA_25fps_YUV(unsigned short *pa_pParameter) {    
	pa_pParameter[0] 	= 0xff00;
	pa_pParameter[1] 	= 0xe004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	// ;Add 10/31/05
	pa_pParameter[4] 	= 0xDA01;
	pa_pParameter[5] 	= 0xD382;
	pa_pParameter[6] 	= 0xc0c8;
	pa_pParameter[7] 	= 0xc196;
	pa_pParameter[8] 	= 0x8635;	//	;enable dcw 
	pa_pParameter[9] 	= 0x5092;
	pa_pParameter[10] 	= 0x5190;
	pa_pParameter[11] 	= 0x522c;
	pa_pParameter[12] 	= 0x5300;
	pa_pParameter[13] 	= 0x5400;
	pa_pParameter[14] 	= 0x5588;
	pa_pParameter[15] 	= 0x5700;
	pa_pParameter[16] 	= 0x5a50;
	pa_pParameter[17] 	= 0x5b3c;
	pa_pParameter[18] 	= 0x5c00;
	pa_pParameter[19] 	= 0xd304;	// pclk control
	pa_pParameter[20] 	= 0xe000;
	pa_pParameter[21] 	= 0xff01;	
	pa_pParameter[22] 	= 0x1180;	
	pa_pParameter[23] 	= 0xffff;
	return 24;
}

unsigned short OVxxxx_paramSet_OV2641_RGB565(unsigned short *pa_pParameter) {	
	pa_pParameter[0] 	= 0xFF00;
	pa_pParameter[1] 	= 0xE004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;//Add 10/31/05
	pa_pParameter[4] 	= 0xDA09;//11/28/05
	pa_pParameter[5] 	= 0x9800;//11/28/05
	pa_pParameter[6] 	= 0x9900;//11/28/05
	pa_pParameter[7] 	= 0xD382;
	pa_pParameter[8] 	= 0xE000;
	pa_pParameter[9] 	= 0xffff;
	return 10;
}

unsigned short OVxxxx_paramSet_OV2641_RAW10(unsigned short *pa_pParameter) {	
	pa_pParameter[0] 	= 0xFF00;
	pa_pParameter[1] 	= 0xE004;
	pa_pParameter[2] 	= 0xE167;
	pa_pParameter[3] 	= 0xD701;	//Add 10/31/05
	pa_pParameter[4] 	= 0xDA04;
	pa_pParameter[5] 	= 0xD382;
	pa_pParameter[6] 	= 0xE000;
	pa_pParameter[7] 	= 0xffff;
	return 8;
}

unsigned short OVxxxx_paramSet_OV2630_Init(unsigned short *pa_pParameter) {	
	pa_pParameter[0] 	= 0x1280;
	pa_pParameter[1] 	= 0x0e00;
	pa_pParameter[2] 	= 0x0f42;
	pa_pParameter[3] 	= 0x1101;
	pa_pParameter[4] 	= 0x1387;
	pa_pParameter[5] 	= 0x14c0;
	pa_pParameter[6] 	= 0x3470;
	pa_pParameter[7] 	= 0x3590;
	pa_pParameter[8] 	= 0x3688;
	pa_pParameter[9] 	= 0x3744;
	pa_pParameter[10] 	= 0x3a94;
	pa_pParameter[11] 	= 0x3f0f;
	pa_pParameter[12] 	= 0x4000;
	pa_pParameter[13] 	= 0x4100;
	pa_pParameter[14] 	= 0x4200;
	pa_pParameter[15] 	= 0x4300;
	pa_pParameter[16] 	= 0x4481;
	pa_pParameter[17] 	= 0x4b00;
	pa_pParameter[18] 	= 0x4c28;
	pa_pParameter[19] 	= 0x5022;
	pa_pParameter[20] 	= 0x5807;
	pa_pParameter[21] 	= 0x5f40;
	pa_pParameter[22] 	= 0x750f;
	pa_pParameter[23] 	= 0x7840;
	pa_pParameter[24] 	= 0x7a10;
	pa_pParameter[25] 	= 0x8404;
	pa_pParameter[26] 	= 0x8620;
	pa_pParameter[27] 	= 0x880c;
	pa_pParameter[28] 	= 0x8a02;
	
	return 29;
}

unsigned short OVxxxx_paramSet_OV2630_UXGA_15fps_RAW10(unsigned short *pa_pParameter) {
//for 15fps UXGA mode
	pa_pParameter[0] 	= 0x0340;
	pa_pParameter[1] 	= 0x172d;
	pa_pParameter[2] 	= 0x1801;
	pa_pParameter[3] 	= 0x1901;
	pa_pParameter[4] 	= 0x1a97;
	pa_pParameter[5] 	= 0x1e80;
	pa_pParameter[6] 	= 0x323b;
	pa_pParameter[7] 	= 0x4dc0;
	pa_pParameter[8] 	= 0x5a00;
	pa_pParameter[9] 	= 0x8718;

//for non-zoom
	pa_pParameter[10] 	= 0x0c20;
	pa_pParameter[11] 	= 0x1600;
	pa_pParameter[12] 	= 0x1200;
	pa_pParameter[13] 	= 0x4880;
	pa_pParameter[14] 	= 0x4a00;
	pa_pParameter[15] 	= 0x4e18;
	pa_pParameter[16] 	= 0x4f08;

//01/15/05
//Keep (HVDD-SVDD > 0.8)  to improve left-right unbalance
//For example: Change to 20 if SVDD=2.5
	pa_pParameter[17] 	= 0x3470; 
	pa_pParameter[18] 	= 0x8908; // overflow latch work (independent on register sequence) 

//01/21/05
	pa_pParameter[19] 	= 0x5025; // improve white left side problem

	pa_pParameter[20] 	= 0x13e7;
	pa_pParameter[21] 	= 0x14ce;
	pa_pParameter[22] 	= 0x4480;
	pa_pParameter[23] 	= 0x3a90;
	pa_pParameter[24] 	= 0x3b04;
	pa_pParameter[25] 	= 0xffff;
	
	return 26;
}

unsigned short OVxxxx_paramSet_OV2630_SVGA_30fps_RAW10(unsigned short *pa_pParameter) {
	pa_pParameter[0] 	= 0x030f;
	pa_pParameter[1] 	= 0x173f;
	pa_pParameter[2] 	= 0x1801;
	pa_pParameter[3] 	= 0x1900;
	pa_pParameter[4] 	= 0x1a4b;
	pa_pParameter[5] 	= 0x1e40;
	pa_pParameter[6] 	= 0x323f;
	pa_pParameter[7] 	= 0x4dc0;
	pa_pParameter[8] 	= 0x5a00;
	pa_pParameter[9] 	= 0x8700;
//for non-zoom
	pa_pParameter[10] 	= 0x0ca0;
	pa_pParameter[11] 	= 0x1600;
	pa_pParameter[12] 	= 0x1241;
	pa_pParameter[13] 	= 0x4800;
	pa_pParameter[14] 	= 0x4a00;
	pa_pParameter[15] 	= 0x4e08;
	pa_pParameter[16] 	= 0x4f00;

//01/15/05
// Keep (HVDD-SVDD > 0.8)  to improve left-right unbalance
// For example: Change to 20 if SVDD=2.5
	pa_pParameter[17] 	= 0x3470; 
	pa_pParameter[18] 	= 0x8908; // overflow latch work (independent on register sequence) 

//01/21/05
	pa_pParameter[19] 	= 0x5025; // improve white left side problem
	pa_pParameter[20] 	= 0x13e7;
	pa_pParameter[21] 	= 0x14ce;
	pa_pParameter[22] 	= 0x4480;
	pa_pParameter[23] 	= 0x3a90;
	pa_pParameter[24] 	= 0xffff;
	
	return 25;   
}

unsigned short OVxxxx_paramSet_OV2655_SVGA_YUV422_25FPS(unsigned long *pa_pParameter){
    unsigned short i = 0;
    //IO & Clock & Analog Setup
    pa_pParameter[i++] = 0x308c80;
    pa_pParameter[i++] = 0x308d0e;
    pa_pParameter[i++] = 0x360b00;
    pa_pParameter[i++] = 0x30b0ff;
    pa_pParameter[i++] = 0x30b1ff;
    pa_pParameter[i++] = 0x30b204;

    pa_pParameter[i++] = 0x300e34;
    pa_pParameter[i++] = 0x300fa6;
    pa_pParameter[i++] = 0x301081;
    pa_pParameter[i++] = 0x308201;
    pa_pParameter[i++] = 0x30f401;
    pa_pParameter[i++] = 0x309043;
    pa_pParameter[i++] = 0x3091c0;
    pa_pParameter[i++] = 0x30ac42;
    
//    pa_pParameter[i++] = 0x307702;
    pa_pParameter[i++] = 0x360040;
    
//    pa_pParameter[i++] = 0x330800;        // testpattern, 01 to enable

    pa_pParameter[i++] = 0x30d108;
    pa_pParameter[i++] = 0x30a854;
    pa_pParameter[i++] = 0x301502;
    pa_pParameter[i++] = 0x309300;
    pa_pParameter[i++] = 0x307ee5;
    pa_pParameter[i++] = 0x307900;
    pa_pParameter[i++] = 0x30aa52;
    pa_pParameter[i++] = 0x301740;
    pa_pParameter[i++] = 0x30f383;
    pa_pParameter[i++] = 0x306a0c;
    pa_pParameter[i++] = 0x306d00;
    pa_pParameter[i++] = 0x336a3c;
    pa_pParameter[i++] = 0x30766a;
    pa_pParameter[i++] = 0x30d995;
    pa_pParameter[i++] = 0x301682;
    pa_pParameter[i++] = 0x360130;
    pa_pParameter[i++] = 0x304e88;
    pa_pParameter[i++] = 0x30f182;
    pa_pParameter[i++] = 0x306f14;
    pa_pParameter[i++] = 0x302a02;
    pa_pParameter[i++] = 0x302b6a;

    pa_pParameter[i++] = 0x301210;
    pa_pParameter[i++] = 0x301101;
    
    //AEC/AGC
    pa_pParameter[i++] = 0x3013f7;
    pa_pParameter[i++] = 0x301870;
    pa_pParameter[i++] = 0x301960;
    pa_pParameter[i++] = 0x301ad4;
    pa_pParameter[i++] = 0x301c13;
    pa_pParameter[i++] = 0x301d17;
    pa_pParameter[i++] = 0x30705d;
    pa_pParameter[i++] = 0x30724d;
    
    //D5060
    pa_pParameter[i++] = 0x30af00;
    pa_pParameter[i++] = 0x30481f; 
    pa_pParameter[i++] = 0x30494e;
    pa_pParameter[i++] = 0x304a20;
    pa_pParameter[i++] = 0x304f20;
    pa_pParameter[i++] = 0x304b02;
    pa_pParameter[i++] = 0x304c00;
    pa_pParameter[i++] = 0x304d02;
    pa_pParameter[i++] = 0x304f20;
    pa_pParameter[i++] = 0x30a310;
    pa_pParameter[i++] = 0x3013f7;
    pa_pParameter[i++] = 0x301444;
    pa_pParameter[i++] = 0x307100;
    pa_pParameter[i++] = 0x30705d;
    pa_pParameter[i++] = 0x307300;
    pa_pParameter[i++] = 0x30724d;
    pa_pParameter[i++] = 0x301c05;
    pa_pParameter[i++] = 0x301d06;
    pa_pParameter[i++] = 0x304d42;     
    pa_pParameter[i++] = 0x304a40;  
    pa_pParameter[i++] = 0x304f40;  
    pa_pParameter[i++] = 0x309507;  
    pa_pParameter[i++] = 0x309616; 
    pa_pParameter[i++] = 0x30971d; 
    
    //Window Setup
    pa_pParameter[i++] = 0x302001;
    pa_pParameter[i++] = 0x302118;
    pa_pParameter[i++] = 0x302200;
    pa_pParameter[i++] = 0x302306;
    pa_pParameter[i++] = 0x302406;
    pa_pParameter[i++] = 0x302558;
    pa_pParameter[i++] = 0x302602;
    pa_pParameter[i++] = 0x30275e;
    pa_pParameter[i++] = 0x308803;
    pa_pParameter[i++] = 0x308920;
    pa_pParameter[i++] = 0x308a02;
    pa_pParameter[i++] = 0x308b58;
    pa_pParameter[i++] = 0x331664;
    pa_pParameter[i++] = 0x331725;
    pa_pParameter[i++] = 0x331880;
    pa_pParameter[i++] = 0x331908;
    pa_pParameter[i++] = 0x331a64;
    pa_pParameter[i++] = 0x331b4b;
    pa_pParameter[i++] = 0x331c00;
    pa_pParameter[i++] = 0x331d38;
    pa_pParameter[i++] = 0x310000;
    
    //AWB
    pa_pParameter[i++] = 0x3320fa;
    pa_pParameter[i++] = 0x332111;
    pa_pParameter[i++] = 0x332292;
    pa_pParameter[i++] = 0x332301;
    pa_pParameter[i++] = 0x332497;
    pa_pParameter[i++] = 0x332502;
    pa_pParameter[i++] = 0x3326ff;
    pa_pParameter[i++] = 0x332710;
    pa_pParameter[i++] = 0x332810;
    pa_pParameter[i++] = 0x33291f;
    pa_pParameter[i++] = 0x332a58;
    pa_pParameter[i++] = 0x332b50;
    pa_pParameter[i++] = 0x332cbe;
    pa_pParameter[i++] = 0x332dce;
    pa_pParameter[i++] = 0x332e2e;
    pa_pParameter[i++] = 0x332f36;
    pa_pParameter[i++] = 0x33304d;
    pa_pParameter[i++] = 0x333144;
    pa_pParameter[i++] = 0x3332f0;
    pa_pParameter[i++] = 0x33330a;
    pa_pParameter[i++] = 0x3334f0;
    pa_pParameter[i++] = 0x3335f0;
    pa_pParameter[i++] = 0x3336f0;
    pa_pParameter[i++] = 0x333740;
    pa_pParameter[i++] = 0x333840;
    pa_pParameter[i++] = 0x333940;
    pa_pParameter[i++] = 0x333a00;
    pa_pParameter[i++] = 0x333b00;
    
    //Color Matrix
    pa_pParameter[i++] = 0x338020;
    pa_pParameter[i++] = 0x33815b;
    pa_pParameter[i++] = 0x338205;
    pa_pParameter[i++] = 0x338322;
    pa_pParameter[i++] = 0x33849d;
    pa_pParameter[i++] = 0x3385c0;
    pa_pParameter[i++] = 0x3386b6;
    pa_pParameter[i++] = 0x3387b5;
    pa_pParameter[i++] = 0x338802;
    pa_pParameter[i++] = 0x338998;
    pa_pParameter[i++] = 0x338a00;
    
    //Gamma
    pa_pParameter[i++] = 0x334009;
    pa_pParameter[i++] = 0x334119;
    pa_pParameter[i++] = 0x33422f;
    pa_pParameter[i++] = 0x334345;
    pa_pParameter[i++] = 0x33445a;
    pa_pParameter[i++] = 0x334569;
    pa_pParameter[i++] = 0x334675;
    pa_pParameter[i++] = 0x33477e;
    pa_pParameter[i++] = 0x334888;
    pa_pParameter[i++] = 0x334996;
    pa_pParameter[i++] = 0x334aa3;
    pa_pParameter[i++] = 0x334baf;
    pa_pParameter[i++] = 0x334cc4;
    pa_pParameter[i++] = 0x334dd7;
    pa_pParameter[i++] = 0x334ee8;
    pa_pParameter[i++] = 0x334f20;
    
    //Lens correction
    pa_pParameter[i++] = 0x335032;
    pa_pParameter[i++] = 0x335125;
    pa_pParameter[i++] = 0x335280;
    pa_pParameter[i++] = 0x33531e;
    pa_pParameter[i++] = 0x335400;
    pa_pParameter[i++] = 0x335584;
    pa_pParameter[i++] = 0x335632;
    pa_pParameter[i++] = 0x335725;
    pa_pParameter[i++] = 0x335880;
    pa_pParameter[i++] = 0x33591b;
    pa_pParameter[i++] = 0x335a00;
    pa_pParameter[i++] = 0x335b84;
    pa_pParameter[i++] = 0x335c32;
    pa_pParameter[i++] = 0x335d25;
    pa_pParameter[i++] = 0x335e80;
    pa_pParameter[i++] = 0x335f1b;
    pa_pParameter[i++] = 0x336000;
    pa_pParameter[i++] = 0x336184;
    pa_pParameter[i++] = 0x336370;
    pa_pParameter[i++] = 0x33647f;
    pa_pParameter[i++] = 0x336500;
    pa_pParameter[i++] = 0x336600;
    
    //UVadjust
    pa_pParameter[i++] = 0x3301ff;
    pa_pParameter[i++] = 0x338B1b;
    pa_pParameter[i++] = 0x338c1f;
    pa_pParameter[i++] = 0x338d40;
    
    //Sharpness/De-noise
    pa_pParameter[i++] = 0x3370d0;
    pa_pParameter[i++] = 0x337100;
    pa_pParameter[i++] = 0x337200;
    pa_pParameter[i++] = 0x337340;
    pa_pParameter[i++] = 0x337410;
    pa_pParameter[i++] = 0x337510;
    pa_pParameter[i++] = 0x337604;
    pa_pParameter[i++] = 0x337700;
    pa_pParameter[i++] = 0x337804;
    pa_pParameter[i++] = 0x337980;
    
    //BLC
    pa_pParameter[i++] = 0x306986;
    pa_pParameter[i++] = 0x307c10;
    pa_pParameter[i++] = 0x308702;
    
    //Black sun - Avdd 2.55~3.0V
    pa_pParameter[i++] = 0x309003;
    pa_pParameter[i++] = 0x30aa52;
    pa_pParameter[i++] = 0x30a380;
    pa_pParameter[i++] = 0x30a141;
    
    //Other functions
    pa_pParameter[i++] = 0x3300fc;
    pa_pParameter[i++] = 0x330211;
    pa_pParameter[i++] = 0x340002;
    pa_pParameter[i++] = 0x360620;
    pa_pParameter[i++] = 0x360130;
    pa_pParameter[i++] = 0x300e34;
    pa_pParameter[i++] = 0x30f383;
    pa_pParameter[i++] = 0x304e88;
    pa_pParameter[i++] = 0x363b01;
    pa_pParameter[i++] = 0x363cf2;

    pa_pParameter[i++] = 0x30860f;
    pa_pParameter[i++] = 0x308600;

    //for 20fps
    pa_pParameter[i++] = 0x301100;
    pa_pParameter[i++] = 0x300E36;
    pa_pParameter[i++] = 0x302c04;

    pa_pParameter[i++] = 0x307100;
    pa_pParameter[i++] = 0x307098;
    pa_pParameter[i++] = 0x301c07;

    pa_pParameter[i++] = 0x307300;
    pa_pParameter[i++] = 0x307281;
    pa_pParameter[i++] = 0x301d08;
    
    return i;
}


unsigned short OVxxxx_paramSet_OV2655_VGA_YUV422_25FPS(unsigned long *pa_pParameter){
    unsigned short i = 0;
    
    //IO & Clock & Analog Setup
    pa_pParameter[i++] = 0x308c80;
    pa_pParameter[i++] = 0x308d0e;
    pa_pParameter[i++] = 0x360b00;
    pa_pParameter[i++] = 0x30b0ff;
    pa_pParameter[i++] = 0x30b1ff;
    pa_pParameter[i++] = 0x30b204;

    pa_pParameter[i++] = 0x300e34;
    pa_pParameter[i++] = 0x300fa6;
    pa_pParameter[i++] = 0x301081;
    pa_pParameter[i++] = 0x308201;
    pa_pParameter[i++] = 0x30f401;
    pa_pParameter[i++] = 0x309043;
    pa_pParameter[i++] = 0x3091c0;
    pa_pParameter[i++] = 0x30ac42;
    
//    pa_pParameter[i++] = 0x307702;
    pa_pParameter[i++] = 0x360040;

    pa_pParameter[i++] = 0x30d108;
    pa_pParameter[i++] = 0x30a854;
    pa_pParameter[i++] = 0x301502;
    pa_pParameter[i++] = 0x309300;
    pa_pParameter[i++] = 0x307ee5;
    pa_pParameter[i++] = 0x307900;
    pa_pParameter[i++] = 0x30aa52;
    pa_pParameter[i++] = 0x301740;
    pa_pParameter[i++] = 0x30f383;
    pa_pParameter[i++] = 0x306a0c;
    pa_pParameter[i++] = 0x306d00;
    pa_pParameter[i++] = 0x336a3c;
    pa_pParameter[i++] = 0x30766a;
    pa_pParameter[i++] = 0x30d995;
    pa_pParameter[i++] = 0x301682;
    pa_pParameter[i++] = 0x360130;
    pa_pParameter[i++] = 0x304e88;
    pa_pParameter[i++] = 0x30f182;
    pa_pParameter[i++] = 0x306f14;
    pa_pParameter[i++] = 0x302a02;
    pa_pParameter[i++] = 0x302b6a;

    pa_pParameter[i++] = 0x301210;
    pa_pParameter[i++] = 0x301100;

    //AEC/AGC
    pa_pParameter[i++] = 0x3013f7;
    pa_pParameter[i++] = 0x301870;
    pa_pParameter[i++] = 0x301960;
    pa_pParameter[i++] = 0x301ad4;
    pa_pParameter[i++] = 0x301c13;
    pa_pParameter[i++] = 0x301d17;
    pa_pParameter[i++] = 0x30705d;
    pa_pParameter[i++] = 0x30724d;

    //D5060
    pa_pParameter[i++] = 0x30af00;
    pa_pParameter[i++] = 0x30481f;
    pa_pParameter[i++] = 0x30494e;
    pa_pParameter[i++] = 0x304a20;
    pa_pParameter[i++] = 0x304f20;
    pa_pParameter[i++] = 0x304b02;
    pa_pParameter[i++] = 0x304c00;
    pa_pParameter[i++] = 0x304d02;
    pa_pParameter[i++] = 0x304f20;
    pa_pParameter[i++] = 0x30a310;
    pa_pParameter[i++] = 0x3013f7;
    pa_pParameter[i++] = 0x301444;
    pa_pParameter[i++] = 0x307100;
    pa_pParameter[i++] = 0x30705d;
    pa_pParameter[i++] = 0x307300;
    pa_pParameter[i++] = 0x30724d;
    pa_pParameter[i++] = 0x301c05;
    pa_pParameter[i++] = 0x301d06;
    pa_pParameter[i++] = 0x304d42;
    pa_pParameter[i++] = 0x304a40;
    pa_pParameter[i++] = 0x304f40;
    pa_pParameter[i++] = 0x309507;
    pa_pParameter[i++] = 0x309616;
    pa_pParameter[i++] = 0x30971d;

    //Window Setup
    pa_pParameter[i++] = 0x302001;
    pa_pParameter[i++] = 0x302118;
    pa_pParameter[i++] = 0x302200;
    pa_pParameter[i++] = 0x302306;
    pa_pParameter[i++] = 0x302406;
    pa_pParameter[i++] = 0x302558;
    pa_pParameter[i++] = 0x302602;
    pa_pParameter[i++] = 0x302761;
    pa_pParameter[i++] = 0x308802;
    pa_pParameter[i++] = 0x308980;
    pa_pParameter[i++] = 0x308a01;
    pa_pParameter[i++] = 0x308be0;
    pa_pParameter[i++] = 0x331664;
    pa_pParameter[i++] = 0x331725;
    pa_pParameter[i++] = 0x331880;
    pa_pParameter[i++] = 0x331908;
    pa_pParameter[i++] = 0x331a28;
    pa_pParameter[i++] = 0x331b1e;
    pa_pParameter[i++] = 0x331c00;
    pa_pParameter[i++] = 0x331d38;
    pa_pParameter[i++] = 0x310000;

    //AWB
    pa_pParameter[i++] = 0x3320fa;
    pa_pParameter[i++] = 0x332111;
    pa_pParameter[i++] = 0x332292;
    pa_pParameter[i++] = 0x332301;
    pa_pParameter[i++] = 0x332497;
    pa_pParameter[i++] = 0x332502;
    pa_pParameter[i++] = 0x3326ff;
    pa_pParameter[i++] = 0x332710;
    pa_pParameter[i++] = 0x332810;
    pa_pParameter[i++] = 0x33291f;
    pa_pParameter[i++] = 0x332a58;
    pa_pParameter[i++] = 0x332b50;
    pa_pParameter[i++] = 0x332cbe;
    pa_pParameter[i++] = 0x332dce;
    pa_pParameter[i++] = 0x332e2e;
    pa_pParameter[i++] = 0x332f36;
    pa_pParameter[i++] = 0x33304d;
    pa_pParameter[i++] = 0x333144;
    pa_pParameter[i++] = 0x3332f0;
    pa_pParameter[i++] = 0x33330a;
    pa_pParameter[i++] = 0x3334f0;
    pa_pParameter[i++] = 0x3335f0;
    pa_pParameter[i++] = 0x3336f0;
    pa_pParameter[i++] = 0x333740;
    pa_pParameter[i++] = 0x333840;
    pa_pParameter[i++] = 0x333940;
    pa_pParameter[i++] = 0x333a00;
    pa_pParameter[i++] = 0x333b00;

    //Color Matrix
    pa_pParameter[i++] = 0x338020;
    pa_pParameter[i++] = 0x33815b;
    pa_pParameter[i++] = 0x338205;
    pa_pParameter[i++] = 0x338322;
    pa_pParameter[i++] = 0x33849d;
    pa_pParameter[i++] = 0x3385c0;
    pa_pParameter[i++] = 0x3386b6;
    pa_pParameter[i++] = 0x3387b5;
    pa_pParameter[i++] = 0x338802;
    pa_pParameter[i++] = 0x338998;
    pa_pParameter[i++] = 0x338a00;

    //Gamma
    pa_pParameter[i++] = 0x334009;
    pa_pParameter[i++] = 0x334119;
    pa_pParameter[i++] = 0x33422f;
    pa_pParameter[i++] = 0x334345;
    pa_pParameter[i++] = 0x33445a;
    pa_pParameter[i++] = 0x334569;
    pa_pParameter[i++] = 0x334675;
    pa_pParameter[i++] = 0x33477e;
    pa_pParameter[i++] = 0x334888;
    pa_pParameter[i++] = 0x334996;
    pa_pParameter[i++] = 0x334aa3;
    pa_pParameter[i++] = 0x334baf;
    pa_pParameter[i++] = 0x334cc4;
    pa_pParameter[i++] = 0x334dd7;
    pa_pParameter[i++] = 0x334ee8;
    pa_pParameter[i++] = 0x334f20;

    //Lens correction
    pa_pParameter[i++] = 0x335032;
    pa_pParameter[i++] = 0x335125;
    pa_pParameter[i++] = 0x335280;
    pa_pParameter[i++] = 0x33531e;
    pa_pParameter[i++] = 0x335400;
    pa_pParameter[i++] = 0x335584;
    pa_pParameter[i++] = 0x335632;
    pa_pParameter[i++] = 0x335725;
    pa_pParameter[i++] = 0x335880;
    pa_pParameter[i++] = 0x33591b;
    pa_pParameter[i++] = 0x335a00;
    pa_pParameter[i++] = 0x335b84;
    pa_pParameter[i++] = 0x335c32;
    pa_pParameter[i++] = 0x335d25;
    pa_pParameter[i++] = 0x335e80;
    pa_pParameter[i++] = 0x335f1b;
    pa_pParameter[i++] = 0x336000;
    pa_pParameter[i++] = 0x336184;
    pa_pParameter[i++] = 0x336370;
    pa_pParameter[i++] = 0x33647f;
    pa_pParameter[i++] = 0x336500;
    pa_pParameter[i++] = 0x336600;

    //UVadjust
    pa_pParameter[i++] = 0x3301ff;
    pa_pParameter[i++] = 0x338B1b;
    pa_pParameter[i++] = 0x338c1f;
    pa_pParameter[i++] = 0x338d40;

    //Sharpness/De-noise
    pa_pParameter[i++] = 0x3370d0;
    pa_pParameter[i++] = 0x337100;
    pa_pParameter[i++] = 0x337200;
    pa_pParameter[i++] = 0x337340;
    pa_pParameter[i++] = 0x337410;
    pa_pParameter[i++] = 0x337510;
    pa_pParameter[i++] = 0x337604;
    pa_pParameter[i++] = 0x337700;
    pa_pParameter[i++] = 0x337804;
    pa_pParameter[i++] = 0x337980;

    //BLC
    pa_pParameter[i++] = 0x306986;
    pa_pParameter[i++] = 0x307c10;
    pa_pParameter[i++] = 0x308702;

    //black sun 
    //Avdd 2.55~3.0V
    pa_pParameter[i++] = 0x309003;
    pa_pParameter[i++] = 0x30aa52;
    pa_pParameter[i++] = 0x30a380;
    pa_pParameter[i++] = 0x30a141;

    //Other functions
    pa_pParameter[i++] = 0x3300fc;
    pa_pParameter[i++] = 0x330211;
    pa_pParameter[i++] = 0x340002;
    pa_pParameter[i++] = 0x360620;
    pa_pParameter[i++] = 0x360130;
    pa_pParameter[i++] = 0x300e34;
    pa_pParameter[i++] = 0x30f383;
    pa_pParameter[i++] = 0x304e88;
    pa_pParameter[i++] = 0x363b01;
    pa_pParameter[i++] = 0x363cf2;

    pa_pParameter[i++] = 0x30a181;
    pa_pParameter[i++] = 0x338b21;
    pa_pParameter[i++] = 0x338d90;

    pa_pParameter[i++] = 0x30860f;
    pa_pParameter[i++] = 0x308600;

    //for 20fps
    pa_pParameter[i++] = 0x301100;
    pa_pParameter[i++] = 0x300E36;
    pa_pParameter[i++] = 0x302c04;

    pa_pParameter[i++] = 0x307100;
    pa_pParameter[i++] = 0x307098;
    pa_pParameter[i++] = 0x301c07;

    pa_pParameter[i++] = 0x307300;
    pa_pParameter[i++] = 0x307281;
    pa_pParameter[i++] = 0x301d08;

    return i;
}
