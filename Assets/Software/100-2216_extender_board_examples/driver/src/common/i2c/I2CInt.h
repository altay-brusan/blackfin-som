/**
 *	@file 		I2CInt.h
 *	@ingroup 	I2C
 *	
 *	@brief 		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Thomas Maier
 *	@cond svn
 *	
 *	Information of last commit
 *	$Rev::               $:  Revision of last commit
 *	$Author::            $:  Author of last commit
 *	$Date::              $:  Date of last commit
 *	
 *	@endcond
 **/

#ifndef _I2C_INT_H_
#define _I2C_INT_H_



#include "I2CMan.h"


#define I2C_INT_RW_TIMEOUT          2000        ///< timeout for read/write in ms


//---------- ENCAPSULATED HANDLE
struct			T_I2C_INT;
typedef struct	T_I2C_INT	*T_I2C_INT_HANDLE;


typedef struct {
    unsigned long mnBitRate;
    unsigned long mnSystemClk;
    unsigned long nIvg;
} T_I2C_INT_CONFIG;


//---------- PROTOTYPES
int I2CIntWriteReg(T_I2C_INT_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb);
int I2CIntReadReg(T_I2C_INT_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb);
T_ERROR_CODE I2CIntControl( T_I2C_INT_HANDLE hI2C, unsigned char hwid, I2CConfig *config, short len );
T_ERROR_CODE I2CIntStatus( T_I2C_INT_HANDLE hI2C, unsigned char hwid, I2CConfig *config, short len );
T_I2C_INT_HANDLE I2CIntOpen(unsigned char pa_cI2CIntnr, unsigned long pa_nBitRate, unsigned long pa_nSystemClk, T_ERROR_CODE *pError, void *pa_pReserved);
T_ERROR_CODE I2CIntClose(T_I2C_INT_HANDLE hI2C);
const I2C_FN_POINTERS *I2CInt_getFunctions(void);


#endif

