/**
 *	@file 		SCCBConfig.c
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

#include "SCCBconfig.h"

#include <services/services.h>
#include <cycle_count_bf.h>

static unsigned long g_nCoreClk = 0;

#define SCCB_BASE_WAIT_CYCLE		2	//us






/**
 *	@private
 *	@brief		Timing Wait function, pauses program flow for ~1us
 *
 *	
 **/
void sccb_wait_us (void) {
	unsigned long long int cur,nd;
	
	_GET_CYCLE_COUNT(cur);
	nd = cur + SCCB_BASE_WAIT_CYCLE * (g_nCoreClk / 1000000);
	while (cur < nd) {
		_GET_CYCLE_COUNT(cur);
	}
}


/**
 *	@private
 *	@brief		Does a number of Waits depending on the provided number of wait states.
 *
 *				wait SCCB_BASE_WAIT_CYCLE us * pa_nWaitStates
 *
 *	@param		pa_nWaitStates	The number of times to make a 1 us wait.
 *	
 *	
 **/
void sccb_waitCycle(unsigned short pa_nWaitStates) {
	unsigned short i = 0;
	for (i=0; i<pa_nWaitStates; i++) {
		sccb_wait_us ();
	}
}


/**
 *	@public
 *	
 *	@brief		Writes a Byte of data to the Register of the specified Camera
 *				device.	
 *
 *	@param 		pa_hSccb 		Handle for the sccb
 *	@param 		pa_cDevAddr 	Camera device address
 *	@param 		pa_nSubAddr 	Camera register
 *	@param 		pa_cData		Data to write
 *
 *	@return 	ERR_NONE on sucess, or other ERR code.
 *	
 **/
T_ERROR_CODE sccb_writeByte(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, unsigned char pa_cData, unsigned long pa_nFlags) {
	
	T_ERROR_CODE erResult = ERR_NONE;
	
	if(pa_nFlags & SCCB_16BIT_REGADDR){
	    return sccb_writeByte16bit(pa_hSccb, pa_cDevAddr, pa_nSubAddr, pa_cData);
	} else{
	    return sccb_writeByte8bit(pa_hSccb, pa_cDevAddr, (unsigned char)pa_nSubAddr, pa_cData);
	}
}


/**
 *	@public
 *	
 *	@brief		Writes a Byte of data to the Register of the specified Camera
 *				device.	
 *
 *	@param 		pa_hSccb 		Handle for the sccb
 *	@param 		pa_cDevAddr 	Camera device address
 *	@param 		pa_cSubAddr 	Camera register
 *	@param 		pa_cData		Data to write
 *
 *	@return 	ERR_NONE on sucess, or other ERR code.
 *	
 **/
T_ERROR_CODE sccb_writeByte8bit(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned char pa_cSubAddr, unsigned char pa_cData) {
	
	T_ERROR_CODE erResult = ERR_NONE;
			
	T_SCCB_SPEC *pSpec = (T_SCCB_SPEC*)pa_hSccb;
	unsigned short i;
	unsigned short nReadByte;
	unsigned short nData;	
	T_GPIO_MASK sio_dMask = pSpec->sio_dPin;
	T_GPIO_MASK sio_cMask = pSpec->sio_cPin;
	
#ifdef _USE_VDK_
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
#endif	
	
	// Signalling "Start of transmission".
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_clear (sio_cMask);
	
	if ((pa_cDevAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} 
	
	else {
		gpio_set (sio_dMask);
	}
	
	pa_cDevAddr <<= 1;
	
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	
	//Writing the address
	for (i = 0; i <= 6; ++i) { 
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    //New data after 3/4 of tcycle.
		if ((pa_cDevAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} 
		
		else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cDevAddr <<= 1;
	}
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    	//New data after 3/4 of tcycle.
	gpio_becomeInput (sio_dMask);			//Setting the "Dont Care" bit, it means setting SIO_D to input.
	sccb_waitCycle(1);    	//New data after 3/4 of tcycle.
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    	//New data after 3/4 of tcycle.
	gpio_becomeOutput (sio_dMask);			//SIO_D as output.
	
	if ((pa_cSubAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	pa_cSubAddr <<= 1;

	//Writing the pa_cSubAddr.
	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((pa_cSubAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cSubAddr <<= 1;
	}
	
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    //New data after 3/4 of tcycle.
	gpio_becomeInput (sio_dMask);		  //Setting the "Dont Care" bit, it means setting SIO_D to input.
	sccb_waitCycle(1);    //New data after 3/4 of tcycle.
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1); 
	gpio_becomeOutput (sio_dMask);

	//Setting first bit of data value.
	if ((pa_cData & 0x80) == 0)
		gpio_clear (sio_dMask);
	else
		gpio_set (sio_dMask);
	pa_cData <<= 1;
	sccb_waitCycle(1);
	gpio_set (sio_cMask);

	//Writing the data value;
	for (i=0; i<=6; i++) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((pa_cData & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cData <<= 1;
	}

	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    //New data after 3/4 of tcycle.
	gpio_becomeInput (sio_dMask);		  //Setting the "Dont Care" bit, it means setting SIO_D to input.
	sccb_waitCycle(1);    //New data after 3/4 of tcycle.
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	
	// Signalling "End of Transmission".
	sccb_waitCycle(1);
	gpio_becomeOutput (sio_dMask);
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(1);
	gpio_set (sio_dMask);
	//SIO_D and SIO_C are "high": "Stop of transmission" signalled.

	sccb_waitCycle(20); //be sure, that SIO_D und SIO_C are "high" for at least 5 tcycle.
		
#ifdef _USE_VDK_
	adi_int_ExitCriticalRegion(pExitCriticalArg);
#endif
	
	return erResult;
}


/**
 *	@public
 *	
 *	@brief		Writes a Byte of data to the Register of the specified Camera
 *				device, Subaddress 16bit.	
 *
 *	@param 		pa_hSccb 		Handle for the sccb
 *	@param 		pa_cDevAddr 	Camera device address
 *	@param 		pa_nSubAddr 	Camera register
 *	@param 		pa_cData		Data to write
 *
 *	@return 	ERR_NONE on sucess, or other ERR code.
 *	
 **/
T_ERROR_CODE sccb_writeByte16bit(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, unsigned char pa_cData) {
	
	T_ERROR_CODE erResult = ERR_NONE;
			
	T_SCCB_SPEC *pSpec = (T_SCCB_SPEC*)pa_hSccb;
	unsigned short i;
	unsigned short nReadByte;
	unsigned short nData;	
	unsigned char cAddrBase = (pa_nSubAddr >> 8) & 0xff;
	unsigned char cSubAddr =  pa_nSubAddr & 0xff;
	// unsigned char cAddrBase = pa_nSubAddr & 0xff;
	// unsigned char cSubAddr =  (pa_nSubAddr >> 8) & 0xff;
	T_GPIO_MASK sio_dMask = pSpec->sio_dPin;
	T_GPIO_MASK sio_cMask = pSpec->sio_cPin;
	
#ifdef _USE_VDK_
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
#endif	
	
	//Signalling "Start of transmission"
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_clear (sio_cMask);
	
	if ((pa_cDevAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} 
	
	else {
		gpio_set (sio_dMask);
	}
	
	pa_cDevAddr <<= 1;
	
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	
	//Writing the address
	for (i = 0; i <= 6; ++i) { 
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    //New data after 3/4 of tcycle.
		if ((pa_cDevAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} 
		
		else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cDevAddr <<= 1;
	}
	
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);
	
	//Writing the Address Base.
	if ((cAddrBase & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	cAddrBase <<= 1;

	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((cAddrBase & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		cAddrBase <<= 1;
	}
	
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);
    
	//Writing the SubAddress.
	if ((cSubAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	cSubAddr <<= 1;

	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((cSubAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		cSubAddr <<= 1;
	}
	
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);

	//Setting first bit of data value.
	if ((pa_cData & 0x80) == 0)
		gpio_clear (sio_dMask);
	else
		gpio_set (sio_dMask);
	pa_cData <<= 1;
	sccb_waitCycle(1);
	gpio_set (sio_cMask);

	//Writing the data value;
	for (i=0; i<=6; i++) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((pa_cData & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cData <<= 1;
	}

	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    //New data after 3/4 of tcycle.
	gpio_becomeInput (sio_dMask);		  //Setting the "Dont Care" bit, it means setting SIO_D to input.
	sccb_waitCycle(1);    //New data after 3/4 of tcycle.
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	
	// Signalling "End of Transmission".
	sccb_waitCycle(1);
	gpio_becomeOutput (sio_dMask);
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(1);
	gpio_set (sio_dMask);
	//SIO_D and SIO_C are "high": "Stop of transmission" signalled.

	sccb_waitCycle(20); //be sure, that SIO_D und SIO_C are "high" for at least 5 tcycle.
		
#ifdef _USE_VDK_
	adi_int_ExitCriticalRegion(pExitCriticalArg);
#endif
	
	return erResult;
}


/**
 *	@public
 *	@brief		Reads a byte of data from the specified register of the specified Camera
 *
 *	@param 		pa_hSccb 		Handle for the sccb 
 *	@param 		pa_cDevAddr 	Camera device address
 *	@param 		pa_nSubAddr 	Camera register
 *	@param 		pa_errCode 		Pointer to error code
 *  @param      pa_nFlags       Flags for selecting the behaviour
 *
 *	@return 	The number of Bytes read
 * 	
 **/
unsigned short sccb_readByte(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, T_ERROR_CODE *pa_errCode, unsigned long pa_nFlags) {
	
	*pa_errCode = ERR_NONE;
	
	if(pa_nFlags & SCCB_16BIT_REGADDR){
	    return sccb_readByte16bit(pa_hSccb, pa_cDevAddr, pa_nSubAddr, pa_errCode);
	} else{
	    return sccb_readByte8bit(pa_hSccb, pa_cDevAddr, (unsigned char)pa_nSubAddr, pa_errCode);
	}
}


/**
 *	@public
 *	@brief		Reads a byte of data from the specified register of the specified Camera
 *
 *	@param 		pa_hSccb 		Handle for the sccb 
 *	@param 		pa_cDevAddr 	Camera device address
 *	@param 		pa_cSubAddr 	Camera register
 *	@param 		pa_errCode 		Pointer to error code
 *
 *	@return 	The number of Bytes read
 * 	
 **/
unsigned short sccb_readByte8bit(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned char pa_cSubAddr, T_ERROR_CODE *pa_errCode) {
	
	*pa_errCode = ERR_NONE;
	
	T_SCCB_SPEC *pSpec = (T_SCCB_SPEC*)pa_hSccb;
	
	unsigned short nReadByte;
	unsigned short i;
	unsigned short nData;
	T_GPIO_MASK sio_cMask = pSpec->sio_cPin;
	T_GPIO_MASK sio_dMask = pSpec->sio_dPin;
	unsigned char cReadAddr = pa_cDevAddr | 0x01;

#ifdef _USE_VDK_
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
#endif
	
	//Signalling "Start of transmission"
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_clear (sio_cMask);
		
	if ((pa_cDevAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	pa_cDevAddr <<= 1;
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	//Write the address
	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    //new data after 3/4 of tcycle
		if ((pa_cDevAddr & 0x80)  == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cDevAddr <<= 1;
	}
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);
	if ((pa_cSubAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	pa_cSubAddr <<= 1;
	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    // new data after 3/4 of tcycle
		if ((pa_cSubAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cSubAddr <<= 1;
	}
	
	//"Dont Care" Bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1); 
	gpio_becomeOutput (sio_dMask);
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(1);
	gpio_set (sio_dMask);
	//***"Stop of transmission" Clock to high, data to high
	
	sccb_waitCycle(5); //wait for phase 2 readcycle
	//Start phase 2 readcycle	
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_clear (sio_cMask);
	//address
	if ((cReadAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	cReadAddr <<= 1;
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    // new data after 3/4 of tcycle
		if ((cReadAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		cReadAddr <<= 1;
	}

	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	//relevantes Datum lesen
	nReadByte = 0;

	for (i = 0; i <= 7; ++i) {		
		gpio_clear (sio_cMask);
		sccb_waitCycle(2);
		gpio_set (sio_cMask);
		sccb_waitCycle(1);
		nData = gpio_readFlag (sio_dMask);
		if (nData != 0) {
			nReadByte |= 0x0001;
		}
		nReadByte <<= 1;
		sccb_waitCycle(1);
	}

	nReadByte >>= 1;
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);
	gpio_becomeOutput (sio_dMask);
	
	//NA bit to high, see sccb spec
	gpio_set (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(1);
	gpio_set (sio_dMask);
	//SIO_D and SIO_C are high now, "Stop of transmission" condition
	
	sccb_waitCycle(20); // SIO_D and SIO_C has to be high for 5 * tcycle
	
#ifdef _USE_VDK_
	adi_int_ExitCriticalRegion(pExitCriticalArg);
#endif
			
	return (nReadByte);
}


/**
 *	@public
 *	@brief		Reads a byte of data from the specified register of the specified Camera, Subaddress 16bit
 *
 *	@param 		pa_hSccb 		Handle for the sccb 
 *	@param 		pa_cDevAddr 	Camera device address
 *	@param 		pa_nSubAddr 	Camera register
 *	@param 		pa_errCode 		Pointer to error code
 *
 *	@return 	The number of Bytes read
 * 	
 **/
unsigned short sccb_readByte16bit(T_SCCB_HANDLE pa_hSccb, unsigned char pa_cDevAddr, unsigned short pa_nSubAddr, T_ERROR_CODE *pa_errCode) {
	
	*pa_errCode = ERR_NONE;
	
	T_SCCB_SPEC *pSpec = (T_SCCB_SPEC*)pa_hSccb;
	
	unsigned short nReadByte = 0;
	unsigned short i;
	unsigned short nData;
	unsigned char cAddrBase = (pa_nSubAddr >> 8) & 0xff;
	unsigned char cSubAddr =  pa_nSubAddr & 0xff;
	// unsigned char cAddrBase = pa_nSubAddr & 0xff;
	// unsigned char cSubAddr =  (pa_nSubAddr >> 8) & 0xff;
	T_GPIO_MASK sio_cMask = pSpec->sio_cPin;
	T_GPIO_MASK sio_dMask = pSpec->sio_dPin;
	unsigned char cReadAddr = pa_cDevAddr | 0x01;

#ifdef _USE_VDK_
	void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);
#endif
	
	//Signalling "Start of transmission"
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_clear (sio_cMask);
	
	if ((pa_cDevAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} 
	
	else {
		gpio_set (sio_dMask);
	}
	
	pa_cDevAddr <<= 1;
	
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	
	//Writing the address
	for (i = 0; i <= 6; ++i) { 
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    //New data after 3/4 of tcycle.
		if ((pa_cDevAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} 
		
		else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		pa_cDevAddr <<= 1;
	}
	
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);
	
	//Writing the Address Base.
	if ((cAddrBase & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	cAddrBase <<= 1;

	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((cAddrBase & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		cAddrBase <<= 1;
	}
	
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);
    
	//Writing the SubAddress.
	if ((cSubAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	cSubAddr <<= 1;

	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);  //New data after 3/4 of tcycle.
		if ((cSubAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		cSubAddr <<= 1;
	}
	
	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_becomeOutput (sio_dMask);
	
	gpio_set (sio_cMask);
	sccb_waitCycle(1);
	gpio_set (sio_dMask);
	//"Stop of transmission" Clock to high, data to high
	
	sccb_waitCycle(5); //wait for phase 2 readcycle
	
	//Start phase 2 readcycle	
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_clear (sio_cMask);
	
	//address
	if ((cReadAddr & 0x80) == 0) {
		gpio_clear (sio_dMask);
	} else {
		gpio_set (sio_dMask);
	}
	cReadAddr <<= 1;
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	for (i = 0; i <= 6; ++i) {
		sccb_waitCycle(2);
		gpio_clear (sio_cMask);
		sccb_waitCycle(1);    // new data after 3/4 of tcycle
		if ((cReadAddr & 0x80) == 0) {
			gpio_clear (sio_dMask);
		} else {
			gpio_set (sio_dMask);
		}
		sccb_waitCycle(1);
		gpio_set (sio_cMask);
		cReadAddr <<= 1;
	}

	//"Dont Care" bit
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);		// new data after 3/4 of tcycle
	gpio_becomeInput (sio_dMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);    // new data after 3/4 of tcycle
	
	//relevantes Datum lesen
	nReadByte = 0;

	for (i = 0; i <= 7; ++i) {		
		gpio_clear (sio_cMask);
		sccb_waitCycle(2);
		gpio_set (sio_cMask);
		sccb_waitCycle(1);
		nData = gpio_readFlag (sio_dMask);
		if (nData != 0) {
			nReadByte |= 0x0001;
		}
		nReadByte <<= 1;
		sccb_waitCycle(1);
	}

	nReadByte >>= 1;
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);
	gpio_becomeOutput (sio_dMask);
	
	//NA bit to high, see sccb spec
	gpio_set (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(2);
	gpio_clear (sio_cMask);
	sccb_waitCycle(1);
	gpio_clear (sio_dMask);
	sccb_waitCycle(1);
	gpio_set (sio_cMask);
	sccb_waitCycle(1);
	gpio_set (sio_dMask);
	//SIO_D and SIO_C are high now, "Stop of transmission" condition
	
	sccb_waitCycle(20); // SIO_D and SIO_C has to be high for 5 * tcycle
	
#ifdef _USE_VDK_
	adi_int_ExitCriticalRegion(pExitCriticalArg);
#endif
			
	return (nReadByte);
}


/** 
 *	@public
 *	@brief		if the open operation succeds pa_hSccb holds a pointer to the initialized sccb handle, ready for use.
 *
 *
 *	@param 		pa_phSccb 		Pointer to the sccb handle
 *	@param 		pa_sio_cMask 	Gpio bitmask to specifiy the connected clock pin
 *	@param 		pa_sio_dMask 	Gpio bitmask to specifiy the connected data pin
 *	@param 		pa_pwdnMask 	Gpio bitmask to specifiy the connected power down pin
 *	@param 		pa_cWriteAddr 	Destination address for the sccb
 *	@param 		pa_nCoreClk 	Core clock frequency in hertz @see getCoreClockFrequency()
 *
 *	@return 	ERR_NONE on sucess, otherwise an error code
 *		
 **/
T_ERROR_CODE sccb_open(T_SCCB_HANDLE *pa_phSccb, T_GPIO_MASK pa_sio_cMask, T_GPIO_MASK pa_sio_dMask, T_GPIO_MASK pa_pwdnMask, unsigned char pa_cWriteAddr, unsigned long pa_nCoreClk) {
	
	T_ERROR_CODE erResult = ERR_NONE;
	// allocate some memory for the sccb struct
	T_SCCB_SPEC *pSpec = (T_SCCB_SPEC*)malloc(sizeof(T_SCCB_SPEC));
	*pa_phSccb = (T_SCCB_HANDLE)pSpec;
	
	pSpec->sio_cPin = pa_sio_cMask;
	pSpec->sio_dPin = pa_sio_dMask;
	pSpec->pwdnPin = pa_pwdnMask;
	pSpec->nCoreClk = pa_nCoreClk;
	g_nCoreClk = pa_nCoreClk;
	
	//SIO_C, SIO_D and PWDN as output.
	gpio_becomeOutput (pa_sio_cMask);
	gpio_becomeOutput (pa_sio_dMask);
	if (pa_pwdnMask) {
		// only if it is not zero use it
		gpio_becomeOutput (pa_pwdnMask);
	}
	gpio_set (pa_sio_cMask);
	gpio_set (pa_sio_dMask);
	gpio_clear (pa_pwdnMask);

	sccb_waitCycle(8); 			//SIO_D and SIO_C has to be high for 2 * tcycle
		
	return erResult;
}


/**
 *	@public		
 *	@brief		Closes the sccb and frees allocted resources
 *
 *	@param 		pa_hSccb handle for the sccb
 *
 *	@return 	on success #ERR_NONE, otherwise an error code
 *	
 **/
T_ERROR_CODE sccb_close(T_SCCB_HANDLE pa_hSccb) {	
	T_ERROR_CODE erResult = ERR_NONE;
	T_SCCB_SPEC *pSpec = (T_SCCB_SPEC*)pa_hSccb;
		
	gpio_becomeInput (pSpec->sio_cPin);
	gpio_becomeInput (pSpec->sio_dPin);
	gpio_becomeInput (pSpec->pwdnPin);
	// free the memory allocated for the handle structure
	free(pSpec);

	return erResult;
}
