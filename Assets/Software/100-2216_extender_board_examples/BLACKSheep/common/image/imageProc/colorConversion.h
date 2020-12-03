#ifndef _COLOR_CONVERSION_H_INCLUDED_
#define _COLOR_CONVERSION_H_INCLUDED_

typedef struct {
    unsigned long nSourceStride;
} T_YUV422_TO_RGB565_SPEC;

// C prototypes for assembler functions
void YUV422toRGB565(unsigned char *input, unsigned char *out, unsigned long N, void *pArg);
// other prototypes
void YUV422ToRGB565setup(unsigned short pa_nYUVpixelStride);


#endif

