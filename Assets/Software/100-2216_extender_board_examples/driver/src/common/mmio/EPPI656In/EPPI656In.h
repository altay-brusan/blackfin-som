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

#ifndef _EPPI656I_H_INCLUDED_
#define _EPPI656I_H_INCLUDED_

#include <Datatypes.h>
#include "../../../../../BLACKSheep\common\frameGrabber\frameGrabber.h"
#include "../analogVideo/AnalogVideo.h"

#define ERR_EPPI656I_OUT_OF_MEM                  (ERR_GENERIC - 10) 
#define ERR_EPPI656I_UNKNWON_AV_STD              (ERR_GENERIC - 11)
#define ERR_EPPI656I_MAX_DEVICES                 (ERR_GENERIC - 12)
#define ERR_EPPI656I_INVALID_HANDLE              (ERR_GENERIC - 13)
#define ERR_EPPI656I_EPPI_NOT_AVAILABLE           (ERR_GENERIC - 14)
#define ERR_EPPI6565I_PLATFORM_INIT              (ERR_GENERIC - 15)
#define ERR_EPPI656I_HOOK_INTERRUPT              (ERR_GENERIC - 16)
#define ERR_EPPI656I_NO_FREE_FRAME               (ERR_GENERIC - 17)
#define ERR_EPPI656I_DEQUEUE_ALWAYS_IN_PROGRESS  (ERR_GENERIC - 18)


// Select driver behaviour ****************************************

// ****************************************************************

#define EPPI656I_MAX_X_SIZE                   1024
#define EPPI656I_MAX_Y_SIZE                   1024
#define EPPI656I_PIXEL_SIZE                   2
#define EPPI656I_MAX_DEVICES                  4
#define EPPI656I_DMA_DESC_SIZE                4
#define EPPI656I_DMA_DESC_PER_PLANE           2

typedef unsigned long T_EPPI656I_HANDLE;
typedef void (*T_EPPI656I_CALLBACK)(void *pa_pClientArg);    ///< Callback function for EPPI interrupt
typedef unsigned long (*T_EPPI656I_GET_TIME_STAMP)(void *pa_pClientArg);    ///< Get time stamp function
typedef void (*EPPI656I_FN_CLOSE_DEVICE)(unsigned long pa_pHwDevHdl);


typedef enum {
    EPPI656I_BUFFER_OVERWRITE,
    EPPI656I_BUFFER_NOT_OVERWRITE
} T_EPPI656I_BUFFER_MODE;

typedef struct {
    unsigned char cEPPInr;
    unsigned long nEPPIIVG;            ///< EPPI IVG 0 = standard
    unsigned long nErrorIVG;          ///< EPPI Error IVG 0 = standard 
    unsigned long nDMAErrorIVG;       ///< DMA Error IVG 0 = standard
    unsigned char cNumberOfFrames;
    T_AV_VIDEOFORMAT tAVformat;
    T_EPPI656I_BUFFER_MODE tBufferMode;
    T_EPPI656I_CALLBACK fnTransferCallback;  ///< This function is executed after a full frame transfer
   	T_EPPI656I_CALLBACK fnErrorCallback;    
   	T_EPPI656I_GET_TIME_STAMP fnGetTimeStamp; ///< Get time stamp function, this function is called to set a time stamp to the transfered frame. If NULL the time stamp is simply a counter incremented by 1 for each frame starting with 0. If not NULL the time stamp is calculated by the calling function.
	EPPI656I_FN_CLOSE_DEVICE fnHwDevCloseDev;///< attach specified hardware function for closing the device to EPPIcameraIn driver, if 0 no hardware attached
	unsigned long hHwDevHndl;                ///< handle of the specified hardware attached to EPPIcameraIn, if 0 no hardware attached   	   	
} T_EPPI656I_CONFIG;



typedef struct {
    unsigned char cDeviceNr;
    unsigned char   cNumberOfFrames;    ///< number of frames
    unsigned char   cEPPInr;             ///< EPPI number
    unsigned long   nEPPIIVG;            ///< EPPI IVG 0 = standard
    unsigned long   nErrorIVG;          ///< EPPI Error IVG 0 = standard
    unsigned long   nDMAErrorIVG;       ///< DMA Error IVG 0 = standard 
    unsigned char   *pcFramebuffer[FG_MAX_NOF_BUFFERED_FRAMES];  ///< Frames buffer for all frames
    ifrm_t          atFrames[FG_MAX_NOF_BUFFERED_FRAMES];        ///< Structure to get the pointer and the time stamp of each frame
    T_AV_VIDEOFORMAT tAVformat;         ///< Active Video format
    T_AV_STANDARD_DESCR tAVstandard;    ///< parameters of analog video standard
    unsigned char cFramesInBuffer;      ///< number of valid frames in buffer
    unsigned char cActiveFrameIn;       ///< 
    unsigned char cActiveFrameOut;
    bool bDequeueInProgress;            ///< flag is set if dequeue is in progress
    T_EPPI656I_BUFFER_MODE tBufferMode;  ///< 
    T_EPPI656I_CALLBACK fnTransferCallback;  ///< 
   	T_EPPI656I_CALLBACK fnErrorCallback;
   	T_EPPI656I_GET_TIME_STAMP fnGetTimeStamp; ///< Get time stamp function, this function is called to set a time stamp to the transfered frame. If NULL the time stamp is simply a counter incremented by 1 for each frame starting with 0. If not NULL the time stamp is calculated by the calling function.   	
   	unsigned long nTransmittedLines;         ///< Number of transmitted lines for the active frame
   	unsigned long nTimeStampCount;           ///< Counter for the time stamp
   	bool bGrabbingStarted;                   ///< is TRUE if a grabbing process was started, otherwise FALSE
	EPPI656I_FN_CLOSE_DEVICE fnHwDevCloseDev;///< attach specified hardware function for closing the device to EPPIcameraIn driver, if 0 no hardware attached
	unsigned long hHwDevHndl;                ///< handle of the specified hardware attached to EPPIcameraIn, if 0 no hardware attached   	
} T_EPPI656I_INST;


// prototypes
void                EPPI656Isetup(void);
void                EPPI656Icleanup(void);

T_EPPI656I_HANDLE   EPPI656Iopen(T_EPPI656I_CONFIG *pa_ptConfig, T_ERROR_CODE *pa_tError, void **pa_tFGfunctions);
T_ERROR_CODE        EPPI656Iclose(T_EPPI656I_HANDLE pa_tHndl);

char                EPPI656IsetHandle(T_EPPI656I_HANDLE pa_tHndl);
void                EPPI656IremoveHandle(T_EPPI656I_HANDLE pa_tHndl);
void                EPPI656IinitializeFrameStructure(T_EPPI656I_HANDLE pa_pClientArg);

void                EPPI656IstartGrabbing(T_EPPI656I_HANDLE pa_tHndl);
void                EPPI656IstopGrabbing(T_EPPI656I_HANDLE pa_tHndl);
void                EPPI656IstartEPPI(T_EPPI656I_HANDLE pa_tHndl);
void                EPPI656IstopEPPI(T_EPPI656I_HANDLE pa_tHndl);

float               EPPI656IgetFramerate(T_EPPI656I_HANDLE pa_tHndl);
int                 EPPI656IframesAvailable(T_EPPI656I_HANDLE pa_tHndl);
T_ERROR_CODE        EPPI656IresetDevice(T_EPPI656I_HANDLE pa_tHndl);
void                EPPI656IgetDeviceInfo(T_EPPI656I_HANDLE pa_tHndl, T_FG_VIDEO_DEVICE_INFO *pa_tInfoBlock);
T_ERROR_CODE        EPPI656IconfigDevice(T_EPPI656I_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg);

T_ERROR_CODE        EPPI656IfreeFrame(T_EPPI656I_HANDLE pa_tHndl);
ifrm_t             *EPPI656IDequeueFrame(T_EPPI656I_HANDLE pa_tHndl);



#endif
