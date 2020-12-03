#ifndef _EXT_BF537E_AD_DA_INCLUDING_
#define _EXT_BF537E_AD_DA_INCLUDING_

#include "../../../../../driver/src/common/ADCDAC/AD7266/AD7266.h"
#include "../../../../../driver/src/common/ADCDAC/AD5415/AD5415.h"
#include "../../../../../driver/src/common/ADCDAC/AD5405/AD5405.h"

typedef enum {
    PROCESS_GAIN,
    PROCESS_CHANNEL_ADDITION,
    PROCESS_CHANNEL_SUBTRACTION_1_2,
    PROCESS_CHANNEL_SUBTRACTION_2_1
}T_PROCESS_MODE;


// prototypes
void AD5415UserCallback(void *pa_pBufferAccessable);

void AD7266UserCallback(void *pa_pBufferAccessable);

void AD5405UserCallback(T_AD5405_CALLBACK_ARG *pa_pBufferFillStatus);

T_ERROR_CODE ProcessData(   float pa_fGain,
                            unsigned short *pa_pBufferBase,
                            unsigned short pa_nBurstSize,
                            unsigned short pa_nStride,
                            unsigned short pa_nOffset,
                            T_AD5415_CHANNEL_MODE pa_nChannelMode,
                            T_PROCESS_MODE pa_nProcessMode);
                            
void ami_setup (void);

#endif

