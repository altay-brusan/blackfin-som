/**
 *	@file 		clockquery.h
 *	@ingroup 	CLOCKS
 *	
 *	@brief 		Core and System Clock frequency Querying
 *	
 *						
 *		
 *	BLT_DISCLAIMER
 *	
 *	@author 	Roland Oberhammer, 
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

/** @defgroup CLOCKS
 *  @ingroup driverapi
 *  Core & Sys Clocks
 */



#ifndef _CLOCKQUERY_H_INCLUDED_
#define _CLOCKQUERY_H_INCLUDED_

unsigned long 	getCoreClockFrequency (void);
unsigned long 	getSystemClockFrequency (void);
void 			setClockIn (unsigned long pa_nClockIn);

#endif
