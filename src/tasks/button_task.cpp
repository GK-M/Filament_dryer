#include <Arduino.h>
#include <Preferences.h>

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
    
    //tablica do przechowywania ostatnich czasów naciśnięć przycisków 
    uint32_t lastPress[22] = {0};
    ButtonRAW event; 
    PID_data pid_data;
    Preferences preferences;
    Screen current_screen;
    EditVar current_var;
    Mode mode;

    for(;;){

    xQueueReceive(xButtonQueue, &event, portMAX_DELAY);

    //Next strona
    if(event.timestamp - lastPress[event.pin] > 50 && event.edge == ButtonEdge::PRESSED && event.pin == Button::Enter){
       lastPress[event.pin] = event.timestamp;
       current_screen = static_cast<Screen>((static_cast<uint8_t>(current_screen) + 1) % static_cast<uint8_t>(Screen::COUNT));
    }

    //Tryby pracy - kiedyś to zrobię 
    if(event.timestamp - lastPress[event.pin] > 50 && event.edge == ButtonEdge::PRESSED && event.pin == Button::Mode){
       lastPress[event.pin] = event.timestamp;
       mode = static_cast<Mode>((static_cast<uint8_t>(mode) + 1) % static_cast<uint8_t>(Mode::COUNT));

    }

    //UP
    if(event.timestamp - lastPress[event.pin] > 50 && event.edge == ButtonEdge::PRESSED && event.pin == Button::Increase){
        lastPress[event.pin] = event.timestamp;

        switch(current_screen){

            case Screen::Main:
                switch(current_var){
                    case EditVar::Setpoint:
                        pid_data.Setpoint += 5.0;
                        if(pid_data.Setpoint > 100.0) pid_data.Setpoint = 100.0;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Setpoint);
                        break;

                    case EditVar::Time:
                        // Implementacja zwiększania czasu
                        break;
                }
            
            case Screen::PID_cook:
                switch(current_var){
                    case EditVar::Kp:
                        pid_data.Kp += 0.1;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Kp);
                        break;
                    case EditVar::Ki:
                        pid_data.Ki += 0.01;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Ki);
                        break;
                    case EditVar::Kd:
                        pid_data.Kd += 0.01;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Kd);
                        break;
                }
        }
    }

    //DOWN
    if(event.timestamp - lastPress[event.pin] > 50 && event.edge == ButtonEdge::PRESSED && event.pin == Button::Decrease){
        lastPress[event.pin] = event.timestamp;

        switch(current_screen){

            case Screen::Main:
                switch(current_var){
                    case EditVar::Setpoint:
                        pid_data.Setpoint -= 5.0;
                        if(pid_data.Setpoint < 0.0) pid_data.Setpoint = 0.0;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Setpoint);
                        break;
                    case EditVar::Time:
                        // Implementacja zmniejszania czasu
                        break;
                }
            
            case Screen::PID_cook:
                switch(current_var){
                    case EditVar::Kp:
                        pid_data.Kp -= 0.1;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Kp);
                        break;
                    case EditVar::Ki:
                        pid_data.Ki -= 0.01;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Ki);
                        break;
                    case EditVar::Kd:
                        pid_data.Kd -= 0.01;
                        xQueueOverwrite(xSetpointQueue, &pid_data.Kd);
                        break;
                }
        }
    
    }
 







  }
 
}


