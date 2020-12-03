/**
 *	@file 		I2C_bitbang.h
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

#ifndef _I2C_BITBANG_H_
#define _I2C_BITBANG_H_

//#include "I2CConfig.h"		///< I2C Physical definitions (share the I2CConfig and I2C Callback definitions).

#include "I2CMan.h"
#include "../GPIOconfig.h"

//---------- ENCAPSULATED HANDLE
struct			T_I2C_BITBANG;
typedef struct	T_I2C_BITBANG	*T_I2CEMU_HANDLE;


typedef struct {
	unsigned long	mnBitRate;
	unsigned long	mnSystemClk;
	T_GPIO_MASK 	mtSCL;
	T_GPIO_MASK		mtSDA;
	char			mcTimer;
} T_I2CEMU_CONFIG;

#define ERR_I2CEMU_NACK							-20;	///< NON-ACKNOWLEDGMENT of TRANSMISSION by SLAVE.
#define ERR_I2CEMU_BUS_NOT_DETECTED 			-21;
#define ERR_I2CEMU_NON_BLOCKING_NOT_SUPPORTED 	-22;

//---------- PROTOTYPES

T_I2CEMU_HANDLE I2CEmuOpen		(unsigned char pa_cI2CEMUnr, T_GPIO_MASK pa_SCL, T_GPIO_MASK pa_SDA, 
									unsigned char pa_cTimernr, unsigned long pa_nBitRate, unsigned long pa_nSystemClk, T_ERROR_CODE *pError);
T_ERROR_CODE	I2CEmuClose		(T_I2CEMU_HANDLE hI2C);
T_ERROR_CODE	I2CEmuStatus	(T_I2CEMU_HANDLE hI2C, unsigned char hwid, I2CConfig *config, short len);
T_ERROR_CODE	I2CEmuControl	(T_I2CEMU_HANDLE hI2C, unsigned char hwid, I2CConfig *config, short len);
int 			I2CEmuRead		(T_I2CEMU_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb);
int			 	I2CEmuWrite		(T_I2CEMU_HANDLE hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *buffer, int size, I2C_CALLBACK *pa_pNb);
const I2C_FN_POINTERS *I2CEmu_getFunctions(void);



#endif

