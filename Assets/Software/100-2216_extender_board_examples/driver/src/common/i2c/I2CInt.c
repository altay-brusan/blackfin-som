/**
 *    @file         I2CInt.c
 *    @ingroup     	I2C
 *    
 *    @brief        I2C Driver for Internal interfaces
 *    
 *                
 *    BLT_DISCLAIMER
 *    
 *    @author     Thomas Maier
 *    
 *    @cond svn
 *    
 *    Information of last commit
 *    $Rev::               $:  Revision of last commit
 *    $Author::            $:  Author of last commit
 *    $Date::              $:  Date of last commit
 *    
 *    @endcond
 **/
 
/** @defgroup I2C
 *    @ingroup driverapi
 *    I2C Interface Controller
 */
#include <conio.h>
#include <services/services.h>
#include "I2CInt.h"            ///< PUBLIC Definitions of I2C Emulator.
#include "I2CMan.h"

extern unsigned int g_nI2Ccount;        // delcared in i2c_global.c
extern T_I2C_SPEC g_I2Cspec[];            // delcared in i2c_global.c

struct T_I2C_INT {
    volatile    bool            bInUse;            ///< If this I2C has been acquired for an operation.
    I2C_CALLBACK  *pCallback;                      ///< Callback Function and Parameter object to be called when using non-blocking I/O.
    T_I2C_SPEC *pSpec;
    unsigned char cI2Cnr;
    volatile E_I2C_STATE state;
    I2CMessageHandler all_handler;
    I2C_FN_SCAN_CALLBACK scan_callback;
    unsigned long nIvg;
    unsigned char cFastFlag;
    unsigned char *pMsg_start;
    unsigned char *pMsg_buffer;
    unsigned char cMsg_size;
    unsigned char cXmt_counter ;
    unsigned char cRcv_counter ;
};



/**
 *    @brief        High-Level I2C Register Write function.
 *
 *    Provides a complete I2C Write sequence, from start to stop.
 *
 *    @param        hI2C        Handle to the I2C Controller.
 *    @param        pa_DevAddr    I2C Slave address. (7-bit addresses only).
 *    @param        pa_DevReg    Register address on the Slave to be written to.
 *    @param        pa_cBuffer    A byte of data to be written to the register on the slave.
 *
 *    @return        ERR_I2C_NACK if the SLAVE didn't ACK the transmission. Else the number of bytes actually written.
 **/
int I2CIntWriteReg(T_I2C_INT_HANDLE pa_hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *pa_cBuffer, int pa_nSize, I2C_CALLBACK *pa_pNb) {
    
   
    int i = (int)ERR_GENERIC;
    
    if (pa_hI2C) {
         
        struct T_I2C_INT *pInst = (struct T_I2C_INT *)pa_hI2C;
        unsigned long nTimeout = I2C_RW_TIMEOUT;

        pInst->state= I2C_SET;
        pInst->pMsg_buffer = pa_cBuffer ;
        pInst->cMsg_size = pa_nSize;
        pInst->pCallback = pa_pNb;

        *(g_I2Cspec[pInst->cI2Cnr].master_addr) = pa_DevAddr;
        *(g_I2Cspec[pInst->cI2Cnr].xmt_data8) = pa_DevReg;
        *(g_I2Cspec[pInst->cI2Cnr].fifo_ctl) = 0;
        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
        *(g_I2Cspec[pInst->cI2Cnr].int_mask) = XMTSERV | MCOMP;
        *(g_I2Cspec[pInst->cI2Cnr].master_ctl)= ( pa_nSize +1 ) << 6 | MEN |  pInst->cFastFlag;
    
        if(!pa_pNb) {
            nTimeout = I2C_INT_RW_TIMEOUT;
#ifdef _USE_VDK_    
            while( (pInst->state != I2C_IDLE) && (nTimeout)) {
                Sleep(1);
                nTimeout--;
            }
#else
            while( pInst->state != I2C_IDLE );
#endif
        
            if (nTimeout == 0) {
                i = (int) ERR_I2C_TIMEOUT;
            } else {
                i = pa_nSize;
            }
        }
    
    } else {
        i = (int) ERR_I2C_INVALID_HANDLE;
    }
    
    return i;
}



/**
 *    @brief        High-Level I2C Register Read function.
 *
 *    Provides a complete I2C Read sequence, from start to stop.
 *
 *    @param        hI2C        Handle to the I2C Controller.
 *    @param        pa_DevAddr  I2C Slave address. (7-bit addresses only).
 *    @param        pa_DevReg   Register address on the Slave to be written to.
 *    @param        buffer      Buffer to fill.
 *    @param        size        Number of bytes to read.
 *
 *    @return        ERR_I2C_NACK if the Slave didn't ACK any part of the Read Sequence..
 *    @return        Success is always a positive number, errors are negative.
 *    @return        On Success, the value is the number of bytes actually read.
 **/
int I2CIntReadReg(T_I2C_INT_HANDLE pa_hI2C, unsigned short pa_DevAddr, unsigned char pa_DevReg, unsigned char *pa_pcBuffer, int pa_nSize, I2C_CALLBACK *pa_pNb) {
    
    int i = (int)ERR_GENERIC;
    
    if (pa_hI2C) {

        unsigned long nTimeout = I2C_RW_TIMEOUT;    
        struct T_I2C_INT *pInst = (struct T_I2C_INT *)pa_hI2C;
        
        pInst->state = I2C_GET;
        pInst->pMsg_buffer = pa_pcBuffer;
        pInst->cMsg_size = pa_nSize;

        *(g_I2Cspec[pInst->cI2Cnr].master_addr) = pa_DevAddr;
        *(g_I2Cspec[pInst->cI2Cnr].xmt_data8) = pa_DevReg;
        *(g_I2Cspec[pInst->cI2Cnr].fifo_ctl) = 0;
        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
        *(g_I2Cspec[pInst->cI2Cnr].int_mask) = MCOMP;
        *(g_I2Cspec[pInst->cI2Cnr].master_ctl) = 1 << 6 | RSTART | MEN | pInst->cFastFlag;

        if(!pa_pNb) {
            nTimeout = I2C_INT_RW_TIMEOUT;
#ifdef _USE_VDK_    
            while( (pInst->state != I2C_IDLE ) && (nTimeout)) {
                Sleep(1);
                nTimeout--;
            }
#else
            while( pInst->state != I2C_IDLE );
#endif

            if (nTimeout == 0) {
                i = (int) ERR_I2C_TIMEOUT;
            } else {
                i = pa_nSize;
            }
        }
    
    } else {
        i = (int) ERR_I2C_INVALID_HANDLE;
    }
    
    return i;    // Return the bytes read.
    
}
    


ADI_INT_HANDLER_RESULT I2CIntHandler(void *pa_cClientArg) {
    struct T_I2C_INT *pInst = (struct T_I2C_INT *)pa_cClientArg;
    
    if (adi_int_SICInterruptAsserted(pInst->pSpec->tPeripheralID) == ADI_INT_RESULT_ASSERTED) {
        
        u16 irq = *(g_I2Cspec[pInst->cI2Cnr].int_stat);
        
        if( irq )
        {
            u16 stat =  *(g_I2Cspec[pInst->cI2Cnr].master_stat);
            u16 addr =  *(g_I2Cspec[pInst->cI2Cnr].master_addr);
            
            if(irq & MERR) {
                *(g_I2Cspec[pInst->cI2Cnr].int_stat) = MERR;
            } else {

                switch( pInst->state )
                {
                    case I2C_SCANBUS:
                    {
                
                        if( ! addr )
                        {
                            *(g_I2Cspec[pInst->cI2Cnr].int_mask) = 0;
                            pInst->state = I2C_IDLE;
                            *(g_I2Cspec[pInst->cI2Cnr].master_stat) = 0x003f;
                            *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        }
                        else
                        {
                            *(g_I2Cspec[pInst->cI2Cnr].master_stat) = 0x003f;
                            *(g_I2Cspec[pInst->cI2Cnr].master_addr) = *(g_I2Cspec[pInst->cI2Cnr].master_addr) - 1;
                            *(g_I2Cspec[pInst->cI2Cnr].master_ctl) = MEN | pInst->cFastFlag;
                            *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        }
                
                        if( ( ! ( stat & ANAK ) ) && pInst->scan_callback )
                            pInst->scan_callback( (u8)addr );
                
                        break;
                    }
            
                    case I2C_WRITE:
                    case I2C_SET:
                    {
                        if( irq & XMTSERV )
                        {
                            *(g_I2Cspec[pInst->cI2Cnr].xmt_data8) = *(pInst->pMsg_buffer++);
                            pInst->cXmt_counter++;
                        }
                        if( irq & MCOMP )
                        {
                            if( pInst->all_handler )
                                pInst->all_handler ( pInst->cXmt_counter, *(pInst->pMsg_start) );
                            *(g_I2Cspec[pInst->cI2Cnr].fifo_ctl) = 0x0001; //flush fifo
                            asm( "ssync;" );
                            *(g_I2Cspec[pInst->cI2Cnr].fifo_ctl) = 0;
                            *(g_I2Cspec[pInst->cI2Cnr].int_mask) = 0;
                            pInst->state = I2C_IDLE;
                        }
                        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        break;
                    }
                    case I2C_GET:
                    {
                        if( irq & MCOMP )
                        {
                            *(g_I2Cspec[pInst->cI2Cnr].int_mask) = RCVSERV | MCOMP;
                            *(g_I2Cspec[pInst->cI2Cnr].master_ctl) = pInst->cMsg_size << 6 | MDIR | MEN | pInst->cFastFlag;
                            pInst->state = I2C_READ;
                        }
                        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        break;
                    }
                    case I2C_EXT_GET0:
                    {
                        if( irq & XMTSERV )
                        {
                            *(g_I2Cspec[pInst->cI2Cnr].xmt_data8) = *(g_I2Cspec[pInst->cI2Cnr].master_addr) | 1;
                            pInst->state = I2C_EXT_GET1;
                        }
                        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        break;
                    }
                    case I2C_EXT_GET1:
                    {
                        if( irq & MCOMP )
                        {
                            *(g_I2Cspec[pInst->cI2Cnr].int_mask) = RCVSERV | MCOMP;
                            *(g_I2Cspec[pInst->cI2Cnr].master_ctl) = pInst->cMsg_size << 6 | MDIR | MEN | pInst->cFastFlag;
                            pInst->state = I2C_READ;
                        }
                        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        break;
                    }
            
                    case I2C_READ:
                    {
                        if( irq & RCVSERV )
                        {
                            *(pInst->pMsg_buffer++) = *(g_I2Cspec[pInst->cI2Cnr].rcv_data8);
                            pInst->cRcv_counter++;
                        }
                        else if( irq & MCOMP )
                        {
                            if(  pInst->all_handler )
                                 pInst->all_handler( pInst->cRcv_counter, *(pInst->pMsg_start) );
                            *(g_I2Cspec[pInst->cI2Cnr].int_mask) = 0;
                            pInst->state = I2C_IDLE;
                        }
                        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        break;
                    }
                
                    case I2C_IDLE:
                        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
                        *(g_I2Cspec[pInst->cI2Cnr].int_mask) = 0;
                        break;
                }
            }
            return ADI_INT_RESULT_PROCESSED;
        
        } else {
            return ADI_INT_RESULT_NOT_PROCESSED;
        }
    
    } else {
        return ADI_INT_RESULT_NOT_PROCESSED;
    }
}


T_I2C_INT_HANDLE I2CIntOpen(unsigned char pa_cI2CIntnr, unsigned long pa_nBitRate, unsigned long pa_nSystemClk, T_ERROR_CODE *pError, void *pa_pReserved) {
    
    T_I2C_INT_HANDLE hI2C = 0;
    
    if (pa_cI2CIntnr < g_nI2Ccount) {
        
        if (!g_I2Cspec[pa_cI2CIntnr].bInUse) {
        
            g_I2Cspec[pa_cI2CIntnr].bInUse = true;
        
            //allocate space for struct
            hI2C = (T_I2C_INT_HANDLE) malloc(sizeof(struct T_I2C_INT));
    
            if(hI2C) {
                // init hardware mux 
                I2CplatformInit(pa_cI2CIntnr);
    
                hI2C->pSpec = (T_I2C_SPEC *)&(g_I2Cspec[pa_cI2CIntnr]);
                hI2C->cI2Cnr = pa_cI2CIntnr;    
                hI2C->state  = I2C_UNCONFIGURED;
        
                hI2C->all_handler = 0; // setzt den default handler
                
                //calculate and set clock
            
                *(g_I2Cspec[pa_cI2CIntnr].clkdiv) = 5000000 / pa_nBitRate; //set SCL freq
                *(g_I2Cspec[pa_cI2CIntnr].clkdiv) |= *(g_I2Cspec[pa_cI2CIntnr].clkdiv) << 8;
                *(g_I2Cspec[pa_cI2CIntnr].control) = 1 << 7 | (u16)( pa_nSystemClk / 10000000. ); //enable twi and set clk ref.
        
                hI2C->cFastFlag = 0;
        
                if( pa_nBitRate > 100000 )
                    hI2C->cFastFlag = FAST;
            
                // Register RXTX hook
        
                if (adi_int_SICGetIVG(hI2C->pSpec->tPeripheralID, &(hI2C->nIvg)) == ADI_INT_RESULT_SUCCESS ) {
                    if(adi_int_CECHook( (hI2C->nIvg)/*INT_LEVEL*/, I2CIntHandler, (void *)hI2C, FALSE ) == ADI_INT_RESULT_SUCCESS){
            
                        if(adi_int_SICEnable( hI2C->pSpec->tPeripheralID) == ADI_INT_RESULT_SUCCESS){
                
                    
                            hI2C->cXmt_counter = 0;
                            hI2C->cRcv_counter = 0;
                    
                            hI2C->state  = I2C_IDLE;
        
                        }
                        else {
                            *pError = ERR_I2C_HOOK_INTERRUPT;
                            g_I2Cspec[pa_cI2CIntnr].bInUse = false;
                            free (hI2C);
                            hI2C = 0;
                        }
                    }        
                    else {
                        *pError = ERR_I2C_HOOK_INTERRUPT;
                         g_I2Cspec[pa_cI2CIntnr].bInUse = false;
                         free (hI2C);
                         hI2C = 0;
                    }            
                }
                else {
                      g_I2Cspec[pa_cI2CIntnr].bInUse = false;
                      free (hI2C);
                      hI2C = 0;
                      *pError = ERR_I2C_HOOK_INTERRUPT;
                }
    
            } else {
                g_I2Cspec[pa_cI2CIntnr].bInUse = false;
                *pError = ERR_I2C_NOT_ENOUGH_MEM;
                hI2C = 0;
            }
    
        } else {
            *pError = ERR_I2C_IN_USE;
            hI2C = 0;
        }
    } else {
        *pError = ERR_ILLEGAL_I2C_NUMBER;
    }
    return hI2C;
    
}

T_ERROR_CODE I2CIntClose(T_I2C_INT_HANDLE pa_hI2C) {
    
    if (pa_hI2C) {
    
        struct T_I2C_INT *pInst = (struct T_I2C_INT*) pa_hI2C;
    
        // unhook interrupts
        adi_int_SICDisable( pInst->pSpec->tPeripheralID );
        adi_int_CECUnhook( pInst->nIvg, I2CIntHandler, (void *)pInst );

        free(pa_hI2C);    // Free the Handle.
        
        // mark as free
        g_I2Cspec[pInst->cI2Cnr].bInUse = false;        

    } else {
        return ERR_I2C_INVALID_HANDLE;    
    }
    
    return ERR_NONE;
}


/*
    I2C Manager Open interface wrapper.
*/
static T_I2C_INT_HANDLE I2CIFopen(char pa_cI2C, T_I2C_INT_CONFIG *pa_ptCfg, T_ERROR_CODE *pa_pError, void *pa_pReserved) {
    return I2CIntOpen(pa_cI2C, pa_ptCfg->mnBitRate, pa_ptCfg->mnSystemClk, pa_pError, pa_pReserved);
}


static void I2CIntScanBus(T_I2C_INT_HANDLE pa_hI2C, I2C_FN_SCAN_CALLBACK callback) {
    
    struct T_I2C_INT *pInst = (struct T_I2C_INT *)pa_hI2C;
    
    if(pInst) {
        unsigned long nTimeout = I2C_RW_TIMEOUT;
    
        pInst->state = I2C_SCANBUS;
        pInst->scan_callback = callback;
    
        *(g_I2Cspec[pInst->cI2Cnr].fifo_ctl) = 0;
        *(g_I2Cspec[pInst->cI2Cnr].master_addr) = 128;
        *(g_I2Cspec[pInst->cI2Cnr].int_stat) = 0x00ff;
        *(g_I2Cspec[pInst->cI2Cnr].int_mask) = MCOMP;
        *(g_I2Cspec[pInst->cI2Cnr].master_ctl) = MEN;      

        nTimeout = I2C_RW_TIMEOUT;
#ifdef _USE_VDK_    
        while( (pInst->state != I2C_IDLE ) && (nTimeout)) {
            Sleep(1);
            nTimeout--;
        }
#else
        while( pInst->state != I2C_IDLE );
#endif
        
    }
}


static const I2C_FN_POINTERS g_I2Capi = {
    (I2C_FN_OPEN)         I2CIFopen,
    (I2C_FN_CLOSE)         I2CIntClose,
    (I2C_FN_READ_REG)        I2CIntReadReg,
    (I2C_FN_WRITE_REG)        I2CIntWriteReg,
    (I2C_FN_SCANBUS)    I2CIntScanBus,
};

const I2C_FN_POINTERS *I2CInt_getFunctions(void) {
    return &g_I2Capi;
}


