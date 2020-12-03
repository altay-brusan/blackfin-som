#ifndef _EXT_BF533_AD_DA_INCLUDING_
#define _EXT_BF533_AD_DA_INCLUDING_

#include "../../../../../driver/src/common/ADCDAC/AD7266/AD7266.h"
#include "../../../../../driver/src/common/ADCDAC/AD5405/AD5405.h"
#include "../../../../../driver/src/common/ADCDAC/AD5415/AD5415.h"


// prototypes
void AD5415UserCallback(void *pa_pBufferAccessable);
void AD5405UserCallback(T_AD5405_CALLBACK_ARG *pa_pBufferFillStatus);
void ami_setup (void);

#endif

