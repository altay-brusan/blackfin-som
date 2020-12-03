/**
 *	@file 		gpTimerConfig.h
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
 
 


#ifndef _GP_TIMER_CONFIG_INCLUDED_
#define _GP_TIMER_CONFIG_INCLUDED_

#include <services/services.h>

typedef void (*T_GP_TIMER_CALLBACK)(void *pa_pCallbackArg);	///< Pointer to a Callback function.

#define ERR_TIMER_NOT_FOUND			(ERR_GENERIC - 10)					///< Timer not Found error response
#define ERR_TIMER_IN_USE			(ERR_GENERIC - 20)						///< Timer in use error response

#include <Datatypes.h>
#include <services/services.h>

typedef void (*T_TIMER_CALLBACK)(void);





/**
 *	@brief 			Structure, defscribing a System Timer's configuration
 *	
 *							Describes configuration for a GP Timer
 *	
 **/
typedef struct {
	bool 										bInUse;							///< Flag to mark if the Timer is already in use.
	unsigned short 					nTimerNr;						///< System Timer Identifier Number.
	ADI_INT_PERIPHERAL_ID 	peripheralIntId;		///< Peripheral ID of the GP Timer.
	unsigned long 					nIVG;								///< Interrupt Event Level to Handle this Timer at.
	unsigned long 					nEnableDisableMask; ///< Bit Mask, for the Enable/Disable bit of the TIMER.
	unsigned long 					nTRunMask;					///< Bit Mask, for the RUN BIT of the TIMER.
	unsigned long 					nTimilMask;					///< Bit Mask, for the Interrupt Assertion for this TIMER.
	volatile unsigned long 	*pTIMERenable;			///< Pointer to TIMER enable register
	volatile unsigned long 	*pTIMERdisable;			///< Pointer to TIMER disable register
	volatile unsigned long 	*pTIMERstatus;			///< Pointer to TIMER status register
	volatile unsigned short *pTIMERconfig;			///< Pointer to TIMER's configuration register.
	volatile unsigned long 	*pTIMERcounter;			///< Pointer to TIMER's count register.
	volatile unsigned long 	*pTIMERperiod;			///< Pointer to TIMER's period register.
	volatile unsigned long 	*pTIMERwidth;				///< Pointer to TIMER's width register.
	T_GP_TIMER_CALLBACK 		fnCallback;					///< Pointer to a Callback function.
	void 										*pCallbackArg;			///< Pointer to an argument, or structure of arguments for the Callback function.
} T_GP_TIMER_SPEC;



/**
 *	@brief 			Structure, used as parameter for timer_platformInit
 **/
typedef struct {
	unsigned short nTimer;
	unsigned short nConfig;
	unsigned char cTMRCLKinputPin;
} T_TIMER_PLATFORMINIT_PARAM;


/**
 *	@brief 			Structure, used as parameter for timer_gp_setup
 **/
typedef struct {
	unsigned short nTimerNr;						///< System Timer identifier Number
	unsigned short nConfig;							///< Timer Config Register value
	unsigned long nWidth;							///< Width of pulse in PWM mode, in System Clock Ticks
	unsigned long nPeriod;							///< Period of Timer, in System Clock Ticks
	unsigned long nIVG;								///< select different IVG, use 0 for standard
	unsigned char cTMRCLKinputPin;					///< select the timer clock input pin, 0 for standard pin
	bool bPositivePulse;							///< positive pulse of PWM
	unsigned long nSystemClk;						///< [Hz]
	T_ERROR_CODE erResult;
	T_GP_TIMER_CALLBACK fnCallback;					///< callback function
	void *pCallbackArg;								///< argument for the callback function
} T_GP_TIMER_SETUP_PARAM;


/**
 *	@brief 			Structure, defining a HANDLE to a GP System Timer instance
 *	
 *							Holds Instance data for a Timer Handle
 *	
 **/
typedef struct {
	unsigned short 					nSystemTimerNr;			///<	System Timer Identifier Number
	unsigned long 					nPeriod;						///<	Period of Timer in System Clock Ticks
	double 									fPwmPeriod;					///< 	PWM Period in ms
	double 									fPwmDutyCycle;			///< 	[%]
	unsigned long 					nSystemClk;					///<	The System Clock frequency in Hz
} T_GP_TIMER_INST;

// prototype declarations
T_GP_TIMER_INST *timer_pwmout_setup (	unsigned short pa_nTimerNr, 
									double fPeriod,					// [ms]
									double fPulsWidth,				// [ms]
									bool bPositivePulse,
									unsigned long pa_nSystemClk);	// [Hz]
T_GP_TIMER_INST *timer_gp_setup (T_GP_TIMER_SETUP_PARAM *pa_pTimerSetup);									
void timer_enable (T_GP_TIMER_INST *pTimerHndl);
void timer_disable (T_GP_TIMER_INST *pTimerHndl);
void timer_set_period (T_GP_TIMER_INST *pTimerHndl, unsigned long pa_nValue);
unsigned long timer_get_period (T_GP_TIMER_INST *pTimerHndl);
void timer_set_width (T_GP_TIMER_INST *pTimerHndl, unsigned long pa_nValue);
void timer_set_pwm_puls_width (T_GP_TIMER_INST *pTimerHndl, double pa_fPulsWidth);	// [ms]
T_ERROR_CODE timer_close (T_GP_TIMER_INST *pTimerHndl);
bool timer_platformInit(T_TIMER_PLATFORMINIT_PARAM *pa_aTimerInit);
unsigned long timer_get_counter (T_GP_TIMER_INST *pTimerHndl);
									
#endif

