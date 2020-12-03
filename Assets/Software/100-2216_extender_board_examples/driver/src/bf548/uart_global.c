#include "../common/UART/uart.h"
#include <cdefbf548.h>
#include <services/services.h>

// global data for synchronization of UARTs

bool g_bUARTsInitialized = false;


T_UART_ENTRY g_UARTtable[MAX_NOF_UART];
T_UART_SPEC g_UARTspec[] = {
    {
        ADI_INT_DMA6_UART0_RX,
        ADI_INT_DMA7_UART0_TX,
        ADI_INT_UART0_STATUS,
        ADI_INT_DMAC0_ERROR,
        ADI_INT_DMAC0_ERROR,        
        pUART0_LCR,
        pUART0_GCTL,
        pUART0_DLL,
        pUART0_DLH,
        pUART0_IER_CLEAR,
        pUART0_LSR,
        pUART0_THR,
        pUART0_RBR,
        pUART0_IER_SET,
        pDMA7_CONFIG,
        (volatile void **) DMA7_START_ADDR,
        pDMA7_X_COUNT,
        pDMA7_X_MODIFY,
        pDMA7_Y_COUNT,
        pDMA7_Y_MODIFY,
        (volatile void **)DMA7_NEXT_DESC_PTR,
        pDMA7_IRQ_STATUS,
        pDMA6_CONFIG,
        (volatile void **)DMA6_START_ADDR,
        pDMA6_X_COUNT,
        pDMA6_X_MODIFY,
        pDMA6_Y_COUNT,
        pDMA6_Y_MODIFY,
        (volatile void **)DMA6_NEXT_DESC_PTR,
        pDMA6_IRQ_STATUS    
    },
    {
        ADI_INT_DMA8_UART1_RX,
        ADI_INT_DMA9_UART1_TX,
        ADI_INT_UART1_STATUS,
        ADI_INT_DMAC0_ERROR,
        ADI_INT_DMAC0_ERROR,                
        pUART1_LCR,
        pUART1_GCTL,
        pUART1_DLL,
        pUART1_DLH,
        pUART1_IER_CLEAR,
        pUART1_LSR,
        pUART1_THR,
        pUART1_RBR,
        pUART1_IER_SET,
        pDMA9_CONFIG,
        (volatile void **) DMA9_START_ADDR,
        pDMA9_X_COUNT,
        pDMA9_X_MODIFY,
        pDMA9_Y_COUNT,
        pDMA9_Y_MODIFY,
        (volatile void **)DMA9_NEXT_DESC_PTR,
        pDMA9_IRQ_STATUS,
        pDMA8_CONFIG,
        (volatile void **)DMA8_START_ADDR,
        pDMA8_X_COUNT,
        pDMA8_X_MODIFY,
        pDMA8_Y_COUNT,
        pDMA8_Y_MODIFY,
        (volatile void **)DMA8_NEXT_DESC_PTR,
        pDMA8_IRQ_STATUS    
    },
    
    {
        ADI_INT_DMA18_SPORT2_RX,
        ADI_INT_DMA19_SPORT2_TX,
        ADI_INT_UART2_STATUS,
        ADI_INT_DMAC1_ERROR,
        ADI_INT_DMAC1_ERROR,                
        pUART2_LCR,
        pUART2_GCTL,
        pUART2_DLL,
        pUART2_DLH,
        pUART2_IER_CLEAR,
        pUART2_LSR,
        pUART2_THR,
        pUART2_RBR,
        pUART2_IER_SET,
        pDMA19_CONFIG,
        (volatile void **) DMA19_START_ADDR,
        pDMA19_X_COUNT,
        pDMA19_X_MODIFY,
        pDMA19_Y_COUNT,
        pDMA19_Y_MODIFY,
        (volatile void **)DMA19_NEXT_DESC_PTR,
        pDMA19_IRQ_STATUS,
        pDMA18_CONFIG,
        (volatile void **)DMA18_START_ADDR,
        pDMA18_X_COUNT,
        pDMA18_X_MODIFY,
        pDMA18_Y_COUNT,
        pDMA18_Y_MODIFY,
        (volatile void **)DMA18_NEXT_DESC_PTR,
        pDMA18_IRQ_STATUS   
    },
    
    {
        ADI_INT_DMA20_SPORT3_RX,
        ADI_INT_DMA21_SPORT3_TX,
        ADI_INT_UART3_STATUS,
        ADI_INT_DMAC1_ERROR,
        ADI_INT_DMAC1_ERROR,                
        pUART3_LCR,
        pUART3_GCTL,
        pUART3_DLL,
        pUART3_DLH,
        pUART3_IER_CLEAR,
        pUART3_LSR,
        pUART3_THR,
        pUART3_RBR,
        pUART3_IER_SET,
        pDMA21_CONFIG,
        (volatile void **) DMA21_START_ADDR,
        pDMA21_X_COUNT,
        pDMA21_X_MODIFY,
        pDMA21_Y_COUNT,
        pDMA21_Y_MODIFY,
        (volatile void **)DMA21_NEXT_DESC_PTR,
        pDMA21_IRQ_STATUS,
        pDMA20_CONFIG,
        (volatile void **)DMA20_START_ADDR,
        pDMA20_X_COUNT,
        pDMA20_X_MODIFY,
        pDMA20_Y_COUNT,
        pDMA20_Y_MODIFY,
        (volatile void **)DMA20_NEXT_DESC_PTR,
        pDMA20_IRQ_STATUS   
    }
};

unsigned int g_nUARTcount = sizeof (g_UARTspec) / sizeof (T_UART_SPEC);

bool uart_platformInit(int pa_nUARTnumber) {
    bool bResult = true;
    
    switch (pa_nUARTnumber) {
        case 0: {
            // enable uart 0 tx pin
            *pPORTE_MUX &= 0xffff3fff;      // clear bit 14,15
            *pPORTE_FER |= (1 << 7);
            // enable uart 0 rx pin
            *pPORTE_MUX &= 0xfffcffff;      // clear bit 17,16
            *pPORTE_FER |= (1 << 8);
            break;
        }       
        case 1: {
            // enable uart 1 tx pin
            *pPORTH_MUX &= 0xfffffffc;      // clear bit 1,0
            *pPORTH_FER |= (1 << 0);
            // enable uart 1 rx pin
            *pPORTH_MUX &= 0xfffffff3;      // clear bit 3,2
            *pPORTH_FER |= (1 << 1);
            break;
        }
        case 2: {
            // enable uart 2 tx pin
            *pPORTB_MUX &= 0xfffffcff;      // clear bit 9,8
            *pPORTB_FER |= (1 << 4);
            // enable uart 2 rx pin
            *pPORTB_MUX &= 0xfffff3ff;      // clear bit 11,10
            *pPORTB_FER |= (1 << 5);
            // map DMA to UART2
            *pDMA18_PERIPHERAL_MAP = 0xC000;
            *pDMA19_PERIPHERAL_MAP = 0xD000;            
            break;
        }
        case 3: {
            // enable uart 3 tx pin
            *pPORTB_MUX &= 0xffffcfff;      // clear bit 13,12
            *pPORTB_FER |= (1 << 6);
            // enable uart 3 rx pin
            *pPORTB_MUX &= 0xffff3fff;      // clear bit 15,14
            *pPORTB_FER |= (1 << 7);
            // map DMA to UART3
            *pDMA20_PERIPHERAL_MAP = 0xE000;
            *pDMA21_PERIPHERAL_MAP = 0xF000;            
            break;
        }
        default: bResult = false;
        
    }
    
    return bResult;
}


/**
 *  @private
 *  @brief      Initialises the global UART description table
 *  
 **/
void UARTs_init(void) {
#ifdef _USE_VDK_
    void *pExitCriticalArg = adi_int_EnterCriticalRegion(NULL);     // Enter a Critical Region (Interrupts Disabled!)
#endif  
    
        int i = 0;
        for (i = 0; i < MAX_NOF_UART; ++i) {
            g_UARTtable[i].nUARTstatus = 0;
        }

#ifdef _USE_VDK_        
    adi_int_ExitCriticalRegion(pExitCriticalArg);       // Exit the Critical Region
#endif  
}
