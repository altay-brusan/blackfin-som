#ifndef _SYSLOG_CONF_INCLUDED_
#define _SYSLOG_CONF_INCLUDED_

#include <stdio.h>
#include <datatypes.h>
#ifdef _USE_VDK_
	#include <VDK.h>
#endif
	
#define SYSLOG_TIMEOUT						0xffffff
#define SYSLOG_MAX_MESSAGE_LENGTH			512
//TODO: implement message buffer as linked list
#define SYSLOG_MAX_MESSAGES					10
#define SYSLOG_ACTIVITY_TICKS				100

// errors
#define ERR_SYSLOG_TIMEOUT					(ERR_GENERIC - 10)

// log levels
enum {
	SYSLOG_LEVEL_INFO,
	SYSLOG_LEVEL_ERROR,
	SYSLOG_LEVEL_WARNING
};

typedef struct {
	// currently not used
} T_SYSLOG_CONF;

typedef struct {
	FILE *Stream;
	bool bMessagePosted;
	char acMessageBuffer[SYSLOG_MAX_MESSAGES][SYSLOG_MAX_MESSAGE_LENGTH + 1];
	unsigned char cLogLevel;
	unsigned short nUserId;
#ifdef _USE_VDK_
	VDK_ThreadID ThreadId;
#endif
	unsigned long nMessagesIn;
	unsigned long nMessagesOut;
}T_SYSLOG_SPEC;

// prototypes
T_ERROR_CODE syslog_setup (FILE *pa_SysLogStream, T_SYSLOG_CONF *pa_tSyslogConfig);
T_ERROR_CODE syslog_post (char *pa_acLogMessage, unsigned char pa_cLogLevel, unsigned short pa_nId, void *pa_Reserved);
T_ERROR_CODE syslog_cleanup (void);


#endif

