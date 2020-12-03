/**
 *	@file 		SCCBConfig.h
 *	@ingroup 	SCCB
 *	
 *	@brief 		Interface to communicate over the SCCB (Serial Camera Control Bus)
 *	
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Daniel Weber, Roland Oberhammer
 *	
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 **/
 
/** @defgroup SCCB
 *  @ingroup driverapi
 *  Serial Camera Control Bus
 */

#ifndef _SCCB_CONFIG_H_INCLUDED_
#define _SCCB_CONFIG_H_INCLUDED_

#include <Datatypes.h>
#include "GPIOconfig.h"

// flags
#define SCCB_16BIT_REGADDR      (1 << 0)

/**
	@name Serial Camera Control Bus (SCCB)
*/
//@{

typedef struct {
		T_GPIO_MASK sio_cPin;			///<PF-flag used for SIO_C.
		T_GPIO_MASK sio_dPin;			///<PF-flag used for SIO_D.
		T_GPIO_MASK pwdnPin;			///<PF-flag used for PWDN.
		unsigned long nCoreClk;
	} T_SCCB_SPEC;

typedef long T_SCCB_HANDLE;				///< handle for the sccb
		
//Prototype declarations.

void 			sccb_waitCycle	(unsigned short pa_nWaitStates);
T_ERROR_CODE 	sccb_open		(T_SCCB_HANDLE *pa_phSccb, T_GPIO_MASK pa_sio_cMask, T_GPIO_MASK pa_sio_dMask, T_GPIO_MASK pa_pwdnMask, unsigned char pa_cWriteAddr, unsigned long pa_nCoreClk);
T_ERROR_CODE 	sccb_writeByte	(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, unsigned char pa_cData, unsigned long pa_nFlags);
T_ERROR_CODE 	sccb_writeByte8bit	(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned char pa_cSubAddr, unsigned char pa_cData);
T_ERROR_CODE 	sccb_writeByte16bit	(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, unsigned char pa_cData);
unsigned short 	sccb_readByte	(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, T_ERROR_CODE *pa_errCode, unsigned long pa_nFlags);
unsigned short 	sccb_readByte8bit	(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned char pa_cSubAddr, T_ERROR_CODE *pa_errCode);
unsigned short 	sccb_readByte16bit	(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, T_ERROR_CODE *pa_errCode);
T_ERROR_CODE 	sccb_close		(T_SCCB_HANDLE pa_hSccb);

//@}

//@}

#endif

