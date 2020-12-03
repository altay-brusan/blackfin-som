#ifndef _AD5415_CONFIG_H_INCLUDED_
#define _AD5415_CONFIG_H_INCLUDED_

#include <datatypes.h>
#include <services/services.h>
#include "../../../../../driver/src/common/GPIOconfig.h"

//error codes
#define ERR_AD5415_BURST_TOO_SMALL		(ERR_GENERIC - 10)
#define ERR_AD5415_CHANNEL_IN_USE		(ERR_GENERIC - 20)
#define ERR_AD5415_NOT_ENOUGH_MEMORY	(ERR_GENERIC - 30)
#define ERR_AD5415_BUFFER_MODE  	    (ERR_GENERIC - 40)
#define ERR_AD5415_CHANNEL  			(ERR_GENERIC - 50)
#define ERR_AD5415_HOOK_INTERRUPT    	(ERR_GENERIC - 60)

// general max ratings of AD5415
#define AD5415_MAX_TRANSFER_CLOCK   50000000              // max. serial interface transfer frequency
#define AD5415_MIN_TFS_DIV          17
#define AD5415_USE_DEFAULT_SPORT    0xffff                // flag for using the standard SPORT


// prototype for user callback function
typedef void (*T_AD5415_CALLBACK)(void *);


/**     
*       @brief enum for BufferMode selection
**/
typedef enum {
    AD5415_SINGLE_BUFFER = 1,
    AD5415_PINGPONG_BUFFER
} T_AD5415_BUFFER_MODE;


/**     
*       @brief enum for ChannelMode selection
**/
typedef enum {
    AD5415_SINGLE_CHANNEL = 1,
    AD5415_DUAL_CHANNEL
} T_AD5415_CHANNEL_MODE;


/**     
*       @brief struct for AD5415 managment
**/
typedef struct {
    unsigned short nMaxElements;
    unsigned short **ppBufferBase;
    unsigned short nCurrBuffer;
    unsigned long nTxIVG;
    unsigned long nDMAerrIVG;
    unsigned long nSPORTerrIVG;
    T_AD5415_CALLBACK fCallback;
    T_AD5415_BUFFER_MODE nBufferMode;
    T_AD5415_CHANNEL_MODE nChannelMode;
    bool bIsActive;
    unsigned short nBurstSize;
    unsigned short nOffset;
    unsigned short nStride;
    unsigned short *pBufferBase0;
    unsigned short *pBufferBase1;
    unsigned short nSPORT;
    bool bInUse;
} T_AD5415_SPEC;


/**     
*       @brief struct for AD5415 configuration
**/
typedef struct {
    T_AD5415_CHANNEL_MODE nChannelMode;
	unsigned long nSamplingRate;
	unsigned short nMaxElements; 
	unsigned short nStride;
    unsigned short nOffset; 
	T_AD5415_CALLBACK fnCallback;
	unsigned long nSclk; 
	unsigned short *pBufferBase0; 
	unsigned short *pBufferBase1;
	float fRealSamplingRate;
	T_AD5415_BUFFER_MODE nBufferMode; 
    unsigned long nSPORTperIVG;
    unsigned long nDMAerrIVG;
    unsigned long nSPORTerrIVG;
    unsigned short nSPORT;
    void *__p_reserved;
} T_AD5415_CONFIG;


// prototype declarations
ADI_INT_HANDLER_RESULT SPORTtxHandler(void *pa_pClientArg);

T_ERROR_CODE AD5415Setup(T_AD5415_CONFIG *pa_aAD5415Config);
							
void AD5415Start(void);

void AD5415Stop(void);

T_ERROR_CODE PrepareAD5415Buffer(   unsigned short *pa_pBufferBase,
                                    unsigned short pa_nBurstSize,
                                    unsigned short pa_nStride,
                                    unsigned short pa_nOffset,
                                    T_AD5415_CHANNEL_MODE pa_nChannelMode);
                                    
void AD5415Cleanup(void);

unsigned short AD5415GenerateClkDiv(    unsigned long pa_nSamplingRate,
                                        T_AD5415_CHANNEL_MODE pa_nChannelMode,
                                        unsigned long pa_nSclk);

unsigned short AD5415GenerateTFSDIV(    unsigned long pa_nSamplingRate,
                                        T_AD5415_CHANNEL_MODE pa_nChannelMode,
                                        unsigned long pa_nRealTclkFreq);


#endif
