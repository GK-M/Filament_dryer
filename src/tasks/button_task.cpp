#include <Arduino.h>
#include "tasks/button_task.h"
#include "rtos_handles.h"
#include "config.h"

void vButtonTask(void *pvParameters) {

    pinMode(Button::Increase,     INPUT_PULLUP);
    pinMode(Button::Decrease,     INPUT_PULLUP);
    pinMode(Button::Mode,         INPUT_PULLUP);
    pinMode(Button::Enter,        INPUT_PULLUP);
    pinMode(Button::PrzyciskEkran,INPUT_PULLUP);
    pinMode(Button::LedPin,             OUTPUT);

    for(;;){



    
    
    
    }

}
