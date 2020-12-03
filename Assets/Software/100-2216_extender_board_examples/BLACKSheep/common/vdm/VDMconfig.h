/**
	@file VDMconfig.h
	@ingroup vdm
	@brief Video Device Manager
	
	This File Contains functions to ad multiple video devices and according manipulation routines
	
	BLT_DISCLAIMER(TBD)
	@author Roland Oberhammer
	@version 1.1
	@date 02.12.2008
**/

#ifndef _VDM_CONFIG_H_INCLUDED_
#define _VDM_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <stdarg.h>

// error codes
#define ERR_VDM_NOT_ENOUGH_MEMORY		(ERR_GENERIC - 10)
#define ERR_VDM_INVALID_HANDLE			(ERR_GENERIC - 20)
#define ERR_VDM_INVALID_NOF_PLANES		(ERR_GENERIC - 30)
#define ERR_VDM_FUNC_NOT_SUPPORTED      (ERR_GENERIC - 31)
#define ERR_VDM_DEVICE_NOT_FOUND        (ERR_GENERIC - 32)
#define ERR_VDM_TO_MANY_DEVICES         (ERR_GENERIC - 33)

typedef signed long T_VD_HANDLE;
typedef unsigned long T_VD_COLOR;
typedef unsigned char T_VD_TYPE;
typedef unsigned char T_VD_COLOR_FORMAT;
typedef unsigned long T_VD_FLAGS;
typedef unsigned long T_VD_FILL;
typedef unsigned long T_VD_VIDEO_FORMAT;

typedef void (*T_VD_SET_PIXEL)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor);
typedef T_VD_COLOR (*T_VD_GET_PIXEL)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY);
typedef void (*T_VD_BLEND_PIXEL)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha);
typedef void (*T_VD_DRAW_LINE)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_nX1, unsigned short pa_nY1, unsigned short pa_nX2, unsigned short pa_nY2, T_VD_COLOR pa_nColor);
typedef void (*T_VD_DRAW_CIRCLE)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_nXm, unsigned short pa_nYm, unsigned short pa_nRadius, T_VD_COLOR pa_nColor);
typedef void (*T_VD_DRAW_POLYGON)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_nXstart, unsigned short pa_nYstart, ...);
typedef void (*T_VD_FILL_SURFACE)(void *pa_pDevHndl, unsigned char pa_cPlane, unsigned short pa_UpperX, unsigned short pa_UpperY, T_VD_COLOR pa_nColor, T_VD_FILL pa_tFillType);
typedef void (*T_VD_CLEAR_SCREEN)(void *pa_pDevHndl, unsigned char pa_cPlane, T_VD_COLOR pa_nColor);
typedef T_ERROR_CODE (*T_VD_FSYNC_WAIT)(void *pa_pDevHndl);
typedef bool (*T_VD_FSYNC_STAT)(void *pa_pDevHndl);
typedef T_ERROR_CODE (*T_VD_SHOW_FRAME)(void *pa_pDevHndl, unsigned char *pa_pcFrameBuffer, unsigned short pa_nXstart, unsigned short pa_nYstart, unsigned short pa_nXsize, unsigned short pa_nYsize);
typedef T_ERROR_CODE (*T_VD_SET_ACTIVE_PLANE)(void *pa_pDevHndl, unsigned char pa_cPlane);
typedef signed char (*T_VD_GET_ACTIVE_PLANE)(void *pa_pDevHndl);
typedef signed char (*T_VD_GET_NEXT_PLANE)(void *pa_pDevHndl);

typedef T_ERROR_CODE (*VDM_FN_CLOSE)(void *pParam);

#define VDM_MAX_NOF_DEVICES				5
#define VDM_MAX_NOF_PLANES				8
#define VDM_MAX_NOF_COLOR_FORMATS		5
#define VDM_MAX_NOF_GRAFIC_FUNCTIONS	25

// defines for T_VD_COLOR_FORMAT;
enum {
	VD_YUV422 = 0,
	VD_RGB565,
	VD_RGB24,
	VD_YUV420
};

// defines for T_VD_TYPE;
enum {
	VD_TFT,
	VD_VIDEO_ITU656,
	VD_RGB565_FB,
	VD_YUV422_FB,
	VD_YUV420_FB
};

// defines for T_VD_FLAGS
enum {
	VD_FILL
};

// defines for T_VD_FILL
enum {
  VD_NO_FILL,
	VD_FILL_FULL
};

// defines for function assignment
enum {
    VD_SET_PIXEL = 0,
    VD_GET_PIXEL,
    VD_BLEND_PIXEL,
    VD_DRAW_LINE,
    VD_DRAW_CIRCLE,
    VD_DRAW_POLYGON,
    VD_FILL_SURFACE,
    VD_CLEAR_SCREEN,
    VD_FSYNC_WAIT,
    VD_FSYNC_STAT,
	VD_OVERLAY_BUFFER,
	VD_SHOW_FRAME,
	VD_SET_ACTIVE_PLANE,
	VD_GET_ACTIVE_PLANE,
	VD_GET_NEXT_PLANE
};

// defines for T_VD_VIDEO_FORMAT
enum {
    VD_PAL,
    VD_NTSC,
    VD_RAW_PROGRESSIVE,
    VD_RAW_PLANAR
};

typedef struct {
    void *pFrameBuffer;
    unsigned short nHWxRes;
    unsigned short nHWyRes;
    unsigned short nCurrXres;
    unsigned short nCurrYres;
    void *pSyncFlag;
    unsigned char cBytesPerPixel;
    T_VD_COLOR_FORMAT tColorFormat;
    T_VD_TYPE tDeviceType;
    char *pcDeviceName;
    void *fGraficFunction[VDM_MAX_NOF_GRAFIC_FUNCTIONS];
    unsigned char cNofPlanes;
    void *pPlaneBuffer[VDM_MAX_NOF_PLANES];
    unsigned char cActivePlane;
    void *pDevHndl;
    bool bOverlayReady;
    VDM_FN_CLOSE fnCloseDevice;
}T_VD_INST;

typedef struct {
    T_VD_HANDLE atDevHndl[VDM_MAX_NOF_DEVICES];
    unsigned char cNofDevices;
    T_VD_INST *pActiveDevInst;
    T_VD_HANDLE tActiveDevHndl;
}T_VDM_SPEC;



///< Configuration structure for the VDMaddDevice
typedef struct {
    void *pDeviceHndl;                  ///< Handle to the Frame Buffer Driver
	T_VD_TYPE tVideoDeviceType;         ///< Type of the Video Out Device
	unsigned short nHWxRes;             ///< X Hardware Resolution of the Device
	unsigned short nHWyRes;             ///< Y Hardware Resolution of the Device
	unsigned short nXresInit;           ///< X Resolution of the input video
	unsigned short nYresInit;           ///< Y Resolution of the input video
	unsigned char cBytesPerPixel;       ///< Bytes per Pixel which the format requires
	bool bOverlayReady;                 ///< True if device is able to show overlay, otherwise VDM should to that task
	T_VD_COLOR_FORMAT tColorFormat;     ///< Color Format of the Video Device
	void *pSyncFlag;                    ///< Start Adress of the Frame Buffer
	char *pcDeviceName;                 ///< Sync Flag for display update
	void **pfGraficFunctions;           ///< Array with pointers to the graphic functions, normally supplied by the Frame Buffer Driver
	unsigned char cNofPlanes;           ///< Number of Planes to use
	void **pPlaneBuffer;                ///< Array with pointers to the Plane Buffers
	unsigned char cCurrActivePlane;     ///< Current active plane
	VDM_FN_CLOSE fnCloseDevice;         ///< Function pointer to the close function of the device
} T_VDM_CONFIG;


// prototype declarations
void VDMsetup(void);
void VDMsetPixel(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor);
void VDMADsetPixel(unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor);
T_VD_COLOR VDMgetPixel(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY);
T_VD_COLOR VDMADgetPixel(unsigned short pa_nX, unsigned short pa_nY);
void VDMblendPixel(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha);
void VDMADblendPixel(unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor, unsigned char pa_cAlpha);
bool VDMgetFrameSyncStat(T_VD_HANDLE pa_tHndl);
bool VDMADgetFrameSyncStat(void);
T_ERROR_CODE VDMwaitForFrameSync(T_VD_HANDLE pa_tHndl);
T_ERROR_CODE VDMADwaitForFrameSync(void);
void VDMdrawLine(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane, unsigned short pa_nX1, unsigned short pa_nY1, unsigned short pa_nX2, unsigned short pa_nY2, T_VD_COLOR pa_nColor);
void VDMADdrawLine(unsigned short pa_nX1, unsigned short pa_nY1, unsigned short pa_nX2, unsigned short pa_nY2, T_VD_COLOR pa_nColor);
void VDMfillSurface(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane, unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor, T_VD_FILL pa_tFillType);
void VDMADfillSurface(unsigned short pa_nX, unsigned short pa_nY, T_VD_COLOR pa_nColor, T_VD_FILL pa_tFillType);
void VDMclearScreen(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane, T_VD_COLOR pa_nColor);
void VDMADclearScreen(T_VD_COLOR pa_nColor);
T_VD_HANDLE VDMaddDevice(T_VDM_CONFIG *pa_tConf,
                         T_ERROR_CODE *tErr,
                         unsigned char *pa_ucDeviceNr,
                         void *pa_pGeneric);
T_ERROR_CODE VDMremoveDevice(T_VD_HANDLE pa_tHndl);
T_ERROR_CODE VDMsetActiveDevice(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane);
unsigned char VMDgetBytesPerPixel(T_VD_HANDLE pa_tHndl);
unsigned char VMDADgetBytesPerPixel(void);
T_ERROR_CODE VDMADsetActivePlane(unsigned char pa_cPlane);
T_ERROR_CODE VDMsetActivePlane(T_VD_HANDLE pa_tHndl, unsigned char pa_cPlane);
signed char VDMgetActivePlane(T_VD_HANDLE pa_tHndl);
signed char VDMADgetActivePlane(void);
signed char VDMgetNextPlane(T_VD_HANDLE pa_tHndl);
signed char VDMADgetNextPlane(void);
unsigned short VDMgetXres(T_VD_HANDLE pa_tHndl);
unsigned short VDMgetYres(T_VD_HANDLE pa_tHndl);
unsigned short VDMADgetXres(void);
unsigned short VDMADgetYres(void);
unsigned short VDMgetColorFormat(T_VD_HANDLE pa_tHndl);
unsigned short VDMADgetColorFormat(void);
T_ERROR_CODE VDMshowFrame(T_VD_HANDLE pa_tHndl, unsigned char *pa_pcFrameBuffer, unsigned short pa_nXoffset, unsigned short pa_nYoffset, unsigned short pa_nXsize, unsigned short pa_nYsize);
T_VD_HANDLE VDMADgetHandle(void);

#endif
