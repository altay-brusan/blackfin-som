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

#include "AnalogVideo.h"
#include "../../mdma/MDMAconfig.h"
#include <cycle_count_bf.h>

extern unsigned int g_nMDMAcount;		//declared in mdma_global.c
extern T_MDMA_SPEC g_aMDMAspec[];		//declared in mdma_global.c

// known video standards --------------------------------------
T_AV_STANDARD_DESCR g_tPALstd = {                       ///< structure describing the PAL standard
    720, 
    576, 
    2, 
    1728, 
    540864, 
    1080000, 
    38304, 
    579168, 
    1440, 
    280,
    625,
    {
        { 0,   AV_FIELD_1 | AV_VERTICAL_BLANK },
        { 22,  AV_FIELD_1                  },              
        { 310, AV_FIELD_1 | AV_VERTICAL_BLANK },
        { 312, AV_FIELD_2 | AV_VERTICAL_BLANK },
        { 335, AV_FIELD_2                  },              
        { 623, AV_FIELD_2 | AV_VERTICAL_BLANK },
        { -1, 0 }
    }
};

T_AV_STANDARD_DESCR g_tNTSCstd = {                      ///< structure describing the NTSC standard
    720, 
    507, 
    2, 
    1716, 
    451308, 
    900900, 
    5424, 
    456732, 
    1440, 
    268,
    525,
    {
        { 0,    AV_FIELD_2 | AV_VERTICAL_BLANK    },
        { 3,    AV_FIELD_1 | AV_VERTICAL_BLANK    },
        { 19,   AV_FIELD_1                     },
        { 263,  AV_FIELD_1 | AV_VERTICAL_BLANK    },
        { 265,  AV_FIELD_2 | AV_VERTICAL_BLANK    },
        { 282,  AV_FIELD_2                     },
        { -1, 0 }
    }
};
// --------------------------------------------------------------

   
char g_acAVstrings[8][16] = {
    "unknown",
    "PAL",
    "NTSC",
    "PAL BGH",
    "SECAM",
    "PAL N",
    "SECAM 525",
    "PAL 60",
    
};

static const unsigned char AVprotectionBitTable[] = {
	0x00,
	0x0d,
	0x0b,
	0x06,
	0x07,
	0x0a,
	0x0c,
	0x01
};   


/**
 * @brief   Generates four-byte SAV ( = Start of Active Video, H = 0 )
 *          synchronization sequence with flags and protection bits.
 * 
 * @param flags A combination of @ref FIELD_1, @ref FIELD_2 and @ref VERTICAL_BLANK
 *              or'ed together.
 */
unsigned long AVsav( unsigned long pa_nFlags ) {
	pa_nFlags |= AVprotectionBitTable[ pa_nFlags >> 4 ];
	pa_nFlags |= 0x80;
		
	return 0x000000ff | pa_nFlags << 24;
}


/**
 * @brief   Generates four-byte EAV ( = End of Active Video, H = 1 )
 *          synchronization sequence with flags and protection bits.
 * 
 * @param flags A combination of @ref FIELD_1, @ref FIELD_2 and @ref VERTICAL_BLANK
 *              or'ed together.
 */
unsigned long AVeav( unsigned long pa_nFlags ) {
	pa_nFlags |= AV_HORIZ_BLANK;
	pa_nFlags |= AVprotectionBitTable[ pa_nFlags >> 4 ];
	pa_nFlags |= 0x80;
	
	return 0x000000ff | pa_nFlags << 24;
}

/**
    @brief: Initializes a buffer with a video matrix depending on video link type and format.
**/
void AVinitITU656Matrix ( unsigned char *pa_pcBuffer, T_AV_STANDARD_DESCR *pa_pStd ) {
    
    
    unsigned short blank = 0x1080;
    unsigned long sav, eav;
    unsigned char *iterator;
    unsigned short line;
    
    long i;
    for( i = 0; i < ((pa_pStd->nLineStride  * pa_pStd->nTotalLines)>>1); i++ ) {
       *( (unsigned short *)pa_pcBuffer + i ) = blank;
    }
    
    line = 0;
    iterator = pa_pcBuffer;
    
    int flagno = 0;
    long matchln;
    
    matchln = pa_pStd->tFlags[flagno].nLine;
    
    do {
        if( line == matchln )
        {
            sav = AVsav( pa_pStd->tFlags[flagno].nFlags );
            eav = AVeav( pa_pStd->tFlags[flagno].nFlags );
            flagno++;
            matchln = pa_pStd->tFlags[flagno].nLine;
        }
        
        *(unsigned long *)iterator = eav;
        *(unsigned long *)( iterator + pa_pStd->nBlankLineSize - 4 + 8 ) = sav;
        iterator += pa_pStd->nLineStride;
        line++;
    } while( line < pa_pStd->nTotalLines );
  
}

/**
	@brief Copies the video Information of one ITU565 frame to an other
	
	@param * pa_tSrcBuffDesc Input Buffer Descriptor
	@param * pa_tDestBuffDesc  Output Buffer Descriptor
	@param pa_nMdmaChannel   MDMA Channel to use for the copy process, if 0xFF MDMA wont be used -> slow
	
	@return Number of substrings found
	
**/
#pragma section("L1_code")
T_ERROR_CODE AV_CopyYUV422toITUR656Frame(T_AV_FRAME_BUFFER_DESC* pa_tDestBuffDesc, 
                                 unsigned short pa_nXOffset,
                                 unsigned short pa_nYOffset,
                                 char* pa_pcYUV422Start, 
                                 unsigned short pa_nWidth, 
                                 unsigned short pa_nHeight, 
                                 unsigned char pa_cMdmaChannel) {
    T_ERROR_CODE tErr = ERR_GENERIC;
    unsigned long long ullMeasureTime1 = 0;
    unsigned long long ullMeasureTime2 = 0;
    unsigned long long ullMeasureTime3 = 0;
    
    //_GET_CYCLE_COUNT(ullMeasureTime1);
    
    if(pa_cMdmaChannel != 0xFF && !g_aMDMAspec[pa_cMdmaChannel].bInUse && (pa_cMdmaChannel < g_nMDMAcount)) {
        /// cover all combinations of input interlaced/progressive modes
        if(pa_tDestBuffDesc->interlaced) {
            // dest interlaced -> 2 MDMA transfers
            
//#ifdef __ADSPBF561__
        
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_nWidth * 2 + 4; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride - pa_nWidth * 2 + 4; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2) & 0xFFFFFFFC);
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0038;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x003A;
 /*       
#else    
            
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_nWidth*2 +2; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride - pa_nWidth*2 + 2; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2) & 0xFFFFFFFC);
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0034;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x0036;
            
#endif    
*/        
            // enable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) |= 0x0001;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) |= 0x0001;
            
            // wait for mdma
            while ((*(g_aMDMAspec[pa_cMdmaChannel].pDestIrqStat) & 0x0008) == 0x0008);
            // disable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) &= 0xfffe;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) &= 0xfffe;
            
            
//#ifdef __ADSPBF561__    

            // second field
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_nWidth*2 + 4; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start + pa_nWidth * 2;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride - pa_nWidth * 2 + 4; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + pa_tDestBuffDesc->field_stride + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2 ) & 0xFFFFFFFC);
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0038;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x003A;
 /*                               
#else
                    
            // second field
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_nWidth*2 +2; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start + pa_nWidth*2;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride - pa_nWidth*2 + 2; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + pa_tDestBuffDesc->field_stride + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2 ) & 0xFFFFFFFC);
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0034;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x0036;
            
#endif
*/            
            // enable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) |= 0x0001;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) |= 0x0001;
            // wait for mdma
            while ((*(g_aMDMAspec[pa_cMdmaChannel].pDestIrqStat) & 0x0008) == 0x0008);
            // disable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) &= 0xfffe;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) &= 0xfffe;
            
            
        } else {
            
//#ifdef __ADSPBF561__
        
            // dest progressive -> 2 MDMA transfers
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2) & 0xFFFFFFFC) ;
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0038;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x003A;        
 /*       
#else
            
            // dest progressive -> 2 MDMA transfers
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2) & 0xFFFFFFFC) ;
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0034;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x0036;
            
#endif
*/            
            // enable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) |= 0x0001;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) |= 0x0001;
            // wait for mdma
            while ((*(g_aMDMAspec[pa_cMdmaChannel].pDestIrqStat) & 0x0008) == 0x0008);
            // disable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) &= 0xfffe;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) &= 0xfffe;
            
            
//#ifdef __ADSPBF561__
        
            // second field
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start + pa_nWidth*pa_nHeight;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 4;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight / 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + pa_tDestBuffDesc->line_stride + pa_tDestBuffDesc->field_stride + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2 ) & 0xFFFFFFFC);
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0034;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 32 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x0036;        
 /*       
#else    
            
            // second field
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceStartAddr) = (char*)pa_pcYUV422Start + pa_nWidth*pa_nHeight;

            *(g_aMDMAspec[pa_cMdmaChannel].pDestXCount)    = pa_nWidth;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestXModify)   = 2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYCount)    = pa_nHeight/2;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestYModify)   = pa_tDestBuffDesc->line_stride; 
            *(g_aMDMAspec[pa_cMdmaChannel].pDestStartAddr) = (char*) (((unsigned long)pa_tDestBuffDesc->start_addr + pa_tDestBuffDesc->offset + pa_tDestBuffDesc->line_stride + pa_tDestBuffDesc->field_stride + (pa_nXOffset<<1) + pa_nYOffset*pa_tDestBuffDesc->line_stride + 2 ) & 0xFFFFFFFC);
            
            // configure source config: stop mode, 2D no interrupt, synchronized, memory read, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) = 0x0034;
            // configure destination config: stop mode, 2D no interrupt, synchronized, memory write, 16 bit
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) = 0x0036;
            
#endif
 */           
            // enable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) |= 0x0001;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) |= 0x0001;
            // wait for mdma
            while ((*(g_aMDMAspec[pa_cMdmaChannel].pDestIrqStat) & 0x0008) == 0x0008);
            // disable mdma
            *(g_aMDMAspec[pa_cMdmaChannel].pSourceConfig) &= 0xfffe;
            *(g_aMDMAspec[pa_cMdmaChannel].pDestConfig) &= 0xfffe;
        }
        g_aMDMAspec[pa_cMdmaChannel].bInUse = false; 
        
        //return ERR_NONE;
    } else {
        /// use loops and memcpy for transfer  
        
        
        
    }
    
   // _GET_CYCLE_COUNT(ullMeasureTime2);
   // ullMeasureTime3 = (unsigned long long)((double)(ullMeasureTime2 - ullMeasureTime1) * (1.0 / 500000.0));
    return ERR_NONE;
}
