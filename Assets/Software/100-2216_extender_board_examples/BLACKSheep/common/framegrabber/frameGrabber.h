#ifndef _FRAME_GRABBER_H_INCLUDED_
#define _FRAME_GRABBER_H_INCLUDED_

#include <datatypes.h>

#define FG_MAX_NOF_BUFFERED_FRAMES						30
#define FG_MAX_VIDEO_DEVICE_FUNCTIONS					10
#define FG_MAX_NOF_DEVICES								 4
#define FG_MAX_DEVICE_NAME_LEN							20

// error codes
#define ERR_FG_NOT_ENOUGH_MEMORY                        (ERR_GENERIC - 10)
#define ERR_FG_TOO_MANY_DEVICES_ATTACHED                (ERR_GENERIC - 20)
#define ERR_FG_INVALID_HANDLE                           (ERR_GENERIC - 30)

typedef unsigned long T_CAPTURE_DEVICE_HANDLE;
typedef float T_FG_FRAMERATE;
typedef unsigned char T_FG_COLOR_MODE;
typedef unsigned char T_FG_MODE;
typedef unsigned short T_FG_CMD;

typedef struct {
    T_FG_FRAMERATE tFramerate;
    unsigned short nXres;
    unsigned short nYres;
    T_FG_COLOR_MODE tColorMode;
    unsigned long nFrameCounter;
    unsigned long nFrameDropCounter;
} T_FG_VIDEO_DEVICE_INFO;

typedef struct iframe {
  struct iframe	*nxt;
  unsigned char	*pucFramePtr;
  unsigned int	ulTS;
} ifrm_t;							// to be compatible with the codecs from analog devices

typedef struct {
    char cDeviceName[FG_MAX_DEVICE_NAME_LEN];
    unsigned long ulControlCmd;
    void *pControlArg;
}T_FG_ARG;

// video device functions
typedef T_ERROR_CODE (*FG_FN_DEVICE_RESET)(void *pa_tVideoDeviceHndl); 
typedef T_ERROR_CODE (*FG_FN_FREE_FRAME)(void *pa_tVideoDeviceHndl);
typedef int (*FG_FN_FRAME_AVAILABLE)(void *pa_tVideoDeviceHndl);
typedef ifrm_t *(*FG_FN_DEQUEUE_FRAME)(void *pa_tVideoDeviceHndl);
typedef T_FG_FRAMERATE (*FG_FN_GET_FRAMERATE)(void *pa_tVideoDeviceHndl);
typedef void (*FG_FN_GET_VIDEO_DEVICE_INFO)(void *pa_tVideoDeviceHndl, T_FG_VIDEO_DEVICE_INFO *pa_pInfoBlock);
typedef void (*FG_FN_START_GRABBING)(void *pa_tVideoDeviceHndl);
typedef void (*FG_FN_STOP_GRABBING)(void *pa_tVideoDeviceHndl);
typedef T_ERROR_CODE (*FG_FN_CONFIG_DEVICE)(void *pa_tVideoDeviceHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg);

// enum for T_FG_CMD
enum {
	FG_CMD_SET_NOF_FRAMES_TO_HOLD,
	FG_CMD_GET_DEVICE_NAME,
    FG_CMD_CLOSE,
    FG_CMD_CONTROL_DEVICE,
    FG_CMD_PERFORM_EC,
};

// enum for T_FG_COLOR_MODE
enum {
    FG_YUV422,
    FG_RGB565,
    FG_RGB24BIT,
    FG_YUV420,
    FG_COLOR_MODE_NOT_SUPPORTED
};

// enum for video device functions
enum {
    FG_DEVICE_RESET,
    FG_FREE_FRAME,
    FG_FRAME_AVAILABLE,
    FG_DEQUEUE_FRAME,
    FG_GET_FRAMERATE,
    FG_GET_VIDEO_DEVICE_INFO,
    FG_START_GRABBING,
    FG_STOP_GRABBING,
    FG_CONFIG_DEVICE
};

// enum for T_FG_MODE
enum {
    FG_MULTI_BUFFER_MODE_NO_OVERWRITE,
    FG_MULTI_BUFFER_MODE_OVERWRITE,
    FG_SINGLE_BUFFER_MODE_NO_OVERWRITE,
    FG_SINGLE_BUFFER_MODE_OVERWRITE
};

typedef struct {
    void *pVideoDeviceHndl;
    unsigned short nVideoDeviceXres;
    unsigned short nVideoDeviceYres;
	T_FG_FRAMERATE tVideoDeviceFrameRate;
	T_FG_COLOR_MODE tVideoDeviceColorMode;
	unsigned short nFGxRes;
	unsigned short nFGyRes;
	T_FG_FRAMERATE tFGframeRate;
	T_FG_COLOR_MODE tFGcolorMode;
	void *pFrameStartAddr[FG_MAX_NOF_BUFFERED_FRAMES];		// only needed if conversion is done (color mode or frame rate), otherwise supported by underlaying video device driver
	bool bScaling;
	bool bConversion;
	char cDeviceName[FG_MAX_DEVICE_NAME_LEN];
	FG_FN_DEVICE_RESET fnDeviceReset;
	FG_FN_FREE_FRAME fnFreeFrame;
	FG_FN_FRAME_AVAILABLE fnFrameAvailable;
	FG_FN_DEQUEUE_FRAME fnDequeueFrame;
	FG_FN_GET_FRAMERATE	fnGetFramerate;
	FG_FN_GET_VIDEO_DEVICE_INFO fnGetVideoDeviceInfo;
	FG_FN_START_GRABBING fnStartGrabbing;
	FG_FN_STOP_GRABBING fnStopGrabbing;
	FG_FN_CONFIG_DEVICE fnConfigDevice;
	unsigned short nNofFramesToHold;
} T_FG_INST;

typedef struct {
	void *pVideoDeviceHndl;
	void **pVideoDeviceFunctions;
	unsigned short nNofFramesToHold;
	T_FG_MODE tOperationMode;
}T_FG_ADD_DEVICE_CONF;

// prototype declarations
void FGsetup(void);
void FGcleanup(void);
T_CAPTURE_DEVICE_HANDLE FGaddDevice(T_FG_ADD_DEVICE_CONF *pa_tFGaddConfig, T_ERROR_CODE *erResult);
T_ERROR_CODE FGchangeMode(	T_FG_FRAMERATE pa_tFGframeRate,
							T_FG_COLOR_MODE pa_tFGcolorMode,
							int pa_nFGxRes,
							int pa_nFGyRes);
T_ERROR_CODE FGremoveDevice (T_CAPTURE_DEVICE_HANDLE pa_tHndl);
ifrm_t *FGgetNextFrame(T_CAPTURE_DEVICE_HANDLE pa_tHndl);
T_ERROR_CODE FGfreeFrame(T_CAPTURE_DEVICE_HANDLE pa_tHndl);
T_ERROR_CODE FGresetVideoDevice(T_CAPTURE_DEVICE_HANDLE pa_tHndl);
int FGframesAvailable(T_CAPTURE_DEVICE_HANDLE pa_tHndl);
int FGgetXres(T_CAPTURE_DEVICE_HANDLE pa_tHndl);
int FGgetYres(T_CAPTURE_DEVICE_HANDLE pa_tHndl);
T_FG_FRAMERATE FGgetFrameRate (T_CAPTURE_DEVICE_HANDLE pa_tHndl, bool pa_bVideoDevice);
T_FG_COLOR_MODE FGgetColorMode (T_CAPTURE_DEVICE_HANDLE pa_tHndl);
void FGstartGrabbing (T_CAPTURE_DEVICE_HANDLE pa_tHndl);
void FGstopGrabbing (T_CAPTURE_DEVICE_HANDLE pa_tHndl);
unsigned long FGgetNofDroppedFrames (T_CAPTURE_DEVICE_HANDLE pa_tHndl);
char *FGgetDeviceName (T_CAPTURE_DEVICE_HANDLE pa_tHndl);
T_ERROR_CODE FGconfigDevice(T_CAPTURE_DEVICE_HANDLE pa_tHndl, T_FG_CMD pa_tCmd, T_FG_ARG *pa_tArg);
T_CAPTURE_DEVICE_HANDLE FGgetActiveDeviceHandle(void);
T_CAPTURE_DEVICE_HANDLE FGgetHandleByDevId(unsigned char pa_cDeviceId);

#endif
