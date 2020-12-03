/**
 *	@file 		gpTimerConfig.c
 *	@ingroup 	GP_Timer
 *	
 *	@brief 		General Purpose Timer Driver
 *	
 *						This driver initialises and controls the General purpose timers.
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
 
/** @defgroup GP_Timer General Purpose Timers
 *  @ingroup 	driverapi
 *	
 */
  

//#include <environment.h>
#include <services/services.h>
#include <stdlib.h>
#include "gpTimerConfig.h"									// Hardware Specific timer definitions.

extern unsigned short g_nTIMERcount;				///< Number of GP Timers available on the chip
extern T_GP_TIMER_SPEC g_aTIMERspec[];			///< An array describing the GP timers, defined in timer_global.h

#define TIMER_CW_CONFIG_WORD				0x0000		// t.b.d TODO
#define TIMER_PWM_CONFIG_WORD				0x0009		///< PWM mode out, Enable timer pin, Clock with System Clock
#define TIMER_INT_SINGLE_PULSE_CONFIG_WORD	0x0055		///< PWM mode out, Disable timer pin, Clock with System Clock, Enable interrupt

#define TIMER_POSITIVE_PULSE_BIT			(1 << 2)	// bit which enables a positive action pulse



/**
 *	@private
 *	@brief		Interrupt Handler for handling GP Timers set as Interrupts!
 *	
 *						When a GP Timer activates an interrupt, the ISR from ADI, will call this interrupt handler
 *						if an interrupt has been set-up meaning that this handler has been hooked.
 *	
 *	@param 		*pa_pClientArg	A pointer (from ADI's ISR) to a structure defined by T_GP_TIMER_SPEC.
 *
 *	@return		ADI_INT_HANDLER_RESULT Returns ADI_INT_RESULT_PROCESSED if we handled the interrupt, else ADI_INT_RESULT_NOT_PROCESSED.	
 *
 *  @see			T_GP_TIMER_SPEC definition.
 *		
 **/
#pragma section ("L1_code")
ADI_INT_HANDLER_RESULT TimerInterruptHandler(void *pa_pClientArg) {
  
	T_GP_TIMER_SPEC *pTimer = (T_GP_TIMER_SPEC*)pa_pClientArg;	// Provide a pointer for the parameter data to the defined T_GP_TIMER_SPEC structure.

	if (adi_int_SICInterruptAsserted(pTimer->peripheralIntId) == ADI_INT_RESULT_ASSERTED) {// Is this interrupt really for us?
	
		if (pTimer->fnCallback) {// Check if a CallBack function (Event Handler) was hooked, (Checking we don't have a NULL pointer!)
			pTimer->fnCallback(pTimer->pCallbackArg);		// Call the CallBack funtion, with the provided argument structure.
		}
	
		*(pTimer->pTIMERstatus) = pTimer->nTimilMask;	// clear the interrupt
		
		asm ("ssync;");		// The ssync instruction ensures that the write operation of the interrupt clear suceeded before issuing the RTI instruction.
		
		return ADI_INT_RESULT_PROCESSED;			// Notify ADI's interrupt service routine, that we processed the interrupt
	} 
	
	else {
		return ADI_INT_RESULT_NOT_PROCESSED;	// Notfifying ADI's ISR that we didn't handle the interrupt, because it was meant for us!
	}
}




/**
 *	@public
 *	@brief		Setup a GP Timer for Pulse-Width-Modulation
 *	
 *						Generates a PWN signal on the specified timer's pin
 *	
 *	@param 		pa_nTimerNr			Timer identifier (which timer to configure)
 *	@param 		fPeriod					The timer's period, measured in ms.
 *	@param 		fPulsWidth			The timer's pulse width, measures in ms.
 *	@param 		bPositivePulse	true or false, polarity of the generated pulse.
 *	@param 		pa_nSystemClk		The System Clock frequency in Hz
 *
 *	@return		Returns a handle to the timer resource.	
 *
 *  @see			timer_close() and T_GP_TIMER_SPEC and T_GP_TIMER_INST definitions.
 *		
 **/
T_GP_TIMER_INST *timer_pwmout_setup (	unsigned short pa_nTimerNr, 
																			double fPeriod,									// [ms]
																			double fPulsWidth,							// [ms]
																			bool bPositivePulse,
																			unsigned long pa_nSystemClk) {	// [Hz]
	
	T_GP_TIMER_INST *pTimerHndl = (T_GP_TIMER_INST *)malloc (sizeof (T_GP_TIMER_INST) );	// Allocate some memory for the handle
	
	unsigned short i = 0;

	while ((i < g_nTIMERcount) && (g_aTIMERspec[i].nTimerNr != pa_nTimerNr)) {	// Provide an index-lookup to the specified timer.
		i++;
	}
	
	if (i == g_nTIMERcount) {// The indexed value exceeds the number of timers, return NULL handle.
		free (pTimerHndl);
		pTimerHndl = 0;
	} 
	
	else {
		
		if (!g_aTIMERspec[i].bInUse) { // Ensure the specified Timer is not in use.
		
			
			g_aTIMERspec[i].bInUse = true;	// Mark it as in use
			
			T_TIMER_PLATFORMINIT_PARAM aTimerPlatformInit;
				
			aTimerPlatformInit.nTimer = pa_nTimerNr;
			aTimerPlatformInit.nConfig = TIMER_PWM_CONFIG_WORD;
			aTimerPlatformInit.cTMRCLKinputPin = 0;
			
			timer_platformInit (&aTimerPlatformInit);	// Call the platform specific initialisation
			
			*(g_aTIMERspec[i].pTIMERconfig) = TIMER_PWM_CONFIG_WORD;	// Write the TIMERx_CONFIG register 
						
			if (bPositivePulse) {
				*(g_aTIMERspec[i].pTIMERconfig) |= TIMER_POSITIVE_PULSE_BIT;	// Set positive action pulse
			}
			
			/* Calculate and assign the the TIMERxPERIOD and TIMERxWIDTH registers */
			/* Converting from MS to SystemClock cycles */
			*(g_aTIMERspec[i].pTIMERperiod) = (unsigned long)(fPeriod * (double)pa_nSystemClk / 1000.0);
			*(g_aTIMERspec[i].pTIMERwidth) 	= (unsigned long)(fPulsWidth * (double)pa_nSystemClk / 1000.0);
			
			/* Populate the handle information for this GP timer instance */
			pTimerHndl->nSystemTimerNr 	= i;
			pTimerHndl->nPeriod 				= *(g_aTIMERspec[i].pTIMERperiod);
			pTimerHndl->fPwmPeriod 			= fPeriod;
			pTimerHndl->fPwmDutyCycle 	= fPulsWidth / fPeriod * 100.0;
			pTimerHndl->nSystemClk 			= pa_nSystemClk;
		} 
		
		else {
			free (pTimerHndl);
			pTimerHndl = 0;			// Provide a NULL pointer!
		}
	}
	
	return pTimerHndl;			// Return a Handle to the timer resource.
}




/**
 *	@public
 *	@brief		Enable a GP Timer
 *	
 *						Enables the specified System Timer
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *  @see			timer_disable() and T_GP_TIMER_INST definition.
 *		
 **/
void timer_enable (T_GP_TIMER_INST *pTimerHndl) {
	
	if (pTimerHndl) {		// Check we don't have a NULL pointer!
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERenable) = g_aTIMERspec[pTimerHndl->nSystemTimerNr].nEnableDisableMask;
		asm ("ssync;");
	}
}




/**
 *	@public
 *	@brief		Disable a GP Timer gracefully
 *	
 *						Disables the specified System Timer without aborting the current cycle.
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *  @see			timer_disable() and T_GP_TIMER_INST definition.
 *		
 **/
void timer_disable_gracefully(T_GP_TIMER_INST *pTimerHndl) {
  
	if (pTimerHndl) {		// Check we don't have a NULL pointer!
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERdisable) = g_aTIMERspec[pTimerHndl->nSystemTimerNr].nEnableDisableMask;
		asm ("ssync;");
	}
}



/**
 *	@public
 *	@brief		Disable a GP Timer
 *	
 *						Disables the specified System Timer
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *  @see			timer_disable() and T_GP_TIMER_INST definition.
 *		
 **/
void timer_disable (T_GP_TIMER_INST *pTimerHndl) {
  
	if (pTimerHndl) {		// Check we don't have a NULL pointer!
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERdisable) = g_aTIMERspec[pTimerHndl->nSystemTimerNr].nEnableDisableMask;
		/* to force an immediate stop, write to timerstatus */
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERstatus) |= g_aTIMERspec[pTimerHndl->nSystemTimerNr].nTRunMask;
		asm ("ssync;");
	}
}





/**
 *	@public
 *	@brief		Configure Period of a GP Timer
 *	
 *						Set's the timer period
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *	@param		pa_nValue				Period in No. of System Clock Ticks
 *
 *  @see			timer_get_period() and T_GP_TIMER_INST definition.
 *		
 **/
void timer_set_period (T_GP_TIMER_INST *pTimerHndl, unsigned long pa_nValue) {
	
	if (pTimerHndl) {		// Check we don't have a NULL pointer!
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERperiod) = pa_nValue;
	}
}




/**
 *	@public
 *	@brief		Get the period of a GP Timer
 *	
 *						Returns an unsigned long integer of the Timer period, in System Clock Ticks.
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *  @see			timer_set_period() and T_GP_TIMER_INST definition.
 *		
 **/
unsigned long timer_get_period (T_GP_TIMER_INST *pTimerHndl) {
	return *(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERperiod);
}




/**
 *	@public
 *	@brief		Configure Width of a GP Timer
 *	
 *						Set's the timer's width
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *	@param		pa_nValue				Width in No. of System Clock Ticks
 *
 *  @see			timer_get_width() and T_GP_TIMER_INST definition.
 *		
 **/
void timer_set_width (T_GP_TIMER_INST *pTimerHndl, unsigned long pa_nValue) {

  	if (pTimerHndl) {	// Check we don't have a NULL pointer!
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERwidth) = pa_nValue;
	}
}




/**
 *	@public
 *	@brief		Get the period of a GP Timer
 *	
 *						Returns an unsigned long integer of the Timer width, in System Clock Ticks.
 *	
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *  @see			timer_set_width() and T_GP_TIMER_INST definition.
 *		
 **/
unsigned long timer_get_width (T_GP_TIMER_INST *pTimerHndl) {
	return *(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERwidth);
}





/**
 *	@public
 *	@brief		Write to a GP timers RUN BIT
 *	
 *						Writing to a Timer's run bit, can be used to halt the timer immediately
 *						without reaching the end of the period.
 *
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *  @see			timer_get_runbit() and T_GP_TIMER_INST definition.
 *		
 **/
void timer_set_runbit (T_GP_TIMER_INST *pTimerHndl) {
	
  	if (pTimerHndl) {		// Check we don't have a NULL pointer!
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERstatus) = g_aTIMERspec[pTimerHndl->nSystemTimerNr].nTRunMask;
	}
}





/**
 *	@public
 *	@brief		Read a GP timer's RUN BIT
 *	
 *						Returns bool, true or false, value of the Timer's Run Bit. 
 *						Is the Timer currently running?
 *
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *	@return		Returns a BOOL bit, true or false, indicating the running status of the timer.
 *
 *  @see			timer_get_runbit() and T_GP_TIMER_INST definition.
 *		
 **/
bool timer_get_runbit (T_GP_TIMER_INST *pTimerHndl) {

  	if ((*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERstatus) & g_aTIMERspec[pTimerHndl->nSystemTimerNr].nTRunMask) == 0) {
		return false;
	} 
	
	else {
		return true;
	}
}





/**
 *	@public
 *	@brief		Read a GP timer's Counter register
 *	
 *						Returns an unsigned long integer of the Timer's current count.
 *
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *	@return		Returns an unsigned long integer, of the Timer's counter register.
 *
 *  @see			T_GP_TIMER_INST definition.
 *		
 **/
unsigned long timer_get_counter (T_GP_TIMER_INST *pTimerHndl) {
	return *(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERcounter);
}




/**
 *	@public
 *	@brief		Set's the PWM pulse width in ms.
 *	
 *
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *	@param		pa_fPulsWidth		Width in ms, of the pulse-width modulation, pulse.
 *
 *
 *  @see			T_GP_TIMER_INST definition.
 *		
 **/
void timer_set_pwm_puls_width (T_GP_TIMER_INST *pTimerHndl, double pa_fPulsWidth) {		// [ms]
	
	if (pTimerHndl) {		// Check we don't have a NULL pointer.
		/* Convert width from ms, to system clock ticks. */
		*(g_aTIMERspec[pTimerHndl->nSystemTimerNr].pTIMERwidth) = pa_fPulsWidth * (double)(pTimerHndl->nSystemClk) / 1000.0;
	}
}





/**
 *	@public
 *	@brief		Stop and Disable Timer, Frees Memory used for HANDLE
 *	
 *
 *	@param 		*pTimerHndl			Pointer to the System Timer Resource handle.
 *
 *	@return		Returns a T_ERROR_CODE upon failure.
 *
 *  @see			T_GP_TIMER_INST definition.
 *		
 **/
T_ERROR_CODE timer_close (T_GP_TIMER_INST *pTimerHndl) {
	
  T_ERROR_CODE erResult = ERR_NONE;
  
	if (pTimerHndl) {	// Check we don't have a NULL pointer.
	
		timer_disable (pTimerHndl);	// Disable the timer, just to be on the safe side!
		
		if (g_aTIMERspec[pTimerHndl->nSystemTimerNr].fnCallback) {	// a callback was installed, so unhook the interrupt handler
		
			/* TODO - Multiple Timers on the same interrupt channel will get disabled here! (I think :) ) */
		  adi_int_SICDisable(g_aTIMERspec[pTimerHndl->nSystemTimerNr].peripheralIntId);
			
			if (adi_int_CECUnhook(g_aTIMERspec[pTimerHndl->nSystemTimerNr].nIVG, TimerInterruptHandler, (void *)&g_aTIMERspec[pTimerHndl->nSystemTimerNr]) != ADI_INT_RESULT_SUCCESS) {
				erResult = ERR_GENERIC;				
			}
		}
		
		g_aTIMERspec[pTimerHndl->nSystemTimerNr].bInUse = false;	// Mark timer as available
		
		free (pTimerHndl);	// free the handle memory
	}
	
	return erResult;
}





/**
 *	@public
 *	@brief		Creates and configures a GP Timer instance.
 *
 *						On sucess a handle to a GP Timer resource is returned.						
 *
 *	@param 		pa_pTimerSetup				pointer to the setup structure
 *	@return		Returns a T_GP_TIMER_INST pointer to a handle, or a NULL pointer on failure.
 *
 *  @see		T_GP_TIMER_SETUP_PARAM definition.
 *		
 **/
T_GP_TIMER_INST *timer_gp_setup (T_GP_TIMER_SETUP_PARAM *pa_pTimerSetup) {
	
	T_GP_TIMER_INST *pTimerHndl = (T_GP_TIMER_INST *)malloc (sizeof (T_GP_TIMER_INST) );	// get the handle memory
	pTimerHndl->nSystemTimerNr = 0;
	pTimerHndl->nPeriod = 0;
	pTimerHndl->fPwmPeriod = 0.0;
	pTimerHndl->fPwmDutyCycle = 0.0;
	pTimerHndl->nSystemClk = 0;
	if (pTimerHndl) {
	
		unsigned short i = 0;
		
		while ((i < g_nTIMERcount) && (g_aTIMERspec[i].nTimerNr != pa_pTimerSetup->nTimerNr)) {		// Provide an index-lookup to the specified timer.
			i++;
		}
		
		if (i == g_nTIMERcount)	{	//  Timer doesn't exist! Provide a NULL pointer! 
			free (pTimerHndl);
			pTimerHndl = 0;
		} 
		
		else {
		
			if (!g_aTIMERspec[i].bInUse) {// Check timer isn't in use
			
				g_aTIMERspec[i].bInUse = true;		// Mark it as in use
				
				T_TIMER_PLATFORMINIT_PARAM aTimerPlatformInit;
				
				aTimerPlatformInit.nTimer = pa_pTimerSetup->nTimerNr;
				aTimerPlatformInit.nConfig = pa_pTimerSetup->nConfig;
				aTimerPlatformInit.cTMRCLKinputPin = pa_pTimerSetup->cTMRCLKinputPin;
				
				timer_platformInit (&aTimerPlatformInit);		// Call the platform specific inistialisation function.
				
				*(g_aTIMERspec[i].pTIMERconfig) = pa_pTimerSetup->nConfig;		// Set TIMERx_CONFIG register.
		
			    if (pa_pTimerSetup->bPositivePulse) {
					*(g_aTIMERspec[i].pTIMERconfig) |= TIMER_POSITIVE_PULSE_BIT;	// set positive action pulse
				} else {
				    *(g_aTIMERspec[i].pTIMERconfig) &= ~(TIMER_POSITIVE_PULSE_BIT);	// set negative action pulse
				}
				
				
				/* Set the TIMERx_PERIOD and TIMERx_WIDTH registers */
				*(g_aTIMERspec[i].pTIMERperiod) = pa_pTimerSetup->nPeriod;
				*(g_aTIMERspec[i].pTIMERwidth) = pa_pTimerSetup->nWidth;
				
				
				/* Populate the handle information for this GP timer instance */
				pTimerHndl->nSystemTimerNr = i;
				pTimerHndl->nPeriod = *(g_aTIMERspec[i].pTIMERperiod);
				pTimerHndl->nSystemClk = pa_pTimerSetup->nSystemClk;
				
				
				if (pa_pTimerSetup->fnCallback) { // Setup interrupts if required (Non NULL pointer)
				
					/* Copy function and argument pointers to the Timer configuration array */
					g_aTIMERspec[i].fnCallback = pa_pTimerSetup->fnCallback;
					g_aTIMERspec[i].pCallbackArg = pa_pTimerSetup->pCallbackArg;

//#ifdef _USE_FIXED_GP_TIMER_IVG_
					/* Assign a custom Event Handling Level */
					if(pa_pTimerSetup->nIVG) {
						g_aTIMERspec[i].nIVG = pa_pTimerSetup->nIVG;
						adi_int_SICSetIVG(g_aTIMERspec[i].peripheralIntId, g_aTIMERspec[i].nIVG);
//#else				
					} else {
						adi_int_SICGetIVG(g_aTIMERspec[i].peripheralIntId, &g_aTIMERspec[i].nIVG);
					}
//#endif					
					/* Hoook the interrupt into ADI's ISR */
					if(adi_int_CECHook(g_aTIMERspec[i].nIVG, TimerInterruptHandler, (void *)&g_aTIMERspec[i], true) == ADI_INT_RESULT_SUCCESS) {
						adi_int_SICWakeup(g_aTIMERspec[i].peripheralIntId, TRUE);		// Interrupt can wake core from IDLE
						adi_int_SICEnable(g_aTIMERspec[i].peripheralIntId);					// Enable interrupt
					} 
					
					else {
					
						free (pTimerHndl);	// Free the HANDLE memory
						pTimerHndl = 0;			// Set a NULL pointer
					}							
				
				}
			} 
			
			else {
				free (pTimerHndl);			// Free the HANDLE memory
				pTimerHndl = 0;					// Set a NULL pointer
			}
		}
	}
	 
	else {		// Malloc failed to acquire memory for the HANDLE, return a NULL pointer!
		pTimerHndl = 0;
	}
		
	return pTimerHndl;	// Return a pointer to our HANDLE
}

