

#ifndef __CAMERA_H_INCLUDED__
#define __CAMERA_H_INCLUDED__

typedef enum {
	CAM_RAW,
	CAM_YUV_422,
	CAM_RGB_565
} T_CAM_COLOR_MODE;


typedef enum {
	CAM_QVGA,                   //  320 x  240
	CAM_VGA,                    //  640 x  480
	CAM_SVGA,                   //  800 x  600
    CAM_XGA,                    // 1024 x  768
    CAM_SXGA,                   // 1280 x 1024
	CAM_UXGA,                   // 1600 x 1200
    CAM_QXGA,                   // 2048 x 1536
    
	CAM_CUSTOM_RES,             // not supported by all cameras
    CAM_MAX_RES,                // camera depening, not supported by all cameras
} T_CAM_RESOLUTION;


typedef enum {
	OMNIVISION,
	MICRON,
} T_CAM_MANUFACTURER;


typedef enum {
    CAM_8BIT_HVSYNC,
	CAM_10BIT_HVSYNC,
    CAM_12BIT_HVSYNC,
	CAM_16BIT_HVSYNC,
} T_CAM_DATA_INTERFACE_MODE;


typedef enum {
    CAM_PPI_SAMPLES_ON_RISING_EDGE,
    CAM_PPI_SAMPLES_ON_FALLING_EDGE,
    CAM_FS_RISING_EDGE_ASSERTED,
    CAM_FS_FALLING_EDGE_ASSERTED,
} T_CAM_DATA_MODES;




#endif
