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
 
/** @defgroup 	I2C I2C Interface Controller
 *  @ingroup 	driverapi
 **/

#include <stdlib.h>
#include "I2CMan.h"

#ifdef _USE_VDK_
#include <VDK.h>
#endif

/**
 *	@private
 *	@brief		An object to describe the interface of an I2C driver.
 **/
struct _I2C_IF {
	I2C_FN_POINTERS	mI2CFns;		///< Function pointers.
	int				mIFId;			///< ID Number of the interface.
	struct _I2C_IF	*pNext;			///< Pointer to the next IF in the linked list.
	bool			bInUse;			///< Marks the IF as inUse.
#ifdef _USE_VDK_
	VDK_SemaphoreID	mAccessSem;		///< Access semaphore for the interface.
#endif
};

/**
 *	@private
 *	@brief		An internal Interface HANDLE type.
 **/
typedef struct _I2C_IF *I2C_IF_HANDLE;		///< Interface HANDLE type. (Private).


static I2C_IF_HANDLE g_HANDLE_I2C = NULL;	///< Encapsulated Handle to a linked list of I2C Interface Handles;
static T_I2C_HANDLE g_I2CMainHandle[MAX_NR_OF_I2C_INTERFACES];
static int	g_nI2CInterfaces = 0;


/**
 *	@private
 *	@brief		A private object describing a Handle to an I2C Bus Controller.
 **/
struct I2C_BUS {							// What an I2C Handle really looks like!
	I2C_IF_HANDLE 	hIF;					///< I2C Interface to use.
	void *phIF;								///< I2C Driver Handle.
};


/**
 *	@public
 *	@brief		Adds an Interface to a driver of an I2C Bus Controller.
 *
 *	@param		pa_pFunctions	Object describing the driver interface.
 *
 *	@return		The number of the I2C interface added, or a negative ERROR_CODE.
 **/
T_ERROR_CODE I2CAddInterface(const I2C_FN_POINTERS *pa_pFunctions) {
	
	I2C_IF_HANDLE hIF 		= g_HANDLE_I2C;
	I2C_IF_HANDLE hNewIF	= (I2C_IF_HANDLE) malloc(sizeof(struct _I2C_IF));
	
	if(!hNewIF) {
		return I2C_ERR_NOT_ENOUGH_MEMORY;
	}
	
	// Populate the new IF.
	memcpy(&hNewIF->mI2CFns, pa_pFunctions, sizeof(I2C_FN_POINTERS));	// Copy the function pointers.
	hNewIF->pNext = NULL;
	hNewIF->bInUse = false;

#ifdef _USE_VDK_
	hNewIF->mAccessSem = VDK_CreateSemaphore(1, 1, 1, 0);
#endif
	
	if(!hIF) {	// First IF to be added.
		g_HANDLE_I2C = hNewIF;
		hNewIF->mIFId = 0;		// Set the ID of the interface.			
	} else { // Run to end!
		while(hIF->pNext) {
			hIF = hIF->pNext;
		}
		
		hIF->pNext = hNewIF;	// Add new interface to end of the chain.
		hNewIF->mIFId = hIF->mIFId + 1;		// Set the ID of the interface.
	}
	
	if(hNewIF->mIFId >= MAX_NR_OF_I2C_INTERFACES) {
		I2CRemoveInterface(hNewIF->mIFId);	
		return -1;
	}
	
	return hNewIF->mIFId; // Return the ID of the new interface. (EASY :D)	
}


// Get

T_I2C_HANDLE I2CgetGlobalHandle(int pa_cNr) {
	
	if(pa_cNr < MAX_NR_OF_I2C_INTERFACES) {
		return g_I2CMainHandle[pa_cNr];
	}
	
	return (T_I2C_HANDLE) NULL;
	
}


// Set
void I2CsetGlobalHandle(int pa_cNr, T_I2C_HANDLE hI2C) {
	if(pa_cNr < MAX_NR_OF_I2C_INTERFACES) {	
		g_I2CMainHandle[pa_cNr] = hI2C;	
	}
}


int I2CgetTotalHandles() {
	return g_nI2CInterfaces;
} 


static I2C_IF_HANDLE I2CgetInterface(int pa_nI2C) {
	I2C_IF_HANDLE hIF = g_HANDLE_I2C;
	
	while(hIF) {
		if(hIF->mIFId == pa_nI2C) {
			return hIF;	
		}
		hIF = hIF->pNext;
	}
	
	return (I2C_IF_HANDLE) NULL;
	
}


T_ERROR_CODE I2CRemoveInterface(int pa_nI2C) {
	I2C_IF_HANDLE hIF;
	I2C_IF_HANDLE hIFRM;
	
	hIFRM = I2CgetInterface(pa_nI2C);
	
	if(hIFRM) {
		hIF = g_HANDLE_I2C;
		
		if(hIF == hIFRM) {
			g_HANDLE_I2C = hIF->pNext;
#ifdef _USE_VDK_			
			VDK_DestroySemaphore(hIF->mAccessSem);	// Destroy protected access to the interface.
#endif
			free(hIF);
			g_nI2CInterfaces--;
			return ERR_NONE;		
		}
		
		while(hIF->pNext) {
			
			if(hIF->pNext == hIFRM) {
				hIF->pNext = hIFRM->pNext;
#ifdef _USE_VDK_			
				VDK_DestroySemaphore(hIF->mAccessSem);
#endif
				free(hIF);
				g_nI2CInterfaces--;
				return ERR_NONE;	
			}
			
			hIF = hIF->pNext;
		}
		
	}
	
	return I2C_ERR_INVALID_INTERFACE;
}




T_I2C_HANDLE I2Copen(char pa_cI2C, T_I2C_CONFIG pa_pCfg, T_ERROR_CODE *pError, void *pReserved) {
	T_I2C_HANDLE hI2C = (T_I2C_HANDLE) malloc(sizeof(struct I2C_BUS));
	T_ERROR_CODE Error;
	
	if(pError) {
		*pError = ERR_NONE;	
	}
	
	if(hI2C) {
		hI2C->hIF = I2CgetInterface(pa_cI2C);
		if(!hI2C->hIF) {
			if(pError) {
				*pError = I2C_ERR_INVALID_INTERFACE;	
			}
			free(hI2C);
			return (T_I2C_HANDLE) NULL;	
		}
		
		hI2C->phIF = hI2C->hIF->mI2CFns.fnOpen(pa_cI2C, pa_pCfg, &Error, pReserved);
		
		if(hI2C->phIF) {
			// DO OTHER INIT STUFF HERE -- Get an ACK from the device.
			g_I2CMainHandle[hI2C->hIF->mIFId] = hI2C;
			g_nI2CInterfaces++;
			return hI2C;	
		}
				
		if(pError) {
			*pError = Error;	// 	
		}
		
		return (T_I2C_HANDLE) NULL;
	}
	
	if(pError) {
		*pError = I2C_ERR_NOT_ENOUGH_MEMORY;	
	}
	
	return (T_I2C_HANDLE) NULL;
}


T_ERROR_CODE I2Cclose(T_I2C_HANDLE hI2C) {
	T_ERROR_CODE erResult;
	int i;
	
	if(!hI2C) {
		return ERR_I2C_NULL_POINTER;	
	}
	
	erResult = hI2C->hIF->mI2CFns.fnClose(hI2C->phIF);
	
	if(erResult) {
		return erResult;	
	}
	
	for(i = 0; i < MAX_NR_OF_I2C_INTERFACES; i++) {
		if(g_I2CMainHandle[i] == hI2C) {
			g_I2CMainHandle[i] = NULL;	
		}
	}
	
	free(hI2C);
	
	g_nI2CInterfaces--;
	
	return erResult;	
}

/**
 *	@public
 *	@brief	Reads Data from an I2C Device on the I2C Bus.
 *
 *	
 **/
int I2CreadReg(T_I2C_HANDLE hI2C, 
               unsigned short pa_DevAddr, 
               unsigned char pa_DevReg, 
               unsigned char *buffer, 
               unsigned long size, 
               I2C_CALLBACK *pa_pNb) {
	int RetVal;
	
	if(!hI2C) {
		return ERR_I2C_NULL_POINTER;	
	}
		
#ifdef _USE_VDK_	
	VDK_PendSemaphore(hI2C->hIF->mAccessSem, 0);
	{
#endif
		RetVal = hI2C->hIF->mI2CFns.fnRead(hI2C->phIF, pa_DevAddr, pa_DevReg, buffer, size, pa_pNb);
#ifdef _USE_VDK_
	}
	VDK_PostSemaphore(hI2C->hIF->mAccessSem);
#endif
	
	return RetVal;
}


int I2CwriteReg(T_I2C_HANDLE hI2C, 
                unsigned short pa_DevAddr, 
                unsigned char pa_DevReg, 
                unsigned char *buffer, 
                unsigned long size, 
                I2C_CALLBACK *pa_pNb) {
	int RetVal;
	
	if(!hI2C) {
		return ERR_I2C_NULL_POINTER;	
	}
	
#ifdef _USE_VDK_	
	VDK_PendSemaphore(hI2C->hIF->mAccessSem, 0);
	{
#endif
		RetVal = hI2C->hIF->mI2CFns.fnWrite(hI2C->phIF, pa_DevAddr, pa_DevReg, buffer, size, pa_pNb);
#ifdef _USE_VDK_
	}
	VDK_PostSemaphore(hI2C->hIF->mAccessSem);
#endif
	
	return RetVal;
}

void I2CControl(void) {
	
}

void I2CStatus(void) {
	
}


/**
 *	@public
 *	@brief	Scans the I2C Bus, and calls the provided Callback function with each found device address.	
 *
 **/
T_ERROR_CODE I2CScanBus(T_I2C_HANDLE hI2C, I2C_FN_SCAN_CALLBACK pa_pfnCallback) {
	
	if(!hI2C) {
		return ERR_I2C_NULL_POINTER;	
	}
	
	if(hI2C->hIF->mI2CFns.fnScanBus) {
		
#ifdef _USE_VDK_	
		VDK_PendSemaphore(hI2C->hIF->mAccessSem, 0);
		{
#endif
			hI2C->hIF->mI2CFns.fnScanBus(hI2C->phIF, pa_pfnCallback);	
#ifdef _USE_VDK_
		}
		VDK_PostSemaphore(hI2C->hIF->mAccessSem);
#endif
		
		return ERR_NONE;	
	}
	
	return ERR_I2C_NOT_SUPPORTED;
}









