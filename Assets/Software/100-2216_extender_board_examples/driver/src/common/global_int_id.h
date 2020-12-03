#ifndef _GLOBAL_INT_ID_H_INCLUDED_
#define _GLOBAL_INT_ID_H_INCLUDED_

typedef unsigned char T_INT_TYPE;

typedef struct {
		T_INT_TYPE nIntType;		
		unsigned short nDeviceID;
		volatile unsigned short *pErrorReg;
		} T_INT_HEADER;

#define INT_TYPE_UART 		0x01
#define INT_TYPE_SPI		0x02



#endif

