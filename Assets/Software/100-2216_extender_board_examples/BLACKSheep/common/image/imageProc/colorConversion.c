#include "colorConversion.h"

#ifdef __ADSPBF561__
    #ifdef __COREA__
        #pragma align 4
        section ("L1_data_a") T_YUV422_TO_RGB565_SPEC g_tYUV422toRGB565Spec;
    #elif defined (__COREB__)
        #pragma align 4
        section ("L1_data_b") T_YUV422_TO_RGB565_SPEC g_tYUV422toRGB565Spec;
    #else
        #error "you must specify a core"
    #endif
#else
    #pragma align 4
    section ("L1_data") T_YUV422_TO_RGB565_SPEC g_tYUV422toRGB565Spec;
#endif

void YUV422ToRGB565setup(unsigned short pa_nYUVpixelStride) {
	g_tYUV422toRGB565Spec.nSourceStride = pa_nYUVpixelStride;
}
