/**
	@file i2c_global.c
	@brief Processor specific implementation of I2C for BF527 
	
	@author Thomas Maier
**/

#include "../common/I2C/I2Cconfig.h"

#include <cdefbf527.h>
#include <services/services.h>

/*
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

*/

T_I2C_SPEC g_I2Cspec[] = {
	{
		ADI_INT_TWI,		
		pTWI_CLKDIV,
 		pTWI_CONTROL,
 		pTWI_SLAVE_CTL,		
 		pTWI_SLAVE_STAT,	
 		pTWI_SLAVE_ADDR,	
		pTWI_MASTER_CTL,		
		pTWI_MASTER_STAT,
 		pTWI_MASTER_ADDR,	
 		pTWI_INT_STAT,		
 		pTWI_INT_MASK,		
 		pTWI_FIFO_CTL,	
 		pTWI_FIFO_STAT,	
 		pTWI_XMT_DATA8,		
 		pTWI_XMT_DATA16,	
 		pTWI_RCV_DATA8,	
 		pTWI_RCV_DATA16,
        false
	},

};

unsigned int g_nI2Ccount = sizeof (g_I2Cspec) / sizeof (T_I2C_SPEC);

bool I2CplatformInit(int pa_nI2Cnumber) {
	bool bResult = true;
	
	switch (pa_nI2Cnumber) {
		case 0: {
		
			//nothing todo TWI pins are decicated
			break;
		}
	
	
		default: bResult = false;
	}
	return bResult;
}

