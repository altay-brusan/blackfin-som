/**
 *	@file 		GPIOconfig.c
 *	@ingroup 	GPIO
 *	
 *	@brief 		Hardware abstracted GPIO Driver to control all GPIO operations.
 *	
 *						This driver can also incorporate GPIO expander drivers into a simple
 *						to use unified generic GPIO driver model.
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
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
 
/** @defgroup GPIO
 *  @ingroup driverapi
 *  GPIO Module, handles internal GPIO and GPIO Expanders.
 */
 
 
#include "GPIOconfig.h"

#define _USE_GPIO_EXPANDER_	// Use of this define, provides for optimisation of code density, if we know that no
														// expander will be used. (When compiled, we'll require less ASM if we don't use it).

//extern T_GPIO_SPEC g_stGPIOspec;

extern 	T_GPIO_BANKS 	g_aGPIObanks[];					///< Hardware Specific Bank Array, defined in gpio_global.c @see gpio_global.c
#ifdef __ADSPBF548__	    
    extern 	T_GPIO_PINT     g_aGPIOpints[]; 			    ///< Hardware Specific Bank Array, defined in gpio_global.c @see gpio_global.c
#endif
extern 	unsigned int 	g_nGPIObankCount;				///< The number of internal GPIO banks.
		unsigned int 	g_nGPIOexpanderBankCount = 0;	///< The number of GPIO banks on GPIO expanders.

#ifdef _USE_GPIO_EXPANDER_
T_GPIO_EXP_SPEC 			g_gpioExp[MAX_GPIO_EXPANDER];
#endif




#ifdef _USE_GPIO_EXPANDER_


/**
 *	@private
 *	@brief		Initialises an array of T_GPIO_EXP_SPEC structures for the maximum number of GPIO expanders.
 *	
 **/
void gpio_InitIOExpander (void) {
	unsigned short i;
	for (i=0; i<MAX_GPIO_EXPANDER; i++) {
		g_gpioExp[i].bInUse = false;
		g_gpioExp[i].extgpio_set = 0;
		g_gpioExp[i].extgpio_clear = 0;
		g_gpioExp[i].extgpio_toggle = 0;
		g_gpioExp[i].extgpio_becomeInput = 0;
		g_gpioExp[i].extgpio_becomeOutput = 0;
	}
}


/**
 *	@public
 *	@brief		Initialises an I/O Expander so that it can be used with the generic GPIO driver.
 *	
 *				This function assigns a driver a GPIO bank, the function initilises a structure that describes
 *				the driver through a list of pointers to the equivalent generic gpio functions.
 *	
 *				The driver can be assigned to any available GPIO bank number, within the range of
 *				MAX_GPIO_EXPANDER + g_nGPIObankCount.
 *	
 *	@param 		pa_nBank 				Bank assigned to gpio driver (must be one above the last internal bank, almost "g_nGPIObankCount")
 *	@param		extgpio_becomeInput		Pointer to become input function of io driver
 *	@param		extgpio_becomeOutput	Pointer to become output function of io driver
 *	@param		extgpio_set				Pointer to function for setting gpio high
 *	@param		extgpio_clear			Pointer to function for setting gpio low
 *	@param		extgpio_toggle			Pointer to function for toggling gpio
 *	@param		extgpio_readFlag		Pointer to function for reading flag value
 *	@param		pa_pGeneric				Reserved for future use (Generic pointer)
 *	
 *	@return		Returns ERR_NONE on success, and ERR_GENERIC on failure.
 *	
 **/
T_ERROR_CODE gpio_AddIOExpander (	
									unsigned short pa_nBank,
									T_GPIO_EXP_FUNC extgpio_becomeInput,	// Pointers to GPIO Expander functions
									T_GPIO_EXP_FUNC extgpio_becomeOutput,
									T_GPIO_EXP_FUNC extgpio_set,
									T_GPIO_EXP_FUNC extgpio_clear,
									T_GPIO_EXP_FUNC extgpio_toggle,
									unsigned short (*extgpio_readFlag)(T_GPIO_MASK), // Pointer to a GPIO read flag function, the function take an argument of type (T_GPIO_MASK)
									void *pa_pGeneric) {	// Generic pointer reserved for future use.
	T_ERROR_CODE erResult = ERR_NONE;
	
	/* Search for a free location in the g_gpioExp array. (Is there a free slot?) */
	
	unsigned short i = 0;
	
	while ((i < MAX_GPIO_EXPANDER) && (g_gpioExp[i].bInUse)) {
		i++;
	}
	
/*
 * 	If the found slot doesn't exceed the number of possible additional GPIO banks, and the 
 *	provided pa_nBank is situated above the internal bank range, we can setup the GPIO driver.
 */
 
	if ((i != MAX_GPIO_EXPANDER) && (pa_nBank >= g_nGPIObankCount)) {
		g_gpioExp[i].bInUse = true;
		g_gpioExp[i].nBank = pa_nBank;
		g_gpioExp[i].extgpio_becomeInput = extgpio_becomeInput;
		g_gpioExp[i].extgpio_becomeOutput = extgpio_becomeOutput;
		g_gpioExp[i].extgpio_set = extgpio_set;
		g_gpioExp[i].extgpio_clear = extgpio_clear;
		g_gpioExp[i].extgpio_toggle = extgpio_toggle;
		g_gpioExp[i].extgpio_readFlag = extgpio_readFlag;
		
		g_nGPIOexpanderBankCount ++;		// a new bank was installed so update the bank count variable
	} 
	
	else {
		erResult = ERR_GENERIC;					// No slot's available, produce an appropriate error!
	}
	
	return erResult;
}



/**
 *	@private
 *	@brief		Returns a 16-bit array index, pointing to the specified bank's (pa_nBank) location in the Expander Data Array
 *	
 *						This function is used internally to map GPIO Expanders into generic GPIO banks.
 *	
 *	@param 		pa_nBank	Bank identifier assigned to the gpio expander driver (must be one above the last internal bank, almost "g_nGPIObankCount")
 *
 *	@return		Returns a 16-bit, array index, for locating the GPIO expander's entry in the g_gpioExp[] array.
 *	
 **/
unsigned short gpio_getExpEntry (unsigned short pa_nBank) {
	
	unsigned short i = 0;
	
 	while ((i<MAX_GPIO_EXPANDER) && (g_gpioExp[i].nBank != pa_nBank)) {
		i++;
	}
	
	return i;
}




#endif

/**
 *	@public
 *	@brief		Sets GPIO flags, as specified in the mask T_GPIO_MASK pa_nFlag
 *	
 *						GPIO flags are addressed via banks, and the sets are provided by the bit-mask, 
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask to set the bits.
 *
 *
 *  @see			Definition of T_GPIO_MASK
 *	
 **/
void gpio_set(T_GPIO_MASK pa_nFlag) {
#ifdef _USE_GPIO_EXPANDER_
	
	unsigned short nBank = (pa_nFlag & 0xffff0000) >> 16;
	
	if (nBank < g_nGPIObankCount) {		// pa_nFlag addresses an internal bank
#ifdef __ADSPBF548__	    
		*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortSet) = (unsigned short)pa_nFlag; // Using a cast to mask off the Bank address
#else
		*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOflagS) = (unsigned short)pa_nFlag; // Using a cast to mask off the Bank address
#endif		
	}
	
	else { 														// pa_nFlag addresses a bank on a GPIO expander
		
		unsigned short nEntry = gpio_getExpEntry(nBank);	// Find the specified bank, in the array of GPIO Expander drivers.
		
		if (g_gpioExp[nEntry].extgpio_set) { 		// Make sure we don't have a NULL pointer!
			g_gpioExp[nEntry].extgpio_set(pa_nFlag); // Execute the Driver's set function.
		}
	}
	
#else	
#ifdef __ADSPBF548__	    
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortSet) = (unsigned short)pa_nFlag;
#else
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOflagS) = (unsigned short)pa_nFlag;
#endif
#endif	

}


/**
 *	@public
 *	@brief		Clears GPIO flags, as specified in the mask T_GPIO_MASK pa_nFlag
 *	
 *				GPIO flags are addressed via banks, and the clears are provided by the bit-mask, 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask to clear the bits.
 *
 *
 *  @see		Definition of T_GPIO_MASK	
 *		
 **/
void gpio_clear(T_GPIO_MASK pa_nFlag) {
#ifdef _USE_GPIO_EXPANDER_
	
	unsigned short nBank = (pa_nFlag & 0xffff0000) >> 16;
	
	if (nBank < g_nGPIObankCount) {		// pa_nFlag addresses an internal bank
#ifdef __ADSPBF548__	    
		*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortClear) = (unsigned short)pa_nFlag; // Using a cast to mask off the Bank address
#else
		*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOflagC) = (unsigned short)pa_nFlag; // Using a cast to mask off the Bank address
#endif		
	}
	
	else { 														// pa_nFlag addresses a bank on a GPIO expander
		
		unsigned short nEntry = gpio_getExpEntry(nBank);	// Find the specified bank, in the array of GPIO Expander drivers.
		
		if (g_gpioExp[nEntry].extgpio_clear) { 		// Make sure we don't have a NULL pointer!
			g_gpioExp[nEntry].extgpio_clear(pa_nFlag); // Execute the Driver's clear function.
		}
	}
	
#else	
#ifdef __ADSPBF548__	    
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortClear) = (unsigned short)pa_nFlag;
#else
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOflagC) = (unsigned short)pa_nFlag;
#endif
#endif	

}



/**
 *	@public
 *	@brief		Sets the PINT Mask register of the specified flag
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier.
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_setPIntMask(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntMaskSet) = pa_nFlag;
}
#endif


/**
 *	@public
 *	@brief		Clears the PINT Mask register of the specified flag
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier.
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_clearPIntMask(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntMaskClear) = pa_nFlag;
}
#endif



/**
 *	@public
 *	@brief		Clears the PINT Request register of the specified flag
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier.
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_clearPIntRequest(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntRequest) = pa_nFlag;
}
#endif



/**
 *	@public
 *	@brief		Clears the PINT Latch register of the specified flag
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier.
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_clearPIntLatch(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntLatch) = pa_nFlag;
}
#endif



/**
 *	@public
 *	@brief		Reads the PINT Latch register of the specified flag
 *	
 *	
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
unsigned int gpio_readPIntLatch(unsigned char pa_nPintNumber) {
    return (*(g_aGPIOpints[pa_nPintNumber].pPIntLatch));
}
#endif



/**
 *	@public
 *	@brief		Sets the PINT Edge register of the specified flag
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier.
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_setPIntEdge(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntEdgeSet) = pa_nFlag;
    ssync();
}
#endif




/**
 *	@public
 *	@brief		Clears the PINT Edge register of the specified flag
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier.
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_clearPIntEdge(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntEdgeClear) = pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Reads the PINT Pin State register of the specified flag
 *	
 *	
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
unsigned int gpio_readPIntPinState(unsigned char pa_nPintNumber) {
    return (*(g_aGPIOpints[pa_nPintNumber].pPIntPinState));
}
#endif



/**
 *	@public
 *	@brief		Sets the PINT Invert Set register of the specified GPIO Flags 
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_setPIntInvert(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntInvertSet) = pa_nFlag;
}
#endif



/**
 *	@public
 *	@brief		Clears the PINT Invert Set register of the specified GPIO Flags 
 *	
 *	
 *	@param 		pa_nFlag	        Flag identifier
 *  @param      pa_nPintNumber      Number of the PINT.
 *
 *  @attention  Only implemented for BF548
 *
 *		
 **/
#ifdef __ADSPBF548__
void gpio_clearPIntInvert(unsigned int pa_nFlag, unsigned char pa_nPintNumber) {
    *(g_aGPIOpints[pa_nPintNumber].pPIntInvertClear) = pa_nFlag;
}
#endif





/**
 *	@public
 *	@brief		Reads the Polarity of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the read masking is provided by the bit-mask, 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for reading the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
unsigned short gpio_readPolar(T_GPIO_MASK pa_nFlag) {
	return *(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOpolar) & (unsigned short)pa_nFlag;
}
#endif



/**
 *	@public
 *	@brief		Reads the EDGE sensitivity register of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the read masking is provided by the bit-mask, 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for reading the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
unsigned short gpio_readEdge(T_GPIO_MASK pa_nFlag) {
	return *(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOedge) & (unsigned short)pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Sets the Polarity of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the bits to be set are provided by the bit-mask, 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for setting the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_setPolar(T_GPIO_MASK pa_nFlag) {
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOpolar) |= (unsigned short)pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Clears the Polarity of the specified GPIO Flags
 *	
 *						GPIO flags are addressed via banks, and the bits to be cleared is provided by the bit-mask, 
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for clearing the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
 #ifndef __ADSPBF548__
void gpio_clearPolar(T_GPIO_MASK pa_nFlag) {
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOpolar) &= (unsigned short)~pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Sets the EDGE sensitivity of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the bits to be set are provided by the bit-mask,
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for setting the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_setEdge(T_GPIO_MASK pa_nFlag) {
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOedge) |= (unsigned short)pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Clears the EDGE sensitivity of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the bits to be cleared are provided by the bit-mask,
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for clearing the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_clearEdge(T_GPIO_MASK pa_nFlag) {   
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOedge) &= (unsigned short)~pa_nFlag;
}
#endif


/**
 *	@public
 *	@brief		Sets the Alternative interrupt of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the bits to be set are provided by the bit-mask,
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for setting the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_setMaska(T_GPIO_MASK pa_nFlag) {   
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOmaskAS) |= (unsigned short)pa_nFlag;
}
#endif



/**
 *	@public
 *	@brief		Clears the Alternative interrupt of the specified GPIO Flags
 *	
 *				GPIO flags are addressed via banks, and the bits to be cleared are provided by the bit-mask,
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for clearing the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__ 
void gpio_clearMaska(T_GPIO_MASK pa_nFlag) {    
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOmaskAC) |= (unsigned short)~pa_nFlag;
}
#endif


/**
 *	@public
 *	@brief		Sets the Alternative interrupt of the specified GPIO Flags
 *	
 *						GPIO flags are addressed via banks, and the bits to be set are provided by the bit-mask,
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for setting the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__ 
void gpio_setMaskb(T_GPIO_MASK pa_nFlag) {
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOmaskBS) |= (unsigned short)pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Clears the Alternative interrupt of the specified GPIO Flags
 *	
 *						GPIO flags are addressed via banks, and the bits to be cleared are provided by the bit-mask,
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for clearing the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_clearMaskb(T_GPIO_MASK pa_nFlag) {
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOmaskBC) |= (unsigned short)~pa_nFlag;
}
#endif




/**
 *	@public
 *	@brief		Disables the specified GPIO Flags as Inputs
 *	
 *				GPIO flags are addressed via banks, and the bits to be disabled are provided by the bit-mask,
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for disabling the bits.
 *
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
void gpio_disableInen(T_GPIO_MASK pa_nFlag) {
#ifdef __ADSPBF548__     
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortInen) &= (unsigned short)~pa_nFlag;
#else
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOinen) &= (unsigned short)~pa_nFlag;
#endif	
}




/**
 *	@public
 *	@brief		Enables the specified GPIO Flags as Inputs
 *	
 *						GPIO flags are addressed via banks, and the bits to be cleared are provided by the bit-mask, , 
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for enabling the bits.
 *
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
void gpio_enableInen(T_GPIO_MASK pa_nFlag) {
#ifdef __ADSPBF548__     
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortInen) |= (unsigned short)pa_nFlag;
#else    
	*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOinen) |= (unsigned short)pa_nFlag;
#endif
}




/**
 *	@public
 *	@brief		Configures the specified GPIO Flags as INPUT(s)
 *	
 *				GPIO flags are addressed via banks, and the bits to be configured are provided by the bit-mask, , 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for configuring the bits.
 *
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
void gpio_becomeInput(T_GPIO_MASK pa_nFlag) {
#ifdef _USE_GPIO_EXPANDER_

	//unsigned short nBank = (pa_nFlag & 0xffff0000) >> 16; // gpioBank() macro does this operation for us!
	unsigned short nBank = gpioBank(pa_nFlag);
	
	if (nBank < g_nGPIObankCount) { // pa_nFlag addresses an internal bank's GPIO flag(s)
	
#ifdef __ADSPBF548__
        *(g_aGPIObanks[ nBank ].pPortDirClear) = (unsigned short)pa_nFlag;
#else		
		*(g_aGPIObanks[ nBank ].pFIOdirection) &= (unsigned short)~pa_nFlag; 	// Set DIRECTION as 0 (IN)
#endif		
        gpio_enableInen(pa_nFlag);
	} 
	
	else {													// pa_nFlag addresses GPIO flag(s) on a GPIO Expander
	
		unsigned short nEntry = gpio_getExpEntry(nBank);
		
		if (g_gpioExp[nEntry].extgpio_becomeInput) { 	// Check we don't have a NULL pointer!
			g_gpioExp[nEntry].extgpio_becomeInput(pa_nFlag); // Execute the Driver's becomeInput function.
		}
	}
	
#else
#ifdef __ADSPBF548__
        *(g_aGPIObanks[ nBank ].pPortDirClear) = (unsigned short)pa_nFlag;
#else		
		*(g_aGPIObanks[ nBank ].pFIOdirection) &= (unsigned short)~pa_nFlag; 	// Set DIRECTION as 0 (IN)
#endif		
        gpio_enableInen(pa_nFlag);
#endif	
}




/**
 *	@public
 *	@brief		Configures the specified GPIO Flags as OUTPUT(s)
 *	
 *				GPIO flags are addressed via banks, and the bits to be configured are provided by the bit-mask, , 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for configuring the bits.
 *
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
void gpio_becomeOutput(T_GPIO_MASK pa_nFlag) {
#ifdef _USE_GPIO_EXPANDER_

	unsigned short nBank = (pa_nFlag & 0xffff0000) >> 16;

	if (nBank < g_nGPIObankCount) { // pa_nFlag addresses an internal bank's GPIO flag(s)
    #ifdef __ADSPBF548__
        *(g_aGPIObanks[ nBank ].pPortDirSet) = (unsigned short)pa_nFlag;
    #else
		*(g_aGPIObanks[ nBank ].pFIOdirection) |= (unsigned short)pa_nFlag;
    #endif			
        gpio_disableInen(pa_nFlag);					
    } 
		
	else {	// pa_nFlag addresses GPIO flag(s) on a GPIO Expander	
	    unsigned short nEntry = gpio_getExpEntry(nBank);
			
		if (g_gpioExp[nEntry].extgpio_becomeOutput)	{	// Check we don't have a NULL pointer!
			g_gpioExp[nEntry].extgpio_becomeOutput(pa_nFlag);		// Call the Driver's becomeOutput function.
		}
	}
		
#else
#ifdef __ADSPBF548__
    *(g_aGPIObanks[ nBank ].pPortDirSet) = (unsigned short)pa_nFlag;
#else
    *(g_aGPIObanks[ nBank ].pFIOdirection) |= (unsigned short)pa_nFlag;
#endif			
    gpio_disableInen(pa_nFlag);					
#endif
	
}




/**
 *	@public
 *	@brief		Toggles the specified GPIO Flag(s)
 *	
 *						GPIO flags are addressed via banks, and the bits to be toggled are provided by the bit-mask, , 
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for toggling the bits.
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
void gpio_toggle(T_GPIO_MASK pa_nFlag) {
  
#ifdef _USE_GPIO_EXPANDER_
	
	unsigned short nBank = (pa_nFlag & 0xffff0000) >> 16;
	
	if (nBank < g_nGPIObankCount) {		// pa_nFlag addresses an internal bank's GPIO flag(s)
	
		
    #ifdef __ADSPBF548__				// toggle register not supported on bf548, so emulate it
	    if((*(g_aGPIObanks[ nBank ].pPort)) & (unsigned short)pa_nFlag)  {// pin is already set
	        *(g_aGPIObanks[ nBank ].pPortClear) = (unsigned short)pa_nFlag;
	    }
	    else {
	        *(g_aGPIObanks[nBank].pPortSet) = (unsigned short)pa_nFlag;
	    }	    
    #else		
		*(g_aGPIObanks[ nBank ].pFIOflagT) = (unsigned short)pa_nFlag;
    #endif		
	} 
	
	else {		// pa_nFlag addresses GPIO flag(s) on a GPIO Expander
		unsigned short nEntry = gpio_getExpEntry(nBank);
		
		if (g_gpioExp[nEntry].extgpio_toggle) {	// Check we don't have a NULL pointer!
			g_gpioExp[nEntry].extgpio_toggle(pa_nFlag);	// Call the Driver's toggle function
		}
	}
	
#else
#ifdef __ADSPBF548__				// toggle register not supported on bf548, so emulate it
    if(*(g_aGPIObanks[ nBank ].pPort & (unsigned short)pa_nFlag)  {// pin is already set
	    *(g_aGPIObanks[ nBank ].pPortClear) = (unsigned short)pa_nFlag;
	}
	else {
	    *(g_aGPIObanks[ nBank ].pPortSet) = (unsigned short)pa_nFlag;
	}	    
#else		
    *(g_aGPIObanks[ nBank ].pFIOflagT) = (unsigned short)pa_nFlag;
#endif		
#endif
	
}





/**
 *	@public
 *	@brief		Reads the specified GPIO Flag
 *	
 *						GPIO flags are addressed via banks, and the bits to be toggled are provided by the bit-mask, , 
 *						both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for toggling the bits.
 *
 *	@return		The bit-mask if the flag is active, else 0.
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
unsigned short gpio_readFlag(T_GPIO_MASK pa_nFlag) {

#ifdef _USE_GPIO_EXPANDER_

	unsigned short nBank = gpioBank(pa_nFlag);
	
	if (nBank < g_nGPIObankCount) { 	// pa_nFlag addresses an internal bank's GPIO flag(s)
    #ifdef __ADSPBF548__
        return(*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPort) & (unsigned short)pa_nFlag);
    #else	
		return(*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOflagD) & (unsigned short)pa_nFlag);
    #endif
	} 
	
	else {	// pa_nFlag addresses GPIO flag(s) on a GPIO Expander
		unsigned short nEntry = gpio_getExpEntry(nBank);
		
		if(g_gpioExp[nEntry].extgpio_readFlag) {		// Check we don't have a NULL pointer!
			return g_gpioExp[nEntry].extgpio_readFlag (pa_nFlag);
		}
		
		else
			return 0xffff;
		// TODO How to handle an error, what to do if the Pointer didn't exist or the driver is broken?
	}
	
#else
#ifdef __ADSPBF548__
    return(*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPort) & (unsigned short)pa_nFlag);
#else	
    return(*(g_aGPIObanks[ gpioBank(pa_nFlag) ].pFIOflagD) & (unsigned short)pa_nFlag);
#endif
#endif	

}




/* TODO POSSIBLE SOLUTION to read multiple Flags */
/**
 *	@public
 *	@brief		Reads the specified GPIO Bank
 *	
 *						Reads all Flags of the specified port. (Internal banks, are read in a single operation)
 *						Banks on GPIO expanders are read bit by bit, (16 reads!).
 *						EXPERIMENTAL Tested Briefly via GPIO Expander Simulator in Software.		
 *
 *	@param 		pa_nBank	Bank identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits are ignored.
 *
 *	@return		(16-bits) of the GPIO's Bank Data register.
 *
 *  @see			Definition of T_GPIO_MASK
 *		
 **/
unsigned long gpio_readPort(T_GPIO_MASK pa_nBank) {
	
	int i;
	unsigned short bitMask 		= 0x0001;	// Initial Bit Mask for Expander Bit Reading
	unsigned short expResult 	= 0x0000;	// Overall View, of Expander Port Read
	
	if(pa_nBank < g_nGPIObankCount) {
    #ifdef __ADSPBF548__
        return(*(g_aGPIObanks[ pa_nBank ].pPort));
    #else	
		return (signed long) *(g_aGPIObanks[ pa_nBank ].pFIOflagD);
    #endif   	    
	}
	
	else {
		unsigned short nEntry = gpio_getExpEntry( pa_nBank );
		
		if(g_gpioExp[ nEntry ].extgpio_readFlag) {		// Check we don't have a NULL pointer!
		
			pa_nBank &= 0xFFFF0000;	// Ensure the Bit-Mask remains clean!
			
			for(i = 0; i < 16; i++) {
				expResult |= g_gpioExp[ nEntry ].extgpio_readFlag( pa_nBank | bitMask );
				bitMask = bitMask << 1;	// Shift the bit we read each time.
			}
			return expResult;
		}
		
		else {
			return 0;	// Prevents Compiler warning, 0 would be reuturned even if this line didn't exist!	
		}
		
	}
	
	
}



/**
 *	@public
 *	@brief		Clears the specified flag in the PORTxFER register
 *	
 *				GPIO flags are addressed via banks, and the read masking is provided by the bit-mask, 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for reading the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_disableFunction(T_GPIO_MASK pa_nFlag) {
  if(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortFER) {
	  *(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortFER) &= (unsigned short)~pa_nFlag;
	}
}
#endif



/**
 *	@public
 *	@brief		Sets the specified flag in the PORTxFER register
 *	
 *				GPIO flags are addressed via banks, and the read masking is provided by the bit-mask, 
 *				both provided in a single parameter.
 *	
 *	@param 		pa_nFlag	Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for reading the bits.
 *  @attention  Not implemented for BF548
 *
 *  @see		Definition of T_GPIO_MASK
 *		
 **/
#ifndef __ADSPBF548__
void gpio_enableFunction(T_GPIO_MASK pa_nFlag) {
  if(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortFER) {
      *(g_aGPIObanks[ gpioBank(pa_nFlag) ].pPortFER) |= (unsigned short)pa_nFlag;
  }
}
#endif


/**
 *	@private
 *	@brief		Interrupt Handler for handling GPIOs set as Interrupts!
 *	
 *				When a GPIO activates an interrupt, the ISR from ADI, will call this interrupt handler
 *				if an interrupt has been set-up meaning that this handler has been hooked.
 *	
 *	@param 		*pa_pClientArg	A pointer (from ADI's ISR) to a structure defined by T_GPIO_INT_PARAM.
 *
 *	@return		ADI_INT_HANDLER_RESULT Returns ADI_INT_RESULT_PROCESSED if we handled the interrupt, else ADI_INT_RESULT_NOT_PROCESSED.	
 *
 *  @see		gpio_setupInterrupt() and T_GPIO_INT_PARAM definition.
 *		
 **/
ADI_INT_HANDLER_RESULT GPIOintHandler(void *pa_pClientArg) {
  
	T_GPIO_INT_PARAM *pParam = (T_GPIO_INT_PARAM*)pa_pClientArg;	// Provide a pointer for the parameter data to the defined T_GPIO_INT_PARAM structure.
	
	if (adi_int_SICInterruptAsserted(pParam->stPeripheral) == ADI_INT_RESULT_ASSERTED) {	// Check if the GPIO peripheral asserted the interrupt
	    
	    if (*(pParam->pData) & pParam->nFlag) { // Check if our Flag asserted the interrupt.
	        #ifdef __ADSPBF548__
	            *(pParam->pFlagClear) = pParam->nFlag;	// clear interrupt
	            ssync();
	        #else
				*(pParam->pFlagClear) = (unsigned short)(pParam->nFlag);	// clear interrupt
			#endif	
				
					/*bool bData = (*(pParam->pData) & (unsigned short)(pParam->nFlag) == (unsigned short)(pParam->nFlag));
					if (bData == !pParam->bOnLow) {
					}	*/	
			
				if (pParam->fnCallback) { 	// If a call-back function was provided, handle it! (Also checking that it's not a NULL pointer!)
					pParam->fnCallback(pParam->nFlag, pParam->pClientArg);
				}
			
				return ADI_INT_RESULT_PROCESSED;	// Notify ADI's ISR that the interrupt was for us, so we processed it!
	    } 
	    
	    else {	// Notify ADI's ISR that the interrupt didn't belong to us, so we didn't process it!
				return ADI_INT_RESULT_NOT_PROCESSED;
	    }
	}
	
	return ADI_INT_RESULT_NOT_PROCESSED;	// Notify ADI's ISR that the interrupt didn't belong to us, so we didn't process it!
}



/**
 *	@public
 *	@brief		Sets up interrupts on the specified GPIO Flag.
 *	
 *				Allows a call-back function to be hooked to the interrupt for processing.
 *	
 *	@param 		pa_nFlag  Flag identifier, 32-bits, the upper 16-bits is the bank identifier, and the lower 16-bits provide the bit-mask for toggling the bits.
 *	@param 		pa_bOnLevel  true->interrupt is level sensitive
 *                           false->interrupt is edge sensitive
 *	@param 		pa_bOnLow  (pa_bOnLevel=false) true->interrupt on falling edge
 *                                             false->interrupt on rising edge
 *                         (pa_bOnLevel=true) true->interrupt on active low
 *                                            false->interrupt on avtive high
 *	@param 		pa_nChannel Interrupt channel
 *	@param 		pa_fnCallback  Callback function
 *
 *	@return		handle to interrupt resource
 *
 *  @warning    Only for BF548: if you want to define an interrupt on both edges so you have to 
 *              set up both interrupt channels with the same callback function. On one channel 
 *              you can define an interrupt on the rising edge and on the other channel an 
 *              interrupt on the falling edge. The parameter pa_bOnHigh is ignored.
 *  @warning    Only for BF548: if you want to define an input interrupt you have to define the pin
 *              as input before you call this function.
 *
 *  @see		T_GPIO_INT_PARAM definition.
 *		
 **/
void *gpio_setupInterrupt(T_GPIO_MASK pa_nFlag,
						  bool pa_bOnLevel, 
						  bool pa_bOnLow, 
						  bool pa_bOnHigh, 
						  unsigned long pa_nChannel, 
						  unsigned long pa_nIVG, 
						  bool pa_bNestedInt, 
						  T_GPIO_CALLBACK pa_fnCallback, 
						  void *pa_pClientArg){
													 
	unsigned short nBank = gpioBank(pa_nFlag);
	unsigned int nOffset = 0;
						       
	T_GPIO_INT_PARAM *pParam = (T_GPIO_INT_PARAM*)malloc( sizeof(T_GPIO_INT_PARAM) );	// create and init the interrupt parameter struct
	
	//pParam->stPeripheral = (pa_nChannel == PF_CHANNEL_A)?g_stGPIOspec.stChannelA:g_stGPIOspec.stChannelB;
	
	
#ifdef __ADSPBF548__
    unsigned int nErrorResult;
    unsigned char nIndexPint = (pa_nChannel == PINT_PERIPHERAL_ID_A) ? g_aGPIObanks[nBank].nPeripheralIdA : 
                                                                       g_aGPIObanks[nBank].nPeripheralIdB;
    pParam->stPeripheral = g_aGPIOpints[nIndexPint].tPeripheralId;
	
    unsigned long nIVG;
	if (pa_nIVG) {	// user requires a fixed ivg
	    nIVG = pa_nIVG;
		nErrorResult = adi_int_SICSetIVG(pParam->stPeripheral, nIVG);
	} 
	else {					// no fixed ivg required, use the default one
		nErrorResult = adi_int_SICGetIVG(pParam->stPeripheral, &nIVG);
	}
    if(nErrorResult != ADI_INT_RESULT_SUCCESS ) {
	    return(NULL);   
	}  

    if(nBank==0 || nBank==1) {  // if PortA or PortB
        // PB.H->Byte3  PB.L->Byte2  PA.H->Byte1  PA.L->Byte0
        *(g_aGPIOpints[nIndexPint].pPIntAssign) = 0x01010000;
        
        nOffset = nBank * 16;
    }
    else {  // PortC - PortJ      
        if((pa_nFlag & 0xFFFF) <= 0xFF) {  // if a low pin will be assign           
            // try Byte0
            if(((*(g_aGPIOpints[nIndexPint].pPIntMaskSet)) & 0x000000FF) != 0) {  // interrupt always assigned
                if(((*(g_aGPIOpints[nIndexPint].pPIntAssign)) & 0x00000007) != (nBank-2)) { // interrupt is assigned to another port
                    // try Byte2
                    if(((*(g_aGPIOpints[nIndexPint].pPIntMaskSet)) & 0x00FF0000) != 0) {  // interrupt always assigned
                        if(((*(g_aGPIOpints[nIndexPint].pPIntAssign)) & 0x00070000) != (nBank-2)) { // interrupt is assigned to another port
                            return(NULL);                        
                        } else { // the correct port is always assigned
                            nOffset = 16;
                        }
                    } else { // no interrupt assigned -> catch this one!
                        *(g_aGPIOpints[nIndexPint].pPIntAssign) &= 0xFF00FFFF;
                        *(g_aGPIOpints[nIndexPint].pPIntAssign) |= (nBank-2) << 16;
                        nOffset = 16;
                    }
                } else {
                    nOffset = 0;                    
                }
            } else {
                *(g_aGPIOpints[nIndexPint].pPIntAssign) &= 0xFFFFFF00;
                *(g_aGPIOpints[nIndexPint].pPIntAssign) |= nBank-2;
                nOffset = 0;              
            }
        }
        else { // if a high pin will be assign
            // try Byte1
            if(((*(g_aGPIOpints[nIndexPint].pPIntMaskSet)) & 0x0000FF00) != 0) {  // interrupt always assigned
                if(((*(g_aGPIOpints[nIndexPint].pPIntAssign)) & 0x00000700 ) != (nBank-2)) { // interrupt is assigned to another port
                    // try Byte3
                    if(((*(g_aGPIOpints[nIndexPint].pPIntMaskSet)) & 0xFF000000) != 0) {  // interrupt always assigned
                        if(((*(g_aGPIOpints[nIndexPint].pPIntAssign)) & 0x07000000) != (nBank-2)) { // interrupt is assigned to another port
                            return(NULL);                        
                        } else { // the correct port is always assigned
                            nOffset = 24;
                        }
                    } else { // no interrupt assigned -> catch this one!
                        *(g_aGPIOpints[nIndexPint].pPIntAssign) &= 0x00FFFFFF;
                        *(g_aGPIOpints[nIndexPint].pPIntAssign) |= (nBank-2) << 24;
                        nOffset = 24;
                    }
                } else {
                    nOffset = 8;                    
                }
            } else {
                *(g_aGPIOpints[nIndexPint].pPIntAssign) &= 0xFFFF00FF;
                *(g_aGPIOpints[nIndexPint].pPIntAssign) |= (nBank-2) << 8;
                nOffset = 8;              
            }
        }
    }    
    
    if(pa_bOnLevel) {    // level sensitive      
        gpio_clearPIntEdge((pa_nFlag & 0xFFFF) << nOffset, nIndexPint);
    } else {  // edge sensitive
        gpio_setPIntEdge((pa_nFlag & 0xFFFF) << nOffset, nIndexPint);
    }
      
    if(pa_bOnLow) {  // interrupt on active low
        gpio_setPIntInvert((pa_nFlag & 0xFFFF) << nOffset, nIndexPint);
    }
    else {  // interrupt on active high
        gpio_clearPIntInvert((pa_nFlag & 0xFFFF) << nOffset, nIndexPint);
    }
    // clear potential latches due to history 
    gpio_clearPIntLatch((pa_nFlag & 0xFFFF) << nOffset, nIndexPint);
    // unmask interrupt
    gpio_setPIntMask((pa_nFlag & 0xFFFF) << nOffset, nIndexPint);   

        
    gpio_enableInen(pa_nFlag);  
    
    
	// Set-up the Interrupt Resource handle structure, 
	// This will be passed back to the Interrupt Handler later as a pointer.
	pParam->nFlag = (pa_nFlag & 0xFFFF) << nOffset;
	pParam->fnCallback = pa_fnCallback;
	pParam->pClientArg = pa_pClientArg;
	pParam->bOnLow = pa_bOnLow;
	pParam->pData = g_aGPIOpints[nIndexPint].pPIntLatch;
    pParam->pFlagClear = g_aGPIOpints[nIndexPint].pPIntRequest;	
	//pParam->pFlagClear = g_aGPIOpints[nIndexPint].pPIntLatch;	
	pParam->nChannel = pa_nChannel;
    

#else
	
	if(pa_nChannel == PF_CHANNEL_A) {// Select which channel the Interrupt should map to, A or B
		pParam->stPeripheral = g_aGPIObanks[ gpioBank(pa_nFlag) ].intIdChannelA; 	// Map to Channel A
	} 
	
	else {
		pParam->stPeripheral = g_aGPIObanks[ gpioBank(pa_nFlag) ].intIdChannelB;	// Map to Channel B
	}
	
	unsigned long nIVG;
	
	if (pa_nIVG) {	// user requires a fixed ivg
		nIVG = pa_nIVG;
		adi_int_SICSetIVG(pParam->stPeripheral, nIVG);
	} 
	
	else {					// no fixed ivg required, use the default one
		adi_int_SICGetIVG(pParam->stPeripheral, &nIVG);
	}
		
	/*
			Set-up the Interrupt Resource handle structure, 
			This will be passed back to the Interrupt Handler later as a pointer.
	*/
	pParam->nFlag = pa_nFlag;
	pParam->fnCallback = pa_fnCallback;
	pParam->pClientArg = pa_pClientArg;
	pParam->bOnLow = pa_bOnLow;
	pParam->pData = (volatile unsigned short *)g_aGPIObanks[ nBank ].pFIOflagD;
	pParam->pFlagClear = (volatile unsigned short *)g_aGPIObanks[ nBank ].pFIOflagC;	
	pParam->nChannel = pa_nChannel;
	
	/*
			Set-up the GPIOs as INPUTS, and enable them in preperation for making
			them INTERRUPTS.
	*/

	*(g_aGPIObanks[ nBank ].pFIOdirection) &= (unsigned short)~pa_nFlag;	// Set direction as INPUT
	*(g_aGPIObanks[ nBank ].pFIOinen) |= (unsigned short)pa_nFlag;				// Enable the INPUT
	
	
	/*
			Set-up the sensitivity of the INPUTS
	*/
	
	if (pa_bOnLow && pa_bOnHigh) { // interrupt on both edges
	
		*( g_aGPIObanks[ nBank ].pFIOboth ) |= (unsigned short)pa_nFlag;
		*( g_aGPIObanks[ nBank ].pFIOedge ) |= (unsigned short)pa_nFlag;
		*(g_aGPIObanks[ nBank ].pFIOpolar)  &= (unsigned short)~pa_nFlag;	// clear polar register
	} 
	
	else {// clear both register
	
		*( g_aGPIObanks[ nBank ].pFIOboth ) &= (unsigned short)~pa_nFlag;
		
		if (pa_bOnLevel) {
		
			*( g_aGPIObanks[ nBank ].pFIOedge ) &= (unsigned short)~pa_nFlag;	// Set Level sensitivity
		} 
		
		else {
		
			*( g_aGPIObanks[ nBank ].pFIOedge ) |= (unsigned short)pa_nFlag;		// Else Clear Level sensitivity
		}
		
		if (pa_bOnLow) {
		
			*( g_aGPIObanks[ nBank ].pFIOpolar ) |= (unsigned short)pa_nFlag;	// Set Active LOW polarity
		} 
		
		else {
		
			*( g_aGPIObanks[ nBank ].pFIOpolar ) &= (unsigned short)~pa_nFlag;	// Set Active HIGH polarity
		}
		
	}
	
	
	if (pa_nChannel == PF_CHANNEL_A) {	// Map interrupt to Channel A
	
		*(g_aGPIObanks[ nBank ].pFIOmaskAS) = (unsigned short)pa_nFlag;
	} 
	
	else {															// Map interrupt to Channel B
	
		*(g_aGPIObanks[ nBank ].pFIOmaskBS) = (unsigned short)pa_nFlag;
	}

#endif  // __ADSPBF548__



	if (pa_bNestedInt) { 	// Hook the interrupt handler as a NESTED Interrupt
		if(adi_int_CECHook(nIVG, GPIOintHandler, pParam, TRUE) != ADI_INT_RESULT_SUCCESS) { // hook the interrupt handler
			// ERROR !!!
			free(pParam);
			return 0;
		}
	} 
	
	else {								// Hook the interrupt handler without NESTING!
		if(adi_int_CECHook(nIVG, GPIOintHandler, pParam, FALSE) != ADI_INT_RESULT_SUCCESS) { // hook the interrupt handler
			// ERROR !!!
			free(pParam);
			return 0;
		}
	}
	
	adi_int_SICWakeup(pParam->stPeripheral, TRUE);	// Allow this interrupt to WAKE the core id IDLED
	adi_int_SICEnable(pParam->stPeripheral);				// Enable the GPIO peripheral to assert interrupts.

	return (void*)pParam;		// Return a handle (pointer) to the Interrupt resource.
}





/**
 *	@public
 *	@brief		Diables interrupts on the specified GPIO Flag.
 *	
 *				Causes the interrupt to be un-hooked from the specified GPIO Flag.
 *				NOTE: Disabling 1 interrupt, will disable all defined on the same peripheral!
 *	
 *	@param 		pa_pIntInfo		Handle to the interrupt resource
 *
 *	@return		VOID
 *
 *  @see		T_GPIO_INT_PARAM definition.
 *	
 *	TODO: 		Enhance GPIO driver so we only disable the single interrupt, and not multiple!
 **/
void gpio_clearInterrupt(void *pa_pIntInfo) {
  
    if (pa_pIntInfo) {// Check we didn't get a NULL pointer!
  
	    T_GPIO_INT_PARAM *pParam = (T_GPIO_INT_PARAM*)pa_pIntInfo; // Provide a pointer for the parameter data to the defined T_GPIO_INT_PARAM structure.
    		
    #ifdef __ADSPBF548__
	    if (pParam->nChannel == PINT_PERIPHERAL_ID_A) {		// Is it a Channel A or B interrupt?
	        gpio_clearPIntMask(pParam->nFlag, g_aGPIObanks[ gpioBank(pParam->nFlag) ].nPeripheralIdA);
	    } 
	    else {
	        gpio_clearPIntMask(pParam->nFlag, g_aGPIObanks[ gpioBank(pParam->nFlag) ].nPeripheralIdB);
	    }
    #else	   
    
	    if (pParam->nChannel == PF_CHANNEL_A) {		// Is it a Channel A or B interrupt?
		    *(g_aGPIObanks[ gpioBank(pParam->nFlag) ].pFIOmaskAC) = (unsigned short)pParam->nFlag;
	    } 
		
	    else {
		    *(g_aGPIObanks[ gpioBank(pParam->nFlag) ].pFIOmaskBC) = (unsigned short)pParam->nFlag;
	    }
		
	    *( g_aGPIObanks[ gpioBank(pParam->nFlag) ].pFIOedge ) &= (unsigned short)~pParam->nFlag;	// clear edge register
	    *( g_aGPIObanks[ gpioBank(pParam->nFlag) ].pFIOpolar) &= (unsigned short)~pParam->nFlag;		// clear polar register
	    *( g_aGPIObanks[ gpioBank(pParam->nFlag) ].pFIOboth ) &= (unsigned short)~pParam->nFlag;	// clear both register
    #endif		
	    adi_int_SICDisable(pParam->stPeripheral);				// Disable the GPIO peripheral as an interrupt.
	    adi_int_SICWakeup(pParam->stPeripheral, FALSE); // Disable the interrupt from Waking the Core if IDLED.
	
	    unsigned long nIVG;	
		
	    adi_int_SICGetIVG(pParam->stPeripheral, &nIVG);	 // Get the interrupt event level
	    adi_int_CECUnhook(nIVG, GPIOintHandler, pParam); // Unhook the handler from this level
		
	    free(pParam);		// finally free the parameter struct
    }
}




/**
 *	@public
 *	@brief		Returns the total number of Banks, internal and on GPIO expanders
 *	
 *
 *	@return		unsigned short 16-bit integer number of available banks
 *
 **/
unsigned short gpio_getNofBanks (void) {
	return (g_nGPIObankCount + g_nGPIOexpanderBankCount);
}
