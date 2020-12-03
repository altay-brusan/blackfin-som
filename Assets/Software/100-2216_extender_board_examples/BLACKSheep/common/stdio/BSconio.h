#ifndef _BS_CONIO_INCLUDED_
#define _BS_CONIO_INCLUDED_

//prototype declarations
int bs_kbhit(void);

#ifdef _USE_VDK_

//this prototypes are only available in a vdk environment
int bs_vdk_kbhit(void);

#endif

#endif
