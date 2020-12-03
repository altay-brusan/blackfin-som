/*****************************************************************************
 * TCM-BF518_BlinkLED.c
 *****************************************************************************/
#include "../../../../../driver/src/bf518/gpio_global.h"
#include "../../../../../driver/src/common/gpioconfig.h"


 
 
#define LED_FLAG_0      _PF10
#define LED_FLAG_1      _PF11

void main( void ) {
    gpio_becomeOutput(LED_FLAG_0);
    gpio_becomeOutput(LED_FLAG_1);
    
    unsigned long wait = 0;
    while (1) {
        gpio_set(LED_FLAG_0);
        gpio_clear(LED_FLAG_1);
        
        for(wait=0; wait<0x1fffff; wait++) {
            asm("nop;");
        }
        
        gpio_set(LED_FLAG_1);
        gpio_clear(LED_FLAG_0);
        
        for(wait=0; wait<0x1fffff; wait++) {
            asm("nop;");
        }
    }
}
