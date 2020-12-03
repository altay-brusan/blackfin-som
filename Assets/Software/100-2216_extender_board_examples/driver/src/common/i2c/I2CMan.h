/**
 *	@file 		I2CMan.c
 *	@ingroup 	I2C
 *	
 *	@brief 		I2C Driver Interface
 *	
 *	Creates a common interface to all I2C devices, by defining an I2C Driver Model.
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


#ifndef _I2CMAN_H_
#define _I2CMAN_H_

#include <Datatypes.h>

#include "I2CConfig.h"

#define MAX_NR_OF_I2C_INTERFACES			10

struct 			I2C_BUS;				// Incomplete Data-type to provide complete encapsulation in C
typedef struct 	I2C_BUS *T_I2C_HANDLE;	// T_I2C_HANDLE is a I2C_BUS object pointer.

typedef void*	T_I2C_CONFIG;			// T_I2C_CONFIG is just a pointer to anything.

/**
 *	@brief Definition of a SCANBUS Callback function for an I2C Driver.
 **/
typedef void	(*I2C_FN_SCAN_CALLBACK)	(unsigned short);

/**
 *	@brief	Definition of Non-blocking I/O callback function. (If driver implements such behaviour).
 **/
typedef void (*I2C_FN_NONBLOCKING)		(T_I2C_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, void *pParam);


/**
 *	@brief	I2C_CALLBACK Object - Configuration for NON-Blocking I/O.
 **/
typedef struct {
	I2C_FN_NONBLOCKING 	mpfnCallback;	///< Pointer to callback function to handle the non-blocking I/O mode. 
	void 				*mpParam;		///< A parameter provided to the non-blocking callback function.
} I2C_CALLBACK;

/**
 *	@brief	Definition of an I2C Driver OPEN function.
 *
 *	Must return NULL on failure, and can set the provided pError code.
 *	Must not set pError if the pError pointer is NULL.
 *	
 *	@param	pa_cI2C		An arbitrary number used to distinguish different I2C controllers.
 *	@param	pa_pCfg		A void* pointer to the drivers configuration object.
 *	@param	pError		A pointer to a T_ERROR_CODE for returning error information. This can be NULL!
 *
 **/
typedef void * 			(*I2C_FN_OPEN)		(char pa_cI2C, T_I2C_CONFIG pa_pCfg, T_ERROR_CODE *pError, void *pReserved);

/**
 *	@brief Definition of an I2C Driver CLOSE function.
 **/
typedef T_ERROR_CODE	(*I2C_FN_CLOSE)		(void *pParam);

/**
 *	@brief Definition of an I2C Driver READ REGISTER function.
 **/
typedef int				(*I2C_FN_READ_REG)	(void *pParam, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb);

/**
 *	@brief Definition of an I2C Driver WRITE REGISTER function.
 **/
typedef int				(*I2C_FN_WRITE_REG)	(void *pParam, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb);

/**
 *	@brief Definition of an I2C Driver ScanBUS function.
 **/
typedef void			(*I2C_FN_SCANBUS)	(void *pParam, I2C_FN_SCAN_CALLBACK);


typedef struct {
	I2C_FN_OPEN			fnOpen;				///< I2C Device Open Function
	I2C_FN_CLOSE		fnClose;			///< I2C Device Close Function
	I2C_FN_READ_REG		fnRead;				///< I2C Device Read Register Function
	I2C_FN_WRITE_REG	fnWrite;			///< I2C Device Write Register Function
	I2C_FN_SCANBUS		fnScanBus;			///< I2C ScanBus Function.
} I2C_FN_POINTERS;

// Error Codes

#define I2C_ERR_NOT_ENOUGH_MEMORY		-4	
#define I2C_ERR_INVALID_INTERFACE		-5
#define I2C_ERR_CONFIG_NOT_PROVIDED		-6
#define I2C_ERR_INTERFACE_OPEN_FAILED	-7
#define ERR_I2C_NOT_SUPPORTED			-8
#define ERR_I2C_NULL_POINTER			-9



//---------- API PROTOTYPES

T_ERROR_CODE 	I2CAddInterface		(const I2C_FN_POINTERS *pa_pFunctions);
T_ERROR_CODE 	I2CRemoveInterface	(int pa_nI2C);
T_I2C_HANDLE 	I2Copen				(char pa_nI2C, T_I2C_CONFIG pa_pCfg, T_ERROR_CODE *pError, void *pReserved);
int 			I2CreadReg			(T_I2C_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, unsigned long size, I2C_CALLBACK *pa_pNb);
int 			I2CwriteReg			(T_I2C_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, unsigned long size, I2C_CALLBACK *pa_pNb);
T_ERROR_CODE 	I2CScanBus			(T_I2C_HANDLE hI2C, I2C_FN_SCAN_CALLBACK pa_pfnCallback);

//---------- HANDLE MANAGEMENT API
int 			I2CgetTotalHandles	();
T_I2C_HANDLE 	I2CgetGlobalHandle	(int pa_cNr);
void 			I2CsetGlobalHandle	(int pa_cNr, T_I2C_HANDLE hI2C);

#endif
