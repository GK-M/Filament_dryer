#include <Arduino.h>
#include "config.h"
#include "rtos_handles.h"
#include "tasks/log_task.h"



void vLedTask(void *pvParameters) {

    Led_control led_control;

    pinMode(Pin::LedPin, OUTPUT);
    
    
    ledcAttachPin(Pin::LedPin, 1);
    ledcChangeFrequency(1,Led_data::freq, Led_data::rozdzielczosc);
    ledcWrite(1,1023);

    for (;;) {
    
    vTaskDelay(pdMS_TO_TICKS(50));
        
    }
}