/**
 *	@file 		I2Cconfig.h
 *	@ingroup 	I2C
 *	
 *	@brief 		I2C Controller Module
 *	
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Thomas Maier, Zlatan Stanojevic, Roland Oberhammer
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
 *  @ingroup driverapi
 *  I2C Interface
 */
 
#ifndef _I2C_CONFIG_H_
#define _I2C_CONFIG_H_

#include <datatypes.h>
#include <services/services.h>

//#include "I2CConfig2.h"

typedef void (*I2CMessageHandler) ( unsigned char , unsigned char);
typedef void (*I2CScanCallback)( unsigned char );

#define I2C_BLOCKING
#define I2C_RW_TIMEOUT						2000	// [ms]
#define MAX_NR_OF_I2C_INTERFACES			10

typedef signed long T_I2C_PHYS_HANDLE;

// error codes
#define ERR_I2C_IN_USE						(ERR_GENERIC - 10)
#define ERR_I2C_NOT_ENOUGH_MEM				(ERR_GENERIC - 20)
#define ERR_ILLEGAL_I2C_NUMBER				(ERR_GENERIC - 30)
#define ERR_I2C_HOOK_INTERRUPT				(ERR_GENERIC - 40)
#define ERR_I2C_TIMEOUT						(ERR_GENERIC - 50)
#define ERR_I2C_INVALID_HANDLE              (ERR_GENERIC - 55)


typedef struct {
	unsigned long mnBitRate;
	unsigned long mnSystemClk;
} T_I2C_INTERNAL_CONFIG;


typedef enum
{
	I2C_UNCONFIGURED,
	I2C_IDLE,
	I2C_SCANBUS,
	I2C_READ,
	I2C_WRITE,
	I2C_GET,
	I2C_SET,
	I2C_EXT_GET0,
	I2C_EXT_GET1,
} E_I2C_STATE;

typedef struct {
	ADI_INT_PERIPHERAL_ID tPeripheralID;
	volatile unsigned short *clkdiv;	
	volatile unsigned short *control;
	volatile unsigned short *slave_ctl;
	volatile unsigned short *slave_stat;
	volatile unsigned short *slave_addr;	
	volatile unsigned short *master_ctl;
	volatile unsigned short *master_stat;
	volatile unsigned short *master_addr;
	volatile unsigned short *int_stat;
	volatile unsigned short *int_mask;
	volatile unsigned short *fifo_ctl;
	volatile unsigned short *fifo_stat;
	volatile unsigned short *xmt_data8;
	volatile unsigned short *xmt_data16;
	volatile unsigned short *rcv_data8;
	volatile unsigned short *rcv_data16;
	bool bInUse;

} T_I2C_SPEC;

typedef struct {
	
	T_I2C_SPEC *pSpec;
	unsigned char cI2Cnr;
	volatile E_I2C_STATE state;
	I2CMessageHandler all_handler;
	I2CScanCallback scan_callback;
	unsigned long nIvg;
	unsigned char cFastFlag;
	unsigned char *pMsg_start;
	unsigned char *pMsg_buffer;
	unsigned char cMsg_size;
	unsigned char cXmt_counter ;
	unsigned char cRcv_counter ;

} T_I2C_INST;

typedef struct {
	bool bInUse;
	T_I2C_PHYS_HANDLE tHndl;
} T_I2C_TABLE;

typedef struct
{
    unsigned char address, value;
} I2CConfig;



#define INFINITE -1  ///< 

// prototype declarations
T_I2C_PHYS_HANDLE 	I2Copen_phys	( unsigned char pa_nI2Cnr, unsigned long pa_nBitRate, unsigned long pa_nSystemClk );
T_ERROR_CODE 		I2Cclose_phys	(T_I2C_PHYS_HANDLE pa_tHndl);
void 				I2CScanbus_phys	( T_I2C_PHYS_HANDLE pa_tHndl, I2CScanCallback callback );
T_ERROR_CODE 		I2CControl_phys	( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char hwid, I2CConfig *config, short len );
T_ERROR_CODE 		I2CStatus_phys	( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char hwid, I2CConfig *config, short len );
//T_I2C_PHYS_HANDLE I2CgetGlobalHandle(unsigned char pa_cI2Cnr);
//void I2CsetGlobalHandle(unsigned char pa_cI2Cnr, T_I2C_PHYS_HANDLE pa_tHndl);
bool I2CplatformInit(int pa_nI2Cnumber);

void _I2CGet_non_blocking( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char p_cHwid , unsigned char p_cStartAddr, 
		   unsigned char *pvalues, unsigned char p_cValsLength,I2CMessageHandler callback);
T_ERROR_CODE _I2CSet_blocking( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char pa_cHwid, unsigned char start_addr, 
		   unsigned char *values, unsigned char vals_length );
T_ERROR_CODE _I2CGet_blocking( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char pa_cHwid, unsigned char start_addr, 
		   unsigned char *values, unsigned char vals_length );		   		   
T_ERROR_CODE _I2CRead_blocking( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char pa_cHwid, unsigned char *message,
		    unsigned char msg_length );
T_ERROR_CODE _I2CWrite_blocking( T_I2C_PHYS_HANDLE pa_tHndl, unsigned char pa_cHwid, unsigned char *message,
			unsigned char msg_length );		    
		    
		    		   
#ifdef I2C_BLOCKING
#define I2CRead  _I2CRead_blocking
#define I2CWrite _I2CWrite_blocking
#define I2CSet   _I2CSet_blocking
#define I2CGet   _I2CGet_blocking
#else
#define I2CRead  _I2CRead_non_blocking
#define I2CWrite _I2CWrite_non_blocking
#define I2CSet   _I2CSet_non_blocking
#define I2CGet   _I2CGet_non_blocking
#endif

#endif
