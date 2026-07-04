#include <Arduino.h>
#include "config.h"
#include "rtos_handles.h"



void vLedTask(void *pvParameters) {

    pinMode(Pin::LedPin, OUTPUT);

    for (;;) {
    
        
    }
}