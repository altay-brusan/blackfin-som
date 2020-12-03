#ifndef _XMODEM_UART_BUF_H_INCLUDED_
#define _XMODEM_UART_BUF_H_INCLUDED_

#include "../../../driver/src/common/UART/UARTconfig.h"

long XM_ReceiveFile (T_UART_HANDLE hUART, unsigned char *pa_FileBuffer);
long XM_TransmitFile (T_UART_HANDLE hUART, unsigned char *pa_FileBuffer, unsigned long pa_nSize);

#endif


