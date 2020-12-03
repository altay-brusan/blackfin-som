/**
	@file  ext_video_av_def.c
	@ingroup video
	@brief Definitions of analog video sources for Video Extender boards V1.X

	
		
	
	
	BLT_DISCLAIMER(TBD)
	@author Thomas Maier
	@version 1.0
	@date 12.05.2009
	
	@startcond Changelog
	
	@endcond
**/

#include "../../mmio/AnalogVideo/AnalogVideo.h"

//channel definitions for switch input
#define EXT_VIDEO_INPUT_CVBS        0
#define EXT_VIDEO_INPUT_SVIDEO      1
#define EXT_VIDEO_INPUT_COMPONENTS  2

T_AV_SOURCE T_EXT_VIDEO_AV_SOURCES_SPEC[] = {
 
    {
        "CVBS input AIN3",
        AV_INPUT_CVBS, 
        AV_UNKNOWN,
        0,
        0,           
        0,   
        {0x02,0,0,0},
    },
     {
        "S-VIDEO input AIN1/4",
        AV_INPUT_YC,
        AV_UNKNOWN,
        0,
        0,          
        0,  
        {0x06,0,0,0},
    },
    {
        "YCrCb input AIN2/5/6",
        AV_INPUT_YPRPB,
        AV_UNKNOWN,
        0,
        0,            
        0,   
        {0x0A,0,0,0},
    },
   
    

};

unsigned char cEXT_VIDEO_AVsourcesNum = sizeof(T_EXT_VIDEO_AV_SOURCES_SPEC) / sizeof(T_AV_SOURCE);
