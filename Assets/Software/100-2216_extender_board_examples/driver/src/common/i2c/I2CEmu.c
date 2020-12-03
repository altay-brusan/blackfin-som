/**
 *	@file 		I2C_bitbang.c
 *	@ingroup 	I2C
 *	
 *	@brief 		I2C GPIO Bit Basher.
 *	
 *				Emulates a fully featured (excluding Arbitration, i.e. single master only)
 *				I2C Interface controller.
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	James Walmsley
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
 
/** @defgroup I2C
 *	@ingroup driverapi
 *	I2C Interface Controller
 */

#include "I2CEmu.h"			///< PUBLIC Definitions of I2C Emulator.
#include "../GPIOconfig.h"			///< GPIO Driver.
#include "../gpTimerconfig.h"		///< General Purpose Timer Driver.
#include "I2CMan.h"

// TODO:
// Define a global timer handle. With an In-use flag, so that unlimited I2C instances can use the same timer.
// Or even better perhaps use (or define if needs be) a timer API function to wait on a timer being used to be free'd.

/**
 *	@private
 *	@brief		Enumeration of all the possible states within the I2C State Machine.
 **/
typedef enum {
	I2C_STATE_START,				///< START 		state	- Put start condition on the BUS.
	I2C_STATE_STOP,					///< STOP 		state	- Put stop condition on the BUS.
	I2C_STATE_TX,					///< TX 		state	- Clock each bit of a byte onto the BUS.
	I2C_STATE_RX,					///< RX 		state	- Clock in a byte of data from the BUS.
	I2C_STATE_TX_ACK,				///< TX_ACK 	state	- Check for an ACK signal from the slave on the BUS.
	I2C_STATE_RX_ACK,				///< RX_ACK 	state	- Acknowledge receipt of data.
	I2C_STATE_RX_NACK,				///< RX_ACK 	state	- Non-acknowledge the received data.
	I2C_STATE_CLOCK,				///< CLOCK 		state	- Make the clock go Tick Tock for a specified number of Ticks.
	I2C_STATE_WAIT,					///< WAIT 		state	- Wait one Timer Tick, while the clock is streched.
	I2C_STATE_SCL_LOW,				///< SCL_LOW 	state	- Special state to hold the Clock low for 2 timer ticks.
	I2C_STATE_FREE,					///< FREE 		state	- Current work is complete, disable the timer, and reset the stack.
} I2C_STATE;

/**
 *	@private
 *	@brief		Defining an I2C Message.
 **/
typedef struct {
	unsigned char 	mcData;			///< Data to be sent, or that was RX'd.
	bool			mbAcked;		///< TRUE (1) if the byte was acknowledged by the receiver.
} T_I2C_MESSAGE;

/**
 *	@private
 *	@brief		Defines a state machine Context.
 *				Holds information that describes the complete state of the state machine.
 **/
typedef struct {
	I2C_STATE		mState;			///< Context State.
	unsigned long	mnStateSeq;		///< Context State sequence number. (Used to clock through Micro-operations within each state).
} T_I2C_CONTEXT;

/**
 *	@private
 *	@brief		Defines an I2C State Machine Stack
 **/
typedef struct {
	T_I2C_CONTEXT	mContexts[5];	///< 5 Deep context stack.
	unsigned long	mnStackPtr;		///< Stack pointer (index to top of the stack).
} T_I2C_STACK;

/**
 *	@private
 *	@brief		I2C State Machine Control
 **/
typedef struct {
	bool			mbClk;			///< Logical Memory of the current clock state. (High or Low, 1 or 0).
	T_I2C_CONTEXT	mCurrentTask;	///< The context that is being currently executed. (Called a task).
	T_I2C_STACK		mTaskStack;		///< Stack of tasks that have been pushed, and can be popped back into the CurrentTask.
} I2C_CONTROL;

/**
 *	@private
 *	@brief		I2C Structure Definition - This can only be seen by this file.
 *				Completes the T_I2C_BITBANG definition from its incomplete type in the header file.
 *				This provides perfect Encapsulation in ANSI C.
 **/
struct T_I2C_BITBANG {
				T_GP_TIMER_INST	*mhTimer;		///< General Purpose Timer Handle.
				T_GPIO_MASK		mSCL;			///< GPIO Flag used for the I2C Clock line.
				T_GPIO_MASK		mSDA;			///< GPIO Flag used for the I2C Data line.
	volatile 	unsigned long	bTimerWait;		///< If timer is waiting.
	volatile	bool			bInUse;			///< If this I2C has been acquired for an operation.
				I2C_CONTROL		mControl;		///< I2C Control structure.
				T_I2C_MESSAGE	mMsg;			///< Current Message to be sent.
				I2C_CALLBACK	pCallback;		///< Callback Function and Parameter object to be called when using non-blocking I/O.
};


//---------- BUS CONTROL ROUTINES ----------//

/**
 *	@private
 *	@brief		Get the current state of the SDA line.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 *	@return		True (1) when SDA is set, else False (0).
 **/
static bool I2C_getSDA(T_I2CEMU_HANDLE hI2C) {
	gpio_becomeInput(hI2C->mSDA);
	if(gpio_readFlag(hI2C->mSDA)) {
		return true;
	}
	
	return false;
}


static bool I2C_getSCL(T_I2CEMU_HANDLE hI2C) {
	gpio_becomeInput(hI2C->mSCL);
	if(gpio_readFlag(hI2C->mSCL)) {
		return true;	
	}
	
	return false;
}

/**
 *	@private
 *	@brief		Sets the SDA line on the I2C BUS.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_setSDA(T_I2CEMU_HANDLE hI2C) {
	//gpio_becomeOutput(hI2C->mSDA);
	//gpio_set(hI2C->mSDA);
	// Both lines on the I2C bus are pulled high, so we don't need to drive the line to set it.
	gpio_becomeInput(hI2C->mSDA);	// Simply release the SDA line allowing it rise.
}


/**
 *	@private
 *	@brief		Clears the SDA line on the I2C BUS.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_clearSDA(T_I2CEMU_HANDLE hI2C) {
	gpio_becomeOutput(hI2C->mSDA);	// To clear SDA we must physically drive the line.
	gpio_clear(hI2C->mSDA);			// Set as output, and then drive the flag low.
}


/**
 *	@private
 *	@brief		Sets the SCL line on the I2C BUS.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_setSCL(T_I2CEMU_HANDLE hI2C) {
	//gpio_becomeOutput(hI2C->mSCL);
	//gpio_set(hI2C->mSCL);
	gpio_becomeInput(hI2C->mSCL);	// Simply release the SCL line, allowing it to rise.
}


/**
 *	@private
 *	@brief		Clears the SCL line on the I2C BUS.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_clearSCL(T_I2CEMU_HANDLE hI2C) {
	gpio_becomeOutput(hI2C->mSCL);	// Drive the clock line low.
	gpio_clear(hI2C->mSCL);	
}


//---------- STACK ROUTINES ----------//

/**
 *	@private
 *	@brief		Define the current state of the state machine.
 *
 *	@param		hI2C		Handle to the I2C Controller.
 *	@param		pa_State	STATE to put the state machine into.
 *	@param		pa_StateSeq	Sequence number to start the state in.
 *
 **/
static void I2C_defState(T_I2CEMU_HANDLE hI2C, I2C_STATE pa_State, unsigned long pa_StateSeq) {
	hI2C->mControl.mCurrentTask.mState = pa_State;
	hI2C->mControl.mCurrentTask.mnStateSeq	= pa_StateSeq;
}


/**
 *	@private
 *	@brief		Pushes the current context onto the stack.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2CEMU_push(T_I2CEMU_HANDLE hI2C) {
	hI2C->mControl.mTaskStack.mContexts[hI2C->mControl.mTaskStack.mnStackPtr] = hI2C->mControl.mCurrentTask;
	hI2C->mControl.mTaskStack.mnStackPtr++;			// Increase the stack pointer.
}


/**
 *	@private
 *	@brief		Pops the Context on the top of the stack into the Current Task.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2CEMU_pop(T_I2CEMU_HANDLE hI2C) {
	if(hI2C->mControl.mTaskStack.mnStackPtr > 0) {
		hI2C->mControl.mTaskStack.mnStackPtr--;		// Decrease the stack pointer.
		hI2C->mControl.mCurrentTask = hI2C->mControl.mTaskStack.mContexts[hI2C->mControl.mTaskStack.mnStackPtr];
	} else {
		// Stack error, there was nothing on the stack to be popped!
		// Place a FREE in the Current Task to be safe.
		I2C_defState(hI2C, I2C_STATE_FREE, 0);	
	}
}


//---------- CLOCK CONTROL ROUTINES ----------//

/**
 *	@private
 *	@brief		Makes the clock transition from Tick to Tock (0 to 1), or Tock to Tick. (1 to 0).
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2CEMU_clock(T_I2CEMU_HANDLE hI2C) {
	if(hI2C->mControl.mbClk) {		// Check current status of the clock. (Tick or Tock - 1 or 0)
		hI2C->mControl.mbClk = 0;	// Switch from Tick to Tock (mentally).
		I2C_clearSCL(hI2C);			// Physically switch from Tick to Tock.
	} else {
		hI2C->mControl.mbClk = 1;	// Switch from Tock to Tick (mentally).
		I2C_setSCL(hI2C);			// Physically switch from Tock to Tick.
	}	
}


/**
 *	@private
 *	@brief		Initialises the CLOCK state, for the specified number of ticks.
 *
 *	@param		hI2C		Handle to the I2C Controller.
 *	@param		pa_nTicks	The number of ticks to make the clock go Tick Tock!
 *
 **/
static void I2CEMU_initclock(T_I2CEMU_HANDLE hI2C, unsigned long pa_nTicks) {
	hI2C->mControl.mCurrentTask.mState 		= I2C_STATE_CLOCK;
	hI2C->mControl.mCurrentTask.mnStateSeq 	= 4*pa_nTicks;	// 1 sequence is a switch from Tick to Tock and vise-verca, plus 2 periods of level therefore 4*pa_nTicks.
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through a STOP sequence.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_stop(T_I2CEMU_HANDLE hI2C) {
	
	switch(hI2C->mControl.mCurrentTask.mnStateSeq) {
		case 0: {
			I2C_clearSDA(hI2C);		
		}
		break;
		
		case 1: {
			I2C_setSCL(hI2C);		
		}
		break;
		
		case 2: {
			I2C_setSDA(hI2C);
		}
		break;
		
		case 3: {
			I2CEMU_pop(hI2C);	// Pop the next task off the stack.	
			return;						
		}
		
	}
	
	hI2C->mControl.mCurrentTask.mnStateSeq++;
}




/**
 *	@private
 *	@brief		Micro-Operations to clock through a START sequence.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_start(T_I2CEMU_HANDLE hI2C) {
	switch(hI2C->mControl.mCurrentTask.mnStateSeq) {
		case 0: {	// SDA Goes High
			hI2C->mControl.mCurrentTask.mnStateSeq++;
			I2C_setSDA(hI2C);	// Set SDA High
		}
		break;
		
		case 1: {
			hI2C->mControl.mCurrentTask.mnStateSeq++;
			I2C_setSCL(hI2C);
		}
		break;
		
		case 2: {
			hI2C->mControl.mCurrentTask.mnStateSeq++;
			I2C_clearSDA(hI2C);
		}
		break;
		
		
		case 3: {
			hI2C->mControl.mCurrentTask.mnStateSeq++;
			I2C_clearSCL(hI2C);
			
			I2CEMU_pop(hI2C);	// Pop the next task off the stack.		
		}
		break;
		
		default:
			break;
						
	}
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through a TX_ACK sequence.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_TX_ACK(T_I2CEMU_HANDLE hI2C) {

	switch(hI2C->mControl.mCurrentTask.mnStateSeq) {
		
		case 0: {
			gpio_becomeInput(hI2C->mSDA);
			I2CEMU_clock(hI2C);				// Clock the rising edge.
		}
		break;
		
		
		case 1:								// Level off clock sequence.
		case 2:
		
		
		break;
		
		
		case 3: {
			if(!I2C_getSDA(hI2C)) {			// If Ackknowledge bit is cleared, ACK was received.
				hI2C->mControl.mCurrentTask.mnStateSeq = 4;	// Skip 2nd Sequence.		
			}
			gpio_becomeOutput(hI2C->mSDA);
			I2CEMU_clock(hI2C);				// Clock the falling edge.
		}
		break;
		
		case 4: {
			gpio_clear(hI2C->mSDA);
			hI2C->mMsg.mbAcked = 0;
			I2C_defState(hI2C, I2C_STATE_FREE, 0);	// Generate the stop signal.			
			// NON ACKKNOWLEDGE		
		}
		break;
		
		
		case 5: {
			
			// ACKKNOWLEDGED - Finish the work!
			gpio_clear(hI2C->mSDA);
			hI2C->mMsg.mbAcked = 1;
			
			I2C_defState(hI2C, I2C_STATE_FREE, 0);
			return;
		}
		
		default:
			I2C_defState(hI2C, I2C_STATE_FREE, 0);
			return;
		
	}
	
	hI2C->mControl.mCurrentTask.mnStateSeq++;
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through an RX_ACK sequence.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_RX_ACK(T_I2CEMU_HANDLE hI2C) {

	switch(hI2C->mControl.mCurrentTask.mnStateSeq) {
		case 0: {
			I2C_clearSDA(hI2C);
			hI2C->mControl.mCurrentTask.mnStateSeq++;
						
		}
		break;
		
		case 1: {						//Keeps the clock tick of an even length!
			I2CEMU_clock(hI2C);
			hI2C->mControl.mCurrentTask.mnStateSeq++;
			
		}
		break;
		
		
		case 2:
		case 3:
			hI2C->mControl.mCurrentTask.mnStateSeq++;
		break;
		
		case 4: {
			I2CEMU_clock(hI2C);
			hI2C->mControl.mCurrentTask.mnStateSeq++;
				
		}
		break;
		
		case 5: {
			I2C_setSDA(hI2C);			// Release SDA pin by setting it (becomeInput!)
			I2C_defState(hI2C, I2C_STATE_FREE, 0);	
		}
		break;
	}
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through an RX_NACK sequence.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_RX_NACK(T_I2CEMU_HANDLE hI2C) {

	switch(hI2C->mControl.mCurrentTask.mnStateSeq) {
		case 0: {
			I2C_setSDA(hI2C);
			hI2C->mControl.mCurrentTask.mnStateSeq++;
						
		}
		break;
		
		case 1: {
			I2CEMU_clock(hI2C);
			hI2C->mControl.mCurrentTask.mnStateSeq++;
			
		}
		break;
		
		
		case 2:							// Keeps the clock tick of an even length!
		case 3:							// (Keep the line high for 2 cycles of the scheduler.
			hI2C->mControl.mCurrentTask.mnStateSeq++;
		break;
		
		case 4: {
			I2CEMU_clock(hI2C);
			hI2C->mControl.mCurrentTask.mnStateSeq++;
				
		}
		break;
		
		case 5: {
			//I2C_setSDA(hI2C);			// Release SDA pin by setting it (becomeInput!)
			I2C_defState(hI2C, I2C_STATE_FREE, 0);	
		}
		break;
	}
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through a TX sequence.
 *
 *	Cycles through each bit of the current byte to be TX'd. Once each byte has been
 *	driven onto the BUS, it PUSHes the TX state onto the stack, and runs 1 Tick Tock
 *	of the clock. The clock goes tick tock, then POPs the stack, allowing this TX
 *	sequence to continue where it left off. I.e. transmit the next bit.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_SM_TX(T_I2CEMU_HANDLE hI2C) {
	
	if(hI2C->mControl.mCurrentTask.mnStateSeq == 8) {
		//	PERHAPS a timing Issue here -- Perhaps clock up, here and read ACK.
		I2C_defState(hI2C, I2C_STATE_TX_ACK, 0);				// Byte is transmitted, Acknowledge.	
		//	NEEDS TESTING. - Should work fine.
	} else {
		
		if((hI2C->mMsg.mcData & 0x80)) {	// Is the current bit high or low?
			I2C_setSDA(hI2C);				// SET flag.
		} else {
			I2C_clearSDA(hI2C);				// CLEAR flag.
		}
	
		hI2C->mMsg.mcData = (hI2C->mMsg.mcData << 1);	// Rotate to the next bit.
	
		hI2C->mControl.mCurrentTask.mnStateSeq++;		// Increase the state sequence number.
		
		I2CEMU_push(hI2C);			// Push state and clock.
		I2CEMU_initclock(hI2C, 1);	// Start the clock for 1 tick.
		
	}
	
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through an RX sequence.
 *
 *	Sets the clock line high, then waits until the next scheduling, where
 *	the clock line is kept high and level while the SDA line settles.
 *	On the 3rd scheduling of the RX state, the SDA line is read. Finally the
 *	sequence is completed by invoking the SCL_LOW state, and repeating until
 *	a complete byte has been clocked in.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_SM_RX(T_I2CEMU_HANDLE hI2C) {
	
	unsigned long SSeq = hI2C->mControl.mCurrentTask.mnStateSeq % 4;
	
	if(!hI2C->mControl.mCurrentTask.mnStateSeq) {
		gpio_becomeInput(hI2C->mSDA);		// Ensure that SDA line is an input here!	
	}
	
	
	if(hI2C->mControl.mCurrentTask.mnStateSeq == 32) {
		I2C_defState(hI2C, I2C_STATE_FREE, 0);				// Byte is RX'd, Acknowledge.	
	} else {
		
		
		switch(SSeq) {
			case 0:
				// Rising Edge
				hI2C->mControl.mCurrentTask.mnStateSeq++;
				//I2CEMU_push(hI2C);
				//I2C_defState(hI2C, CLOCK, 1);				// Tick to Rising Edge!
				I2C_setSCL(hI2C);
				return;
			
			
			
			case 1:
			
			break;
			
			case 2:
				// Read	-- 2 Ticks??
				
				hI2C->mMsg.mcData = (hI2C->mMsg.mcData << 1);	// Shift data in!
		
				if(I2C_getSDA(hI2C)) {
					hI2C->mMsg.mcData |= 0x01;	
				} else {
					hI2C->mMsg.mcData &= ~0x01;
				}
			
			break;
					
			
			
			case 3:
				// Falling Edge
				hI2C->mControl.mCurrentTask.mnStateSeq++;
				I2C_clearSCL(hI2C);
				I2CEMU_push(hI2C);
				I2C_defState(hI2C, I2C_STATE_SCL_LOW, 0);				// Keep clock even, hold low for 2 timers.
								
				return;
			
		}
		
		hI2C->mControl.mCurrentTask.mnStateSeq++;		// Increase the state sequence number.
	}
}



/**
 *	@private
 *	@brief		Resets the Stack Pointer (Clearing the stack).
 *
 *	This should occur on every free. This guarantees that on each invokation of
 *	of some work by the I2C state machine, the stack is always in a well defined
 *	state.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_resetStack(T_I2CEMU_HANDLE hI2C) {
	hI2C->mControl.mTaskStack.mnStackPtr = 0;		// Set stack pointer to 0.
}


/**
 *	@private
 *	@brief		Micro-Operations to clock through a CLOCK sequence.
 *
 *	Detects clock stretching on each clock cycle.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2CEMU_clockOps(T_I2CEMU_HANDLE hI2C) {

	unsigned long uOp = (hI2C->mControl.mCurrentTask.mnStateSeq % 4);
	
	switch(uOp) {	// Order of the cases is backwards because we clock through the sequence number from 4 to 0.
		
		case 0:		// Rising Edge Clock.
			gpio_becomeInput(hI2C->mSCL);
			asm("ssync;");
			if(gpio_readFlag(hI2C->mSCL)) {	// Check for clock stretching.
				// Rising edge occurred, no need to wait!
				I2CEMU_clock(hI2C);							// Physically drive the clock! (And mentally clock).
			} else {
				// GOTO WAIT STATE.
				//hI2C->mControl.mCurrentTask.mnStateSeq--;
				I2CEMU_push(hI2C);		// Push clock state.
				I2C_defState(hI2C, I2C_STATE_WAIT, 0);
				return;		
			}
		break;
		
		case 1:	I2CEMU_clock(hI2C); // Falling edge of the clock.
		
		break;
		
		case 2:// Level for 2 ticks of the timer.
		case 3:		
			
		break;	
		
	}
	
	hI2C->mControl.mCurrentTask.mnStateSeq--;
	
	if(!hI2C->mControl.mCurrentTask.mnStateSeq) {
		//gpio_becomeInput(hI2C->mSDA);
		//I2C_clearSDA(hI2C);	// Make sure we're not driving the SDA line when there's an acknowledge.	
	}
}

/**
 *	@private
 *	@brief		State Machine Scheduler
 *
 *	This scheduler is invoked by the timer interrupt. It clocks through
 *	each state, until work is completed, and the FREE state occurs.
 *	The FREE state disable the timer, and the scheduler can no longer
 *	be invoked by the processor.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2CEMU_timer_callback(T_I2CEMU_HANDLE hI2C) {
	/*
		Generate the Clock Signal.
	*/
	
	switch(hI2C->mControl.mCurrentTask.mState) {
		case I2C_STATE_CLOCK: {
			
			if(hI2C->mControl.mCurrentTask.mnStateSeq) {
				I2CEMU_clockOps(hI2C);	// Do the relevant 
			} else {
				//I2CEMU_clock(hI2C);
				//gpio_becomeInput(hI2C->mSDA);
				I2CEMU_pop(hI2C);	// Switch back into the previous state.
			}
		}
		
		break;
		
		case I2C_STATE_START: {
			I2C_start(hI2C);		// Call the I2C start Micro-operation for the current sequence number.
		}
		
		break;
		
		case I2C_STATE_STOP: {
			I2C_stop(hI2C);			// Call the I2C stop Micro-operation for the current sequence number.
		}
		
		break;
		
		case I2C_STATE_TX: {
			// Start transmiting data. - Transmit a byte of anything (defined in the message).
			I2C_SM_TX(hI2C);			
		}
		break;
		
		
		case I2C_STATE_RX: {	// RX a complete Byte of data from the line.
			I2C_SM_RX(hI2C);			
		}
		
		break;
		
		case I2C_STATE_WAIT: {	// Wait until SCL is not being held high! Then pop the current task off the stack.
			I2CEMU_pop(hI2C);
		}
		break;
		
		case I2C_STATE_TX_ACK: {	// ACK of Transmission
			I2C_TX_ACK(hI2C);
		}
		break;
		
		case I2C_STATE_RX_ACK: {	// ACK the Recieving!
			I2C_RX_ACK(hI2C);
		}
		break;
		
		case I2C_STATE_RX_NACK: {
			I2C_RX_NACK(hI2C);
		}
		break;
		
		case I2C_STATE_SCL_LOW: {	// Hold SCL low for 2 timer ticks. (To keep clock even).
			switch(hI2C->mControl.mCurrentTask.mnStateSeq) {
				case 0: {
					
				}
				break;
				
				case 1: {
					I2CEMU_pop(hI2C);
					return;
				}
				
			}
			hI2C->mControl.mCurrentTask.mnStateSeq++;
		}
		break;
		
		
		case I2C_STATE_FREE: {
			I2C_resetStack(hI2C);			// Reset the task stack. So when the state machine begins again, it cannot be corrupted.
			timer_disable(hI2C->mhTimer);
			hI2C->bTimerWait = 0;			// Signal to Pend that we have completed the work unit.			
		}
		
		break;
		
		default:
			I2C_defState(hI2C, I2C_STATE_FREE, 0);	// Something went wrong! Best just clock through to a FREE.
			break;
	}
}


/**
 *	@private
 *	@brief		Starts the Timer, and thus starts the State Machine.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static T_ERROR_CODE I2C_StartSM(T_I2CEMU_HANDLE hI2C) {
	
	/*if(!I2C_getSCL(hI2C) && !I2C_getSDA(hI2C)) {
		hI2C->bTimerWait = 0;
		return -1;	// No Physical bus detected!	
	}*/
	
	hI2C->bTimerWait = 1;
	timer_enable(hI2C->mhTimer);
	
	return 0;
}


/**
 *	@private
 *	@brief		Pend on the work of the State Machine to finish.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_PendSM(T_I2CEMU_HANDLE hI2C) {
	// Setup and wait on the timer.
		
	while(hI2C->bTimerWait) {
		#ifdef _USE_VDK_
		//VDK_Yield();
		#endif	
	}
}


/**
 *	@private
 *	@brief		Send ACK
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_sendAck(T_I2CEMU_HANDLE hI2C) {
	I2C_defState(hI2C, I2C_STATE_RX_ACK, 0);
	I2C_StartSM(hI2C);
	I2C_PendSM(hI2C);
}


/**
 *	@private
 *	@brief		Send NACK
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_sendNack(T_I2CEMU_HANDLE hI2C) {
	I2C_defState(hI2C, I2C_STATE_RX_NACK, 0);
	I2C_StartSM(hI2C);
	I2C_PendSM(hI2C);
}


/**
 *	@private
 *	@brief		Gets Acknowledgement status of the last TX'd byte.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static bool I2C_getAck(T_I2CEMU_HANDLE hI2C) {
	return hI2C->mMsg.mbAcked;	// Return ack of the latest ACK.
}


/**
 *	@private
 *	@brief		Acquires the I2C Handle, blocking other threads from using it for I/O.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_acquire(T_I2CEMU_HANDLE hI2C) {	/*	This essentially creates a Semaphore */
	
	void *pCritArg = adi_int_EnterCriticalRegion(NULL);
		
	while(hI2C->bInUse) {	// Wait until the I2C is not in use.
		adi_int_ExitCriticalRegion(pCritArg);
		#ifdef _USE_VDK_
			//VDK_Yield();
		#endif
		pCritArg = adi_int_EnterCriticalRegion(pCritArg);
	}
	
	hI2C->bInUse = 1;		// Set in use to 1.	
	
	adi_int_ExitCriticalRegion(pCritArg);
	
}


/**
 *	@private
 *	@brief		Release the I2C Handle, allowing work from other threads.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_release(T_I2CEMU_HANDLE hI2C) {
	hI2C->bInUse = 0;	// Release the Acquired Handle.	
}


/**
 *	@private
 *	@brief		Send Start
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_sendStart(T_I2CEMU_HANDLE hI2C) {
	I2C_defState(hI2C, I2C_STATE_FREE, 0);	// Push the FREE state onto the state machine's stack
	I2CEMU_push(hI2C);				// so that after the STOP state, the state machine POP's
	I2C_defState(hI2C, I2C_STATE_START, 0);	// into the FREE state, and thus finishes the current sequence.
	I2C_StartSM(hI2C);				
	I2C_PendSM(hI2C);				// Pend on the requested work to be completed.
}


/**
 *	@private
 *	@brief		Send Stop
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static void I2C_sendStop(T_I2CEMU_HANDLE hI2C) {
	I2C_defState(hI2C, I2C_STATE_FREE, 0);	// Push the FREE state onto the state machine's stack
	I2CEMU_push(hI2C);				// so that after the STOP state, the state machine POP's
	I2C_defState(hI2C, I2C_STATE_STOP, 0);	// into the FREE state, and thus finishes the current sequence.
	I2C_StartSM(hI2C);
	I2C_PendSM(hI2C);				// Pend on the requested work to be completed.
}


/**
 *	@private
 *	@brief		Initialise the state machine to TX the specified data.
 *
 *	This is a low-level transmission, and doesn't handle Starts or Stops.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static bool I2C_tx(T_I2CEMU_HANDLE hI2C, unsigned char data) {
	
	bool Ack;
	
	hI2C->mMsg.mcData = data;	// Set the working message to the provided data.
	
	I2C_defState(hI2C, I2C_STATE_TX, 0);	// Set the state machine to begin in the TX state.
								// 0 is the state sequence number, in the TX state this is bit number being TX'd.
	
	// Start the work!
	I2C_StartSM(hI2C);			// Start the state machine.
	I2C_PendSM(hI2C);			// Pend on the works completion.
		
	Ack = I2C_getAck(hI2C);		// Get acknowledgement of transmission.
		
	return Ack;					// Return the status of the acknowledgement.
}


/**
 *	@private
 *	@brief		Initialise the state machine to RX a byte of Data.
 *
 *	This is a low-level RX, and doesn't handle Starts or Stopsor even ACKS.
 *
 *	@param		hI2C	Handle to the I2C Controller.
 *
 **/
static unsigned char I2C_rx(T_I2CEMU_HANDLE hI2C) {
	I2C_defState(hI2C, I2C_STATE_RX, 0);
	I2C_StartSM(hI2C);
	I2C_PendSM(hI2C);
	return hI2C->mMsg.mcData;	// Return the received data.
}


/**
 *	@brief		High-Level I2C Register Write function.
 *
 *	Provides a complete I2C Write sequence, from start to stop.
 *
 *	@param		hI2C		Handle to the I2C Controller.
 *	@param		pa_DevAddr	I2C Slave address. (7-bit addresses only).
 *	@param		pa_DevReg	Register address on the Slave to be written to.
 *	@param		pa_cData	A byte of data to be written to the register on the slave.
 *
 *	@return		ERR_I2C_NACK if the SLAVE didn't ACK the transmission. Else the number of bytes actually written.
 **/
int I2CEmuWrite(T_I2CEMU_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb) {
	
	unsigned char txData = pa_DevAddr;
	int i;
	
	if(pa_pNb) {
		return ERR_I2CEMU_NON_BLOCKING_NOT_SUPPORTED;	
	}
	
	I2C_acquire(hI2C);
	{
		I2C_sendStart(hI2C);
		
		txData = txData << 1; 	// Push the Address up to the top of the first byte.
		txData &= ~0x01;		// Set the write bit.(Clear it!)
		
		if(!I2C_tx(hI2C, txData)) {
			I2C_sendStop(hI2C);
			I2C_release(hI2C);
			return ERR_I2CEMU_NACK;
		}
		
		txData = pa_DevReg;
		
		if(!I2C_tx(hI2C, txData)) {
			I2C_sendStop(hI2C);
			I2C_release(hI2C);
			return ERR_I2CEMU_NACK;
		}
		
		
		for(i = 0; i < size; i++) {
			
			txData = *(buffer++);
			
			if(!I2C_tx(hI2C, txData)) {
				I2C_sendStop(hI2C);
				I2C_release(hI2C);
				return ERR_I2CEMU_NACK;
			}
		}
		
		I2C_sendStop(hI2C);
		
	}
	I2C_release(hI2C);
	
	return i;
}



/**
 *	@brief		High-Level I2C Register Read function.
 *
 *	Provides a complete I2C Read sequence, from start to stop.
 *
 *	@param		hI2C		Handle to the I2C Controller.
 *	@param		pa_DevAddr	I2C Slave address. (7-bit addresses only).
 *	@param		pa_DevReg	Register address on the Slave to be written to.
 *	@param		buffer		Buffer to fill.
 *	@param		size		Number of bytes to read.
 *
 *	@return		ERR_I2C_NACK if the Slave didn't ACK any part of the Read Sequence..
 *	@return		Success is always a positive number, errors are negative.
 *	@return		On Success, the value is the number of bytes actually read.
 **/
int I2CEmuRead(T_I2CEMU_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb) {
	
	unsigned char txData = pa_DevAddr;
	unsigned char rxData = 0x00;
	int i = -1;
	
	if(pa_pNb) {
		return ERR_I2CEMU_NON_BLOCKING_NOT_SUPPORTED;	
	}
	
	I2C_acquire(hI2C);
	{
		I2C_sendStart(hI2C);
		
		txData = txData << 1; 	// Push the Address up to the top of the first byte.
		txData &= ~0x01;		// Set the write bit.(Clear it!)
		
		if(!I2C_tx(hI2C, txData)) {
			I2C_sendStop(hI2C);
			I2C_release(hI2C);
			return ERR_I2CEMU_NACK;
		}
		
		txData = pa_DevReg;
		
		if(!I2C_tx(hI2C, txData)) {
			I2C_sendStop(hI2C);
			I2C_release(hI2C);
			return ERR_I2CEMU_NACK;
		}
		
		txData = pa_DevAddr;
		
		txData = txData << 1; 		// Push the Address up to the top of the first byte.
		txData |= 0x01;				// Set the read bit.(Set it!)
		
		I2C_sendStart(hI2C);
		
		if(!I2C_tx(hI2C, txData)) {
			I2C_sendStop(hI2C);
			I2C_release(hI2C);
			return ERR_I2CEMU_NACK;	
		}
		
		// RX the data
		for(i = 0; i < size - 1; i++) {
			*(buffer++) = I2C_rx(hI2C);
			I2C_sendAck(hI2C);
		}
		
		*(buffer++) = I2C_rx(hI2C);
		
		I2C_sendNack(hI2C);			// Last byte so we NACK!
		
		I2C_sendStop(hI2C);			// Send the STOP signal on the line!
		
	}
	I2C_release(hI2C);
	
	return i;	// Return the bytes read.
	
}
	





T_ERROR_CODE I2CEmuControl( T_I2CEMU_HANDLE hI2C, unsigned char hwid, I2CConfig *config, short len ) {
    short i;
    T_ERROR_CODE erResult = ERR_NONE;
    for( i = 0; i < len; i++ ) {
        
    	if((erResult = I2CEmuWrite(hI2C, (unsigned short) hwid, config[i].address, &config[i].value, 1, NULL))) {
    		i = len;	
    	}

    }
    return erResult;
}

T_ERROR_CODE I2CEmuStatus( T_I2CEMU_HANDLE hI2C, unsigned char hwid, I2CConfig *config, short len ) {
    short i;
    unsigned char Val;
    T_ERROR_CODE erResult = ERR_NONE;
    for( i = 0; i < len; i++ ) {
        
    	erResult = I2CEmuRead(hI2C, (unsigned short) hwid, config[i].address, &Val, 1, NULL);
    	
    	if(!erResult) {
    		config[i].value = Val;
    	}
    }
    return erResult;
}


#define I2CEMU_TIMER_CONFIG 	EMU_RUN | OUT_DIS | IRQ_ENA | PERIOD_CNT | PWM_OUT		///< value for TIMERX configuration register 

T_I2CEMU_HANDLE I2CEmuOpen(unsigned char pa_cI2CEMUnr, T_GPIO_MASK pa_SCL, T_GPIO_MASK pa_SDA, 
							unsigned char pa_cTimernr, unsigned long pa_nBitRate, unsigned long pa_nSystemClk, T_ERROR_CODE *pError) {
	
	T_I2CEMU_HANDLE hI2C = (T_I2CEMU_HANDLE) malloc(sizeof(struct T_I2C_BITBANG));
	unsigned long nTimerPeriod;
	
	if(hI2C) {
		hI2C->mSCL = pa_SCL;
		hI2C->mSDA = pa_SDA;
		hI2C->bInUse = 0;
		hI2C->mControl.mTaskStack.mnStackPtr = 0;
		hI2C->mControl.mbClk = 0;
		
		gpio_becomeInput(pa_SCL);
		gpio_becomeInput(pa_SDA);
		
		if(!I2C_getSCL(hI2C) || !I2C_getSDA(hI2C)) {
			free(hI2C);
			if(pError) {
				*pError = ERR_I2CEMU_BUS_NOT_DETECTED;	
			}
			return (T_I2CEMU_HANDLE) NULL;			
		}
		
		nTimerPeriod = (unsigned long)	(pa_nSystemClk / (pa_nBitRate * 8));	// 4 Timer Micro-ticks per clock.
		
		// timer setup
		T_GP_TIMER_SETUP_PARAM tTimerSetup;
		tTimerSetup.nTimerNr = pa_cTimernr;
		tTimerSetup.nConfig = I2CEMU_TIMER_CONFIG;
		tTimerSetup.nWidth = nTimerPeriod / 2;
		tTimerSetup.nPeriod = nTimerPeriod;
		tTimerSetup.nIVG = 0;
		tTimerSetup.cTMRCLKinputPin = 0;
		tTimerSetup.bPositivePulse = true;
		tTimerSetup.nSystemClk = pa_nSystemClk;
		tTimerSetup.erResult = ERR_NONE;
		tTimerSetup.fnCallback = (T_GP_TIMER_CALLBACK) I2CEMU_timer_callback;
		tTimerSetup.pCallbackArg = hI2C;
		
		hI2C->mhTimer = timer_gp_setup(&tTimerSetup);			
		
	}
	
	return hI2C;
	
}

T_ERROR_CODE I2CEmuClose(T_I2CEMU_HANDLE hI2C) {
	
	if(timer_close(hI2C->mhTimer) != ERR_NONE) {	// Safely close the timer instance.
		return ERR_GENERIC;
	}

	free(hI2C);	// Free the Handle.

	return ERR_NONE;
}

/*
	I2C Manager Open interface wrapper.
*/
static T_I2CEMU_HANDLE I2CIFopen(char pa_cI2C, T_I2CEMU_CONFIG *pCfg, T_ERROR_CODE *pError, void *pReserved) {
	return I2CEmuOpen(pa_cI2C, pCfg->mtSCL, pCfg->mtSDA, pCfg->mcTimer, pCfg->mnBitRate, pCfg->mnSystemClk, pError);
}

static void I2CEmuScanBus(T_I2CEMU_HANDLE hI2C, I2C_FN_SCAN_CALLBACK callback) {
	unsigned short i;
	unsigned char	x = 0;
	for(i = 1; i < 128; i++) {
		if(!I2CEmuRead(hI2C, i, 0x0000, &x, 1, NULL)) {
			// Callthe callback!
			if(callback) {
				callback(i);
			}
		}
	}
}


static const I2C_FN_POINTERS g_I2Capi = {
	(I2C_FN_OPEN) 		I2CIFopen,
	(I2C_FN_CLOSE) 		I2CEmuClose,
	(I2C_FN_READ_REG)	I2CEmuRead,
	(I2C_FN_WRITE_REG)	I2CEmuWrite,
	(I2C_FN_SCANBUS)	I2CEmuScanBus,
};

const I2C_FN_POINTERS *I2CEmu_getFunctions(void) {
	return &g_I2Capi;
}


