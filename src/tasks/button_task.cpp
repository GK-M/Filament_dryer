#include <Arduino.h>
#include "tasks/button_task.h"
#include "rtos_handles.h"
#include "config.h"


void IRAM_ATTR isrButton(void* arg) {
    ButtonRAW event;
    event.pin = (uint32_t)arg;
    event.edge = (digitalRead(event.pin) == LOW) ? ButtonEdge::PRESSED : ButtonEdge::RELEASED;
    event.timestamp = millis();
    BaseType_t woken = pdFALSE;
    xQueueSendFromISR(xButtonQueue, &event, &woken);
    if(woken) portYIELD_FROM_ISR();
}


void vButtonTask(void *pvParameters) {

    pinMode(Button::Increase,     INPUT_PULLUP);
    pinMode(Button::Decrease,     INPUT_PULLUP);
    pinMode(Button::Mode,         INPUT_PULLUP);
    pinMode(Button::Enter,        INPUT_PULLUP);
    pinMode(Button::PrzyciskEkran,INPUT_PULLUP);
    pinMode(Button::LedPin,             OUTPUT);

    attachInterruptArg(Button::Increase,      isrButton, (void*)Button::Increase,      CHANGE);
    attachInterruptArg(Button::Decrease,      isrButton, (void*)Button::Decrease,      CHANGE);
    attachInterruptArg(Button::Mode,          isrButton, (void*)Button::Mode,          CHANGE);
    attachInterruptArg(Button::Enter,         isrButton, (void*)Button::Enter,         CHANGE);
    attachInterruptArg(Button::PrzyciskEkran, isrButton, (void*)Button::PrzyciskEkran, CHANGE);


    for(;;){

    ButtonRAW event;    

    xQueueReceive(xButtonQueue, &event, portMAX_DELAY);
     
    if()       

    
    
    
    }

}
