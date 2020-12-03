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

#ifndef _PPI656I_H_INCLUDED_
#define _PPI656I_H_INCLUDED_

#include <Datatypes.h>
#include "../../../../../BLACKSheep\common\frameGrabber\frameGrabber.h"
#include "../analogVideo/AnalogVideo.h"

#define ERR_PPI656I_OUT_OF_MEM                  (ERR_GENERIC - 10) 
#define ERR_PPI656I_UNKNWON_AV_STD              (ERR_GENERIC - 11)
#define ERR_PPI656I_MAX_DEVICES                 (ERR_GENERIC - 12)
#define ERR_PPI656I_INVALID_HANDLE              (ERR_GENERIC - 13)
#define ERR_PPI656I_PPI_NOT_AVAILABLE           (ERR_GENERIC - 14)
#define ERR_PPI6565I_PLATFORM_INIT              (ERR_GENERIC - 15)
#define ERR_PPI656I_HOOK_INTERRUPT              (ERR_GENERIC - 16)
#define ERR_PPI656I_NO_FREE_FRAME               (ERR_GENERIC - 17)
#define ERR_PPI656I_DEQUEUE_ALWAYS_IN_PROGRESS  (ERR_GENERIC - 18)

#define PPI656I_MAX_X_SIZE                   1024
#define PPI656I_MAX_Y_SIZE                   1024
#define PPI656I_PIXEL_SIZE                   2
#define PPI656I_MAX_DEVICES                  4
#define PPI656I_DMA_DESC_SIZE                4

typedef signed long T_PPI656I_HANDLE;
typedef void (*T_PPI656I_CALLBACK)(void *pa_pClientArg);    ///< Callback function for PPI interrupt
typedef unsigned long (*T_PPI656I_GET_TIME_STAMP)(void *pa_pClientArg);    ///< Get time stamp function
typedef void (*PPI656I_FN_CLOSE_DEVICE)(unsigned long pa_pHwDevHdl);


typedef enum {
    PPI656I_BUFFER_OVERWRITE,
    PPI656I_BUFFER_NOT_OVERWRITE
} T_PPI656I_BUFFER_MODE;

typedef struct {
    unsigned char cPPInr;
    unsigned long nPPIIVG;            ///< PPI IVG 0 = standard
    unsigned long nErrorIVG;          ///< PPI Error IVG 0 = standard 
    unsigned long nDMAErrorIVG;       ///< DMA Error IVG 0 = standard
    unsigned char cNumberOfFrames;
    T_AV_VIDEOFORMAT tAVformat;
    T_PPI656I_BUFFER_MODE tBufferMode;
    T_PPI656I_CALLBACK fnTransferCallback;  ///< This function is executed after a full frame transfer
   	T_PPI656I_CALLBACK fnErrorCallback;    
   	T_PPI656I_GET_TIME_STAMP fnGetTimeStamp; ///< Get time stamp function, this function is called to set a time stamp to the transfered frame. If NULL the time stamp is simply a counter incremented by 1 for each frame starting with 0. If not NULL the time stamp is calculated by the calling function.
	PPI656I_FN_CLOSE_DEVICE fnHwDevCloseDev;///< attach specified hardware function for closing the device to PPIcameraIn driver, if 0 no hardware attached
	unsigned long hHwDevHndl;                ///< handle of the specified hardware attached to PPIcameraIn, if 0 no hardware attached   	   	
} T_PPI656I_CONFIG;



typedef struct {
    unsigned char cDeviceNr;     
    void*   ptDeviceHndl;               ///< handle of underlying hardware driver
    unsigned char   cNumberOfFrames;    ///< number of frames
    unsigned char   cPPInr;             ///< PPI number
    unsigned long   nPPIIVG;            ///< PPI IVG 0 = standard
    unsigned long   nErrorIVG;          ///< PPI Error IVG 0 = standard
    unsigned long   nDMAErrorIVG;       ///< DMA Error IVG 0 = standard 
    unsigned char   *pcFramebuffer[FG_MAX_NOF_BUFFERED_FRAMES];  ///< Frames buffer for all frames
    ifrm_t          atFrames[FG_MAX_NOF_BUFFERED_FRAMES];        ///< Structure to get the pointer and the time stamp of each frame
    T_AV_VIDEOFORMAT tAVformat;         ///< Active Video format
    T_AV_STANDARD_DESCR tAVstandard;    ///< parameters of analog video standard
    unsigned char cFramesInBuffer;      ///< number of valid frames in buffer
    unsigned char cActiveFrameIn;       ///< 
    unsigned char cActiveFrameOut;
    bool bDequeueInProgress;            ///< flag is set if dequeue is in progress
    T_PPI656I_BUFFER_MODE tBufferMode;  ///< 
    T_PPI656I_CALLBACK fnTransferCallback;  ///< 
   	T_PPI656I_CALLBACK fnErrorCallback;
   	T_PPI656I_GET_TIME_STAMP fnGetTimeStamp; ///< Get time stamp function, this function is called to set a time stamp to the transfered frame. If NULL the time stamp is simply a counter incremented by 1 for each frame starting with 0. If not NULL the time stamp is calculated by the calling function.   	
   	unsigned long nTransmittedLines;         ///< Number of transmitted lines for the active frame
   	unsigned long nTimeStampCount;           ///< Counter for the time stamp
   	bool bGrabbingStarted;                   ///< is TRUE if a grabbing process was started, otherwise FALSE
	PPI656I_FN_CLOSE_DEVICE fnHwDevCloseDev;///< attach specified hardware function for closing the device to PPIcameraIn driver, if 0 no hardware attached
	unsigned long hHwDevHndl;                ///< handle of the specified hardware attached to PPIcameraIn, if 0 no hardware attached   	
} T_PPI656I_INST;



T_PPI656I_HANDLE PPI656Iopen(T_PPI656I_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_tError, void **pa_tFGfunctions);
T_ERROR_CODE PPI656IresetDevice(T_PPI656I_HANDLE pa_tHndl);
void PPI656IgetDeviceInfo(T_PPI656I_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock);
T_ERROR_CODE PPI656IconfigDevice(T_PPI656I_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg);

#endif
