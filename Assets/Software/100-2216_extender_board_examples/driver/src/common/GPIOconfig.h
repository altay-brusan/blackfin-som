/**
 *	@file 		GPIOconfig.h
 *	@ingroup 	GPIO
 *	
 *	@brief 		Hardware abstracted GPIO Driver to control all GPIO operations.
 *	
 *						This driver can also incorporate GPIO expander drivers into a simple
 *						to use unified generic GPIO driver model.
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, Daniel Weber
 *	
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 * 	@see GPIOconfig.c
 **/

#ifndef _GPIO_CONFIG_H_INCLUDED_
#define _GPIO_CONFIG_H_INCLUDED_

#include <services/services.h>
#include <datatypes.h>


typedef unsigned long T_GPIO_MASK;	///< gpio mask (contains flag-bitmask and gpio bank).
typedef void (*T_GPIO_CALLBACK)(T_GPIO_MASK pa_tFlag, void *pa_pClientArg); ///< Callback function pointer, for a GPIO interrupt handler.
typedef void (*T_GPIO_EXP_FUNC)(T_GPIO_MASK pa_tFlag);	///< GPIO Expander driver function pointer.

#define gpioMask(bank, bitmask) (T_GPIO_MASK)(((bank) << 16) + (bitmask))	///< Macro building a gpio mask of a bitmask and a gpio bank
#define gpioBank(gpioMask) (unsigned short)((gpioMask) >> 16)	///< Macro to extract bank information from a gpio mask

#ifndef __ADSPBF548__
    #define PF_CHANNEL_A 0		///< GPIO interrupts on channel A
    #define PF_CHANNEL_B 1		///< GPIO interrupts on channel B
#endif    

extern unsigned int g_nGPIObankCount;

#define MAX_GPIO_EXPANDER			5	///< Maximum number of GPIO expanders

/**
 *	@brief 	Enumeration defining the two possible Peripheral IDs for peripheral interrupts.
 *	
 *	
 **/
#ifdef __ADSPBF548__
    typedef enum ENUM_PINT_PERIPHERAL_ID{PINT_PERIPHERAL_ID_A=0, PINT_PERIPHERAL_ID_B=1} T_PINT_PERIPHERAL_ID;
#endif



/*
typedef struct {
		ADI_INT_PERIPHERAL_ID stChannelA;
		ADI_INT_PERIPHERAL_ID stChannelB;
	} T_GPIO_SPEC;
*/


/**
 *	@brief 	Structure defining a GPIO expander, providing HANDLES to the Expander driver.
 *	
 *	
 **/
typedef struct {
		bool 			bInUse;						///< Flag marking the GPIO Expander as IN USE
		T_GPIO_EXP_FUNC extgpio_becomeInput;		///< GPIO Expander's becomeInput driver function.
		T_GPIO_EXP_FUNC extgpio_becomeOutput;		///< GPIO Expander's becomeOutput driver function.
		T_GPIO_EXP_FUNC extgpio_set;				///< GPIO Expander's set driver function.
		T_GPIO_EXP_FUNC extgpio_clear;				///< GPIO Expander's clear driver function.
		T_GPIO_EXP_FUNC extgpio_toggle;				///< GPIO Expander's toggle driver function.
		unsigned short 	(*extgpio_readFlag)(T_GPIO_MASK pa_tFlag);	///< GPIO Expander's becomeInput driver function.
		unsigned short 	nBank;						///< GPIO generic driver, assigned Bank No.
		
} T_GPIO_EXP_SPEC;




/**
 *	@brief 			Hardware specific description of GPIO banks
 *					Forms a Hardware Abstraction layer for the GPIO driver.
 *	
 **/
 
#if defined (__ADSPBF548__)
 
typedef struct {
        volatile unsigned short *pPortFer;          ///< Pointer to BANKS' GPIO Function Enable register
        volatile unsigned short *pPort;             ///< Pointer to BANKS' GPIO Data register
        volatile unsigned short *pPortSet;          ///< Pointer to BANKS' GPIO Data Set register
        volatile unsigned short *pPortClear;        ///< Pointer to BANKS' GPIO Data Clear register
        volatile unsigned short *pPortDirSet;       ///< Pointer to BANKS' GPIO Direction Set register
        volatile unsigned short *pPortDirClear;     ///< Pointer to BANKS' GPIO Direction Clear register
        volatile unsigned short *pPortInen;         ///< Pointer to BANKS' GPIO Input Enable register
        volatile unsigned long *pPortMux;          ///< Pointer to BANKS' GPIO Multiplexer Control register
        unsigned char nPeripheralIdA;               ///< Interrupt Channel peripheral ID A.
        unsigned char nPeripheralIdB;               ///< Interrupt Channel peripheral ID B.
} T_GPIO_BANKS;
    
typedef struct {
        ADI_INT_PERIPHERAL_ID 	tPeripheralId;  	///< Interrupt Peripheral ID.
        volatile unsigned long *pPIntMaskSet;      ///< Pointer to BANKS' Pin Interrupt Set Mask register
        volatile unsigned long *pPIntMaskClear;    ///< Pointer to BANKS' Pin Interrupt Clear Mask register
        volatile unsigned long *pPIntRequest;      ///< Pointer to BANKS' Interrupt Request register
        volatile unsigned long *pPIntAssign;       ///< Pointer to BANKS' Pin Interrupt Assignment register
        volatile unsigned long *pPIntEdgeSet;      ///< Pointer to BANKS' Interrupt Set Edge register
        volatile unsigned long *pPIntEdgeClear;    ///< Pointer to BANKS' Interrupt Clear Edge register
        volatile unsigned long *pPIntInvertSet;    ///< Pointer to BANKS' Pin Interrupt Set Invert register
        volatile unsigned long *pPIntInvertClear;  ///< Pointer to BANKS' Pin Interrupt Clear Invert register
        volatile unsigned long *pPIntPinState;     ///< Pointer to BANKS' Pin Interrupt Pin State 
        volatile unsigned long *pPIntLatch;        ///< Pointer to BANKS' Interrupt Latch register
} T_GPIO_PINT;

#else
 
typedef struct {
		ADI_INT_PERIPHERAL_ID 	intIdChannelA;	///< Interrupt Channel peripheral ID for Channel A.
		ADI_INT_PERIPHERAL_ID 	intIdChannelB;	///< Interrupt Channel peripheral ID for Channel B.
		volatile unsigned short *pFIOflagC;		///< Pointer to BANKS' GPIO Write to Clear register.
		volatile unsigned short *pFIOflagD;		///< Pointer to BANKS' GPIO Data (Status) register.
		volatile unsigned short *pFIOflagS;		///< Pointer to BANKS' GPIO Write to Set register.
		volatile unsigned short *pFIOflagT;		///< Pointer to BANKS' GPIO Write to TOGGLE register.
		volatile unsigned short *pFIOinen;		///< Pointer to BANKS' GPIO Input Enable register.
		volatile unsigned short *pFIOdirection; ///< Pointer to BANKS' GPIO Direction config register.
		volatile unsigned short *pFIOedge;		///< Pointer to BANKS' GPIO Edge Sensing config register.
		volatile unsigned short *pFIOpolar;		///< Pointer to BANKS' GPIO Polarity config register.
		volatile unsigned short *pFIOboth;		///< Pointer to BANKS' GPIO Both edge senssing config register.
		volatile unsigned short *pFIOmaskAC;	///< Pointer to BANKS' GPIO Interrupt on Channel A clear register.
		volatile unsigned short *pFIOmaskAS;	///< Pointer to BANKS' GPIO Interrupt on Channel A set register.
		volatile unsigned short *pFIOmaskBC;	///< Pointer to BANKS' GPIO Interrupt on Channel B clear register.
		volatile unsigned short *pFIOmaskBS;	///< Pointer to BANKS' GPIO Interrupt on Channel B set register.
		volatile unsigned short *pPortFER;	    ///< Pointer to BANKS' GPIO Function enable register.		
		volatile unsigned short *pPortMUX;	    ///< Pointer to BANKS' GPIO MUX register.		
	} T_GPIO_BANKS;

#endif

	

	
	
/**
 *	@brief 			Interrupt Handler Description for GPIO hooked events
 *		
 *	
 **/
typedef struct {
	T_GPIO_MASK 			nFlag;					///< GPIO Pin Specific Identifier (e.g. _PF14 etc)
	ADI_INT_PERIPHERAL_ID 	stPeripheral;		    ///< Interrupt peripheral ID.
	unsigned int 			nChannel;				///< Channel ID
	T_GPIO_CALLBACK 		fnCallback;				///< Pointer to the Callback function.
	void 					*pClientArg;			///< Arguments for the Callback function.
	bool 					bOnLow;					///< Polarity of the Interrupt
#if defined (__ADSPBF548__)
	volatile unsigned long  *pData;					///< Pointer to the GPIO Bank's Data register
	volatile unsigned long  *pFlagClear;			///< Pointer to the GPIO Bank's Clear register
#else 
	volatile unsigned short  *pData;				///< Pointer to the GPIO Bank's Data register
	volatile unsigned short  *pFlagClear;			///< Pointer to the GPIO Bank's Clear register
#endif
} T_GPIO_INT_PARAM;

//prototype definitions
/*
	@param pa_nFlag
	
	sets a certain gpio flag
	
*/
void gpio_set(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	clears a certain gpio flag
*/
void gpio_clear(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	sets the specified gpio flag to become input (enables the input buffer)
*/
void gpio_becomeInput(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	sets the specified gpio flag to become output
*/
void gpio_becomeOutput(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	toggles a certain gpio flag
*/
void gpio_toggle(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
*/
unsigned short gpio_readFlag(T_GPIO_MASK pa_nFlag);

unsigned long gpio_readPort(T_GPIO_MASK pa_nBank);

/*
	@param pa_nFlag
	@param pa_bOnLevel
	@param pa_bOnLow
	@param pa_nChannel
	@param pa_fnCallback
	@return handle to interrupt resource
	
	experimental: sets up a interrupt for gpio flag
	
*/
void *gpio_setupInterrupt(T_GPIO_MASK pa_nFlag, 
                          bool pa_bOnLevel, 
                          bool pa_bOnLow, 
                          bool pa_OnHigh, 
                          unsigned long pa_nChannel, 
                          unsigned long pa_nIVG, 
                          bool pa_bNestedInt, 
                          T_GPIO_CALLBACK pa_fnCallback, 
                          void *pa_pClientArg);

/*
	@param pa_nFlag
	@return the polar register value for the specified flag
*/
unsigned short gpio_readPolar(T_GPIO_MASK pa_nFlag);

/**
	@param pa_nFlag
	@return the edge register value for the specified
*/
unsigned short gpio_readEdge(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	sets the specified flag in the polar register
*/
void gpio_setPolar(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	clears the specified flag in the polar register
*/
void gpio_clearPolar(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	clears the specified flag in the edge register
*/
void gpio_setEdge(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	clears the specified flag in the edge register
*/
void gpio_clearEdge(T_GPIO_MASK pa_nFlag);

/*
	@param pa_nFlag
	
	disables the input buffer of the specified flag
*/
void gpio_disableInen(T_GPIO_MASK pa_nFlag);


/*
	@param pa_nFlag
	
	enables the input buffer of the specified flag
*/
void gpio_enableInen(T_GPIO_MASK pa_nFlag);


/*
	@param pa_nFlag
	
	set flasg as GPIO
*/
void gpio_disableFunction(T_GPIO_MASK pa_nFlag);


/*
	@param pa_nFlag
	
	set flag as Peripheral function
*/
void gpio_enableFunction(T_GPIO_MASK pa_nFlag);


/*
	@param pa_pIntInfo handle to interrupt resource
	
	experimental: frees resources allocated by the interrupt handler
	
*/
void gpio_clearInterrupt(void *pa_pIntInfo);


/*
	
	returns the nof banks installed	
*/
unsigned short gpio_getNofBanks (void);



T_ERROR_CODE gpio_AddIOExpander (	unsigned short pa_nBank,
									T_GPIO_EXP_FUNC extgpio_becomeInput,
									T_GPIO_EXP_FUNC extgpio_becomeOutput,
									T_GPIO_EXP_FUNC extgpio_set,
									T_GPIO_EXP_FUNC extgpio_clear,
									T_GPIO_EXP_FUNC extgpio_toggle,
									unsigned short (*extgpio_readFlag)(T_GPIO_MASK),
									void *pa_pGeneric);
#endif

//@}
//@}

