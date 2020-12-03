#include "Talkthrough.h"

extern bool bToggleInput;		// declared in main.c
//--------------------------------------------------------------------------//
// Function:	Process_Data()												//
//																			//
// Description: This function is called from inside the SPORT0 ISR every 	//
//				time a complete audio frame has been received. The new 		//
//				input samples can be found in the variables iChannel0LeftIn,//
//				iChannel0RightIn, iChannel1LeftIn and iChannel1RightIn 		//
//				respectively. The processed	data should be stored in 		//
//				iChannel0LeftOut, iChannel0RightOut, iChannel1LeftOut,		//
//				iChannel1RightOut, iChannel2LeftOut and	iChannel2RightOut	//
//				respectively.												//
//--------------------------------------------------------------------------//
void Process_Data(void)
{

	if (!bToggleInput) {  
		iChannel0LeftOut  = iChannel0LeftIn;
		iChannel0RightOut = iChannel0RightIn;
		iChannel1LeftOut  = iChannel0LeftIn;
		iChannel1RightOut = iChannel0RightIn;
		iChannel2LeftOut  = iChannel0LeftIn;
		iChannel2RightOut = iChannel0RightIn;
	} else {
		iChannel0LeftOut  = iChannel1LeftIn;
		iChannel0RightOut = iChannel1RightIn;
		iChannel1LeftOut  = iChannel1LeftIn;
		iChannel1RightOut = iChannel1RightIn;
		iChannel2LeftOut  = iChannel1LeftIn;
		iChannel2RightOut = iChannel1RightIn;
	}
}
