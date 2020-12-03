#ifndef _AD7266_CONFIG_H_INCLUDED_
#define _AD7266_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <services/services.h>
#include "../../GPIOconfig.h"
#include "../../SPORT/sportconfig.h"

//error codes
#define ERR_AD7266_BURST_TOO_SMALL		(ERR_GENERIC - 10)
#define ERR_AD7266_CHANNEL_IN_USE		(ERR_GENERIC - 20)
#define ERR_AD7266_NOT_ENOUGH_MEMORY	(ERR_GENERIC - 30)
#define ERR_AD7266_BUFFER_MODE  	    (ERR_GENERIC - 40)
#define ERR_AD7266_CHANNEL  			(ERR_GENERIC - 50)
#define ERR_AD7266_HOOK_INTERRUPT    	(ERR_GENERIC - 60)

#define AD7266_FASTEST_SAMPLING         (1 << 0)

// general max ratings of AD7266
#define AD7266_MAX_TRANSFER_CLOCK   32000000              // max. serial interface transfer frequency
#define AD7266_MIN_RFS_DIV          14
#define AD7266_USE_DEFAULT_SPORT        0xffff


typedef unsigned long T_AD7266_FLAGS;

typedef void (*T_AD7266_CALLBACK)(void *);

/**     
*       @brief enum for BufferMode selection
**/
typedef enum {
    AD7266_SINGLE_BUFFER = 1,
    AD7266_PINGPONG_BUFFER
} T_AD7266_BUFFER_MODE;


/**     
*       @brief struct for AD7266 management
**/
typedef struct {
//	unsigned short **ppBufferBase;                   ///< Buffer that can be accessed in PingPong Buffer Mode, in Single Buffer Mode AD7266 has to be stoped
	unsigned short nMaxElements;
	unsigned short nCurrElement;
	unsigned long nRxIVG;
    unsigned long nDMAerrIVG;
    unsigned long nSPORTerrIVG;
	unsigned short nCurrBuffer;
	T_AD7266_CALLBACK fnCallback;
	T_AD7266_BUFFER_MODE nBufferMode;
	unsigned short nOffset;
    unsigned short nStride;
    unsigned short *pBufferBase0;
    unsigned short *pBufferBase1;
	bool bIsActive;
	unsigned short nSPORT;
	bool bInUse;
} T_AD7266_SPEC;


typedef struct {
    unsigned short nChannel;                            ///< select the channel of AD7266 between 1 and 6
	unsigned long nSamplingRate;                        ///< Sampling Rate, in Samples per second
    unsigned short nMaxElements;                        ///< number of Samples in the Buffer indipendent of the stride or offset
    unsigned short nStride;
    unsigned short nOffset;
    T_AD7266_CALLBACK fnCallback;  						///< user callback Function
    T_GPIO_MASK adr0;            					    ///< address pins of AD7266 multiplexer
    T_GPIO_MASK adr1;
    T_GPIO_MASK adr2;
    unsigned long nSclk;                                ///< system clock
    unsigned short *pBufferBase0;                       ///< base address of buffer 0
    unsigned short *pBufferBase1;                       ///< base address of buffer 1
    float fRealSamplingRate;                            ///< the real sampling rate, is written by the setup function
    T_AD7266_BUFFER_MODE nBufferMode;                   ///< selects if one single buffer or a pingpong buffer should be used
    unsigned long nSPORTperIVG;                         ///< custom IVGs, if 0 use default
    unsigned long nDMAerrIVG;
    unsigned long nSPORTerrIVG;
    T_AD7266_FLAGS nFlags;
    unsigned short nSPORT;                              ///< selects the SPORT to which the AD7266 is connected, if EXT-AD-DA Extender Board is used, use 0xffff for SPORT
    void *__p_reserved;
} T_AD7266_CONFIG;


//prototype declarations
ADI_INT_HANDLER_RESULT AD7266rxHandler(void *pa_pClientArg);

T_ERROR_CODE AD7266Setup(T_AD7266_CONFIG *pa_AD7266Config);

void AD7266Start(void);

void AD7266Stop(void);

void AD7266Cleanup(void);

unsigned short AD7266GenerateClkDiv(    unsigned long pa_nSamplingRate,
                                        unsigned long pa_nSclk);
                                        
unsigned short AD7266GenerateRFSDIV(    unsigned long pa_nSamplingRate,
                                        unsigned long pa_nRealRclkFreq);


#endif
