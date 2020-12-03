#ifndef _AD5405_CONFIG_INCLUDED_
#define _AD5405_CONFIG_INCLUDED_

#include <datatypes.h>
#include <services/services.h>
#include "../../../../../driver/src/common/GPIOconfig.h"

//error  codes
#define ERR_AD5405_BURST_TOO_SMALL		(ERR_GENERIC - 10)
#define ERR_AD5405_CHANNEL_IN_USE		(ERR_GENERIC - 20)
#define ERR_AD5405_NOT_ENOUGH_MEMORY	(ERR_GENERIC - 30)
#define ERR_AD5405_BUFFER_MODE  	    (ERR_GENERIC - 40)
#define ERR_AD5405_CHANNEL  			(ERR_GENERIC - 50)
#define ERR_AD5405_HOOK_INTERRUPT    	(ERR_GENERIC - 60)
#define ERR_AD5405_RINGBUFFER_FULL    	(ERR_GENERIC - 70)

// global max settings for AD5405
#define AD5405_MAX_SAMPLE_RATE            200000         // [Sps] Samples per second
#define AD5405_DEFAULT_MAX_CHANNELS            2
#define AD5405_MIN_PULSE                     200



/**     
*       @brief enum for ChannelMode selection
**/
typedef enum {
    AD5405_SINGLE_CHANNEL = 1,
    AD5405_DUAL_CHANNEL
}T_AD5405_CHANNEL_MODE;



/**     
*       @brief enum for Buffer Empty Strategy
**/
typedef enum {
    AD5405_BUFFER_EMPTY_WRITE_0,            // write 0x000, zero output
    AD5405_BUFFER_EMPTY_WRITE_1,            // write 0x0fff, max. output
    AD5405_BUFFER_EMPTY_WRITE_LAST_SAMPLE,   // write last sample to AD5405
    AD5405_BUFFER_EMPTY_AUTOBUFFER          // if buffer is empty, start from the beginning again
}T_AD5405_BUFFER_EMPTY_MODE;



/**     
*       @brief  struct for Ring Buffer Management
**/
typedef struct {
    unsigned short nIndexIn;                        // index to write in the ring buffer
    unsigned short nIndexOut;                       // index to read out of the ring buffer and write to AD5405
    unsigned short nElementsLoaded;                 // number of elements loaded to ring buffer
    unsigned short *pBufferBase;                    // address of the ring buffer
    unsigned short *pWriteBuffer;                   // address to write to AD5405
    unsigned short nMaxFillStatus;                  // strategy of filling the ring buffer, max condition
    unsigned short nMinFillStatus;                  // strategy of filling the ring buffer, min condition
    T_AD5405_BUFFER_EMPTY_MODE nBufferEmptyMode;    // what to do if the buffer is empty, write zero, write 1 (max, value), write last sample, autobuffer
    bool bAutobufferEnabled;                        // if autobuffer mode is enabled, no values can be written to the ringbuffer
}T_AD5405_BUFFER_SPEC;


/**     
*       @brief  struct that is handed over to the user callback funtion
**/
typedef struct {
    unsigned short nElementsLoaded;
    unsigned short nChannel;
    bool bBufferFull;                               // indicates if the buffer fill max. condition is reached
}T_AD5405_CALLBACK_ARG;


// prototype for user callback function
typedef void (*T_AD5405_CALLBACK)(T_AD5405_CALLBACK_ARG *);


/**     
*       @brief  struct for AD5405 management
**/
typedef struct {
    unsigned short nMaxElements;
    T_AD5405_CHANNEL_MODE nChannelMode;
    T_AD5405_CALLBACK fnCallback;
}T_AD5405_SPEC;


/**     
*       @brief  struct for AD5405 configuration
**/
typedef struct {
    unsigned long nSamplingRate;
    unsigned short nMaxElements;
    unsigned short nTimerNr;
    unsigned long nSystemClk;
    unsigned long nWriteBufferAddress;
    T_AD5405_CHANNEL_MODE nChannelMode;
    T_AD5405_BUFFER_EMPTY_MODE nBufferEmptyMode;
    T_AD5405_CALLBACK fnCallback;
    unsigned short nMaxFillStatus;
    unsigned short nMinFillStatus;
    void *__p_reserved;
}T_AD5405_CONFIG;

// prototype declarations
void AD5405WriteSample(void *pa_pCallbackArg);

T_ERROR_CODE AD5405Setup(T_AD5405_CONFIG *pa_aAD5405Config);

T_ERROR_CODE AD5405Write(unsigned short pa_nValue, unsigned short pa_nChannel);

void AD5405Start(void);

void AD5405Stop(void);

void AD5405Cleanup(void);

#endif
