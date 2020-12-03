/*
	Copyright 2001, 2002 Georges Menie (www.menie.org)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* Modifications made by suedtirolli 2005,
   adapted for the BLACKSheep
*/

/* this code needs standard functions memcpy() and memset()
   and input/output functions _inbyte() and _outbyte().

   the prototypes of the input/output functions are:
     int _inbyte(unsigned short timeout); // msec timeout
     void _outbyte(int c);

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cycle_count_bf.h>
//local includes
#include "..\crc\crc16.h"
#include "../../../driver/src/common/pwrmngt/clockquery.h"
#include "xmodemUART-buf.h"
//#include "clock.h"

#define	XM_MAX_TRANSMIT_SIZE 	(0x7FFFFFFF)
#define XM_MAX_RECEIVE_SIZE		(0x7FFFFFFF)

long xmodemReceive (long destsz);
int xmodemTransmit (long srcsz);

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define XM_NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 10000
#define MAXRETRANS 25

// global stuff
T_UART_HANDLE g_hUART = 0;
unsigned char *g_FileBuffer = 0;
unsigned long g_BytesToTransfer = 0;
unsigned long g_BytesTransfered = 0;


void XM_InstallUartHandler (T_UART_HANDLE hUART) {
	g_hUART = hUART;
}

int _inbyte (unsigned long timeout) {
	T_ERROR_CODE pa_erCode = ERR_NONE;
	int nData;
	unsigned long nCclk = getCoreClockFrequency();
	while (timeout) {
		nData = uart_getChar (g_hUART, &pa_erCode);
		if (pa_erCode != ERR_NONE) {
			//no data in buffer
			unsigned long long int cur,nd;
			unsigned long i = 0;
			_GET_CYCLE_COUNT(cur);
			nd = cur + nCclk / 10000;
			while (cur < nd) {
				_GET_CYCLE_COUNT(cur);
			}
			timeout--;
		} else {
			return (nData);
		}
	}
	return -1;
}

void _outbyte (int c) {
	uart_putChar (g_hUART, (unsigned char)c);
}


static int check(int crc, const unsigned char *buf, int sz) {
	if (crc) {
		unsigned short crc = crc16_ccitt(buf, sz);
		unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
		if (crc == tcrc)
			return 1;
	}
	else {
		int i;
		unsigned char cks = 0;
		for (i = 0; i < sz; ++i) {
			cks += buf[i];
		}
		if (cks == buf[sz])
		return 1;
	}

	return 0;
}

static void flushinput(void) {
	while (_inbyte(((DLY_1S)*3)>>1) >= 0);
}


// Return value: Bytes received.
// 			 <0: Something wrong. (Look at xmodemReceive (...)).
long XM_ReceiveFile (T_UART_HANDLE hUART, unsigned char *pa_FileBuffer) {
	g_FileBuffer = pa_FileBuffer;
	XM_InstallUartHandler (hUART);
	return xmodemReceive(XM_MAX_RECEIVE_SIZE);
	
}

long XM_TransmitFile (T_UART_HANDLE hUART, unsigned char *pa_FileBuffer, unsigned long pa_nSize) {
	g_FileBuffer = pa_FileBuffer;
	g_BytesToTransfer = pa_nSize;
	g_BytesTransfered = 0;
	XM_InstallUartHandler (hUART);
	return xmodemTransmit(XM_MAX_TRANSMIT_SIZE);
}


long CopyFromFile (unsigned char *pa_cBuffer, long count) {
	long i;
	
	for (i=0; i<count; i++) {
		if (g_BytesTransfered < g_BytesToTransfer) {
			*pa_cBuffer++ = *g_FileBuffer++;
			g_BytesTransfered ++;
		} else {
			return i;
		}
	}
	return count;
}

void CopyToFile (unsigned char *pa_cBuffer, int count) {
	int i;
	
	for (i=0; i<count; i++) {
		*g_FileBuffer++ = *pa_cBuffer++;
	}
}

long xmodemReceive(long destsz) {
	unsigned char xbuff[1030]; // 1024 for XModem 1k + 3 head chars + 2 crc + nul
	unsigned char *p;
	int bufsz, crc = 0;
	unsigned char trychar = 'C';
	unsigned char packetno = 1;
	long i, c, len = 0;
	int retry, retrans = MAXRETRANS;

	for(;;) {
		for( retry = 0; retry < 32; ++retry) {
			if (trychar) _outbyte(trychar);
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case SOH:
					bufsz = 128;
					goto start_recv;
				case STX:
					bufsz = 1024;
					goto start_recv;
				case EOT:
					flushinput();
					_outbyte(ACK);
					return len; // normal end
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						flushinput();
						_outbyte(ACK);
						return -1; // canceled by remote
					}
					break;
				default:
					break;
				}
			}
		}
		if (trychar == 'C') { trychar = XM_NAK; continue; }
		flushinput();
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		return -2; // sync error
	start_recv:
		if (trychar == 'C') crc = 1;
		trychar = 0;
		p = &xbuff[0];
		*p++ = c;
		for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
			if ((c = _inbyte(DLY_1S)) < 0) goto reject;
			*p++ = c;
		}
		if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
			(xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
			check(crc, &xbuff[3], bufsz)) {
			if (xbuff[1] == packetno)	{
				register int count = destsz - len;
				if (count > bufsz) count = bufsz;
				if (count > 0) {
					CopyToFile (&xbuff[3], count);	//modified by suedtirolli.
					len += count;
				}
				++packetno;
				retrans = MAXRETRANS+1;
			}
			if (--retrans <= 0) {
				flushinput();
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				return -3; // too many retry error
			}
			_outbyte(ACK);
			continue;
		}
	reject:
		flushinput();
		_outbyte(XM_NAK);
	}
}

int xmodemTransmit(long srcsz) {
	unsigned char xbuff[1030]; // 1024 for XModem 1k + 3 head chars + 2 crc + nul
	int bufsz, crc = -1;
	unsigned char packetno = 1;
	long i, c, len = 0;
	int retry;
	unsigned char AllTransmitted;

	for(;;) {
		for( retry = 0; retry < 32; ++retry) {
			if ((c = _inbyte((DLY_1S)<<1)) >= 0) {
				switch (c) {
				case 'C':
					crc = 1;
					AllTransmitted = 0;
					goto start_trans;
				case XM_NAK:
					crc = 0;
					AllTransmitted = 0;
					goto start_trans;
				case CAN:
					if ((c = _inbyte(DLY_1S)) == CAN) {
						_outbyte(ACK);
						flushinput();
						return -1; // canceled by remote
					}
					break;
				default:
					break;
				}
			}
		}
		_outbyte(CAN);
		_outbyte(CAN);
		_outbyte(CAN);
		flushinput();
		return -2; // no sync

		{start_trans:
			xbuff[0] = SOH; bufsz = 128;
			xbuff[1] = packetno;
			xbuff[2] = ~packetno;
			memset (&xbuff[3], 0, bufsz);
			c = CopyFromFile (&xbuff[3], bufsz);
			if (!AllTransmitted) {
				if (c < bufsz) {
					xbuff[3+c] = CTRLZ;
					AllTransmitted = 1;
				}
				if (crc) {
					unsigned short ccrc = crc16_ccitt(&xbuff[3], bufsz);
					xbuff[bufsz+3] = (ccrc>>8) & 0xFF;
					xbuff[bufsz+4] = ccrc & 0xFF;
				}
				else {
					unsigned char ccks = 0;
					for (i = 3; i < bufsz+3; ++i) {
						ccks += xbuff[i];
					}
					xbuff[bufsz+3] = ccks;
				}
				for (retry = 0; retry < MAXRETRANS; ++retry) {
					for (i = 0; i < bufsz+4+(crc?1:0); ++i) {
						_outbyte(xbuff[i]);
					}
					if ((c = _inbyte(DLY_1S)) >= 0 ) {
						switch (c) {
						case ACK:
							++packetno;
							len += bufsz;
							goto start_trans;
						case CAN:
							if ((c = _inbyte(DLY_1S)) == CAN) {
								_outbyte(ACK);
								flushinput();
								return -1; // canceled by remote
							}
							break;
						case XM_NAK:
						default:
							break;
						}
					}
				}
				_outbyte(CAN);
				_outbyte(CAN);
				_outbyte(CAN);
				flushinput();
				return -4; // xmit error
			}
			else {
				for (retry = 0; retry < 10; ++retry) {
					_outbyte(EOT);
					if ((c = _inbyte((DLY_1S)<<1)) == ACK) break;
				}
				flushinput();
				return (c == ACK)?len:-5;
			}
		}
	}
}

