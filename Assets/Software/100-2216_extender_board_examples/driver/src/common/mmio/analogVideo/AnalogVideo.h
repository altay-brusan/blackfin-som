/**
    @file AnalogVideo.h
    @ingroup video
    
    @brief Common functions and paramters  used for analog video operations 
    
    
    
    BLT_DISCLAIMER(TBD)
    @author Thomas Maier
    @version 1.0
    @date 30.06.2009
    
    @startcond Changelog
    
    @endcond
**/

#ifndef _ANALOGVIDEO_H_INCLUDED_
#define _ANALOGVIDEO_H_INCLUDED_

#include <Datatypes.h>
#include "../../../../../BLACKSheep/common/vdm/VDMconfig.h"

#define AV_FLAG_NUM                     7

/**
    @brief Available analog video formats
**/
typedef enum {
    AV_UNKNOWN,
    AV_PAL,
    AV_NTSC,
    AV_PAL_BGH,
    AV_SECAM,
    AV_PAL_N,
    AV_SECAM_525,
    AV_PAL_60,
} T_AV_VIDEOFORMAT;

// defines for the ITU656 interface standard
typedef enum {
	/// Flag for field 1
	AV_FIELD_1			= 0x00, 
	/// Flag for field 2
	AV_FIELD_2			= 0x40,
	AV_FIELD			= AV_FIELD_2,
	///Vertical Blank flag
	AV_VERTICAL_BLANK	= 0x20,
	AV_HORIZ_BLANK		= 0x10
	
} T_AV_ITU656_FLAGS;


/**
    @brief Different types of analog inputs
**/
typedef enum {
    AV_INPUT_CVBS,      ///< Composite
    AV_INPUT_YC,        ///< S-Video
    AV_INPUT_YPRPB,     ///< YPrPb Components 
    
} T_AV_INPUT_TYPE;

/**
    @brief descriptor for an analog video source
**/
typedef struct {
    char acDescriptor[32];          ///< human readable Name 
    T_AV_INPUT_TYPE tInputType;     ///< Physical type of input  
    T_AV_VIDEOFORMAT tVideoType;    ///< Video Format   
    bool bInterlaced;               ///< interlaced video
    bool bConnected;                ///< is there an signal at this input
    T_ERROR_CODE erErrorCode;       ///< ErrorCode if e.g. detection of video standard fails
    unsigned char acOptions[4];     ///< Videocodec specific options 
    
} T_AV_SOURCE;


typedef struct {
     void *start_addr;                 ///< start address of buffer
     unsigned short width;             ///< surface width
     unsigned short height;            ///< surface height
     unsigned long offset;             ///< address ofset of first visible pixel
     unsigned short pixel_stride;      ///< address diffecence between neighbouring pixels
     unsigned short line_stride;       ///< address difference between two consecutive line starts
     unsigned long field_stride;       ///< address difference between the start if field 1 and 2
     unsigned short interlaced : 1;    ///< interlaces surface
} T_AV_FRAME_BUFFER_DESC;

/**
    @brief This contains the information necessary to work with various video standards
 **/
typedef struct
{
    long nWidth;                        ///< active video width
    long nHeight;                       ///< active video height
    long nPixelStride;                  ///< address difference between two neighbouring pixels
    long nLineStride;                   ///< address difference between two line starts
    long nFieldStride;                  ///< address difference between two field starts
    long nFrameStride;                  ///< address difference between two frame starts
    long nActiveVideo1Offset;           ///< active video start address offset in first field
    long nActiveVideo2Offset;           ///< active video start address offset in second field
    long nActiveVideoLineSize;          ///< size in bytes of an active video line
    long nBlankLineSize;                ///< size in bytes of blanking region in a line
    long nTotalLines;                   ///< total number of lines (active & blank)
    struct {
        long nLine;                     ///< line where flags change
        unsigned long nFlags;           ///< new flags or'ed together
    } tFlags[AV_FLAG_NUM];              ///< flag change positions
} T_AV_STANDARD_DESCR;



T_ERROR_CODE AV_CopyYUV422toITUR656Frame(T_AV_FRAME_BUFFER_DESC* pa_tDestBuffDesc, 
                                 unsigned short pa_nXOffset,
                                 unsigned short pa_nYOffset,
                                 char* pa_pcYUV422Start, 
                                 unsigned short pa_nWidth, 
                                 unsigned short pa_nHeight, 
                                 unsigned char pa_cMdmaChannel);


#endif

