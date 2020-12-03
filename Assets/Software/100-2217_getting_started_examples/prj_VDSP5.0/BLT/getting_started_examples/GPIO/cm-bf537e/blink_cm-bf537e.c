/*****************************************************************************
 * blink_cm-bf537e.c
 *****************************************************************************/
#include "../../../../../driver/src/common/gpioconfig.h"
#include "../../../../../driver/src/bf537/gpio_global.h"
 
 
#define LED_FLAG_0      _PG10
#define LED_FLAG_1      _PG11

void main( void ) {
    gpio_becomeOutput(LED_FLAG_0);
    gpio_becomeOutput(LED_FLAG_1);
     
    
    
    unsigned long wait = 0;
    while (1) {
        gpio_set(LED_FLAG_0);
        gpio_clear(LED_FLAG_1); 
        
        for(wait=0; wait<0x8fffff; wait++) {
            asm("nop;");
        }
        
        gpio_set(LED_FLAG_1);
        gpio_clear(LED_FLAG_0);
        
        for(wait=0; wait<0x8fffff; wait++) {
            asm("nop;");
        }
    }
}
