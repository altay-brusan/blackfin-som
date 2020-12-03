#ifdef _USE_VDK_

#include <VDK.h>
#include <cycle_count_bf.h>

#else

#include <cycle_count_bf.h>
#include "../../../driver/src/common/pwrmngt/clockquery.h"

#endif

#include "sleep.h"

void bs_sleep (unsigned long pa_nMs) {
	
#ifdef _USE_VDK_
	VDK_Sleep((VDK_Ticks)((float)pa_nMs / VDK_GetTickPeriod ()));
#else

	unsigned long long int cur,nd;
	
	_GET_CYCLE_COUNT(cur);
	nd = cur + (getCoreClockFrequency() / 1000) * pa_nMs;
	while (cur < nd) {
		_GET_CYCLE_COUNT(cur);
	}
	
#endif

}


void bs_sleep_us (unsigned long pa_nUs) {
	
	unsigned long long int cur,nd;
	
	_GET_CYCLE_COUNT(cur);
	nd = cur + (getCoreClockFrequency() / 1000000) * pa_nUs;
	while (cur < nd) {
	    #ifdef _USE_VDK_
	        VDK_Yield();
	    #endif
		_GET_CYCLE_COUNT(cur);
	}
}
