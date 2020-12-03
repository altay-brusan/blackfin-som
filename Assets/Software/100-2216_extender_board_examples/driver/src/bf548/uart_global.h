/** *********************************************************************
 * @file uart_global.h
 * @author Daniel Weber, Roland Oberhammer
 * @date 2005-10-23
 * @version 1.0
 * @brief Bitpattern definitions for the SPI registers.
 *
 * @see uart_global.c, UARTvdkConfig.c, UARTvdkConfig.h
  ********************************************************************* **/
#ifndef _UART_GLOBAL_H_INCLUDED_
#define _UART_GLOBAL_H_INCLUDED_

/**
@brief Bitpattern definitions for the Line Control Register (UART_LCR)
**/
#define WLS_0				(0x0001 << 00)	///Bit 0 of Word length select
#define WLS_1				(0x0001 << 01)	///Bit 1 of Word length select
#define STOP_BIT			(0x0001 << 02)	///Number of stop bits
#define PARITY_ENABLE		(0x0001 << 03)	///Enable or disable parity
#define EVEN_PARITY_SELECT	(0x0001 << 04)	///Select even or odd parity
#define STICK_PARITY		(0x0001 << 05)	///Select stick parity
#define SET_BREAK			(0x0001 << 06)	///Forces TX pin to 0
#define DLAB				(0x0001 << 07)	///Divisor latch access: Enables access to UART_DLL and UART_DLH

/**
@brief Bitpattern definitions for the Modem Control Register (UART_MCR)
**/
#define LOOP				(0x0001 << 04)	///Loopback mode enable

/**
@brief Bitpattern definitions for the Line Status Register (UART_LSR)
**/
#define DATA_READY			(0x0001 << 00)	///New data is ready
#define OVERRUN_ERR			(0x0001 << 01)	///Overrun error
#define PARITY_ERR			(0x0001 << 02)	///Parity error
#define FRAMEING_ERR		(0x0001 << 03)	///Framing error
#define BREAK_INT			(0x0001 << 04)	///Break interrupt
#define THR_EMPTY			(0x0001 << 05)	///THR empty
#define T_EMPTY				(0x0001 << 06)	///TSR and UART_THR empty

/**
@brief Bitpattern definitions for the Interrupt Enable Register (UART_IER)
**/
#define ERBFI				(0x0001 << 00)	///Enable receive buffer full interrupt
#define ETBEI				(0x0001 << 01)	///Enable transmit buffer empty interrupt
#define ELSI				(0x0001 << 02)	///Enable RX sttaus interrupt

/**
@brief Bitpattern definitions for the Interrupt Identification Register (UART_IIR)
**/
#define NINT				(0x0001 << 00)	///1: No interrupt pending, 0: interrupt is pending
#define STATUS_0			(0x0001 << 01)	///Uart status bit 0
#define STATUS_1			(0x0001 << 02)	///Uart status bit 1

/**
@brief Bitpattern definitions for the Bitmasks for status bit pattern in (UART_IIR)
**/
#define	UART_THR_EMPTY		(0x0001 << 01)	///UART transmitt buffer empty
#define	UART_RBR_FULL		(0x0002 << 01)	///UART receive buffer full (data ready)
#define UART_LINE_STATUS	(0x0003 << 01)	///Some line status errors

/**
@brief Bitpattern definitions for the Global Control Register (UART_GCTL)
**/
#define CLOCK_ENABLE		(0x0001 << 00)	///Enable UART clocks
#define IRDA_ENABLE			(0x0001 << 01)	///Enable IrDA mode
#define TPOLC				(0x0001 << 02)	///IrDA TX polarity change
#define RPOLC				(0x0001 << 03)	///IrDA RX polarity change

#endif	//_UART_GLOBAL_H_INCLUDED_

