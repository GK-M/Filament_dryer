#include <Arduino.h>
#include <Preferences.h>

#include "tasks/button_task.h"
#include "tasks/log_task.h"
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


    attachInterruptArg(Button::Increase,      isrButton, (void*)Button::Increase,      CHANGE);
    attachInterruptArg(Button::Decrease,      isrButton, (void*)Button::Decrease,      CHANGE);
    attachInterruptArg(Button::Mode,          isrButton, (void*)Button::Mode,          CHANGE);
    attachInterruptArg(Button::Enter,         isrButton, (void*)Button::Enter,         CHANGE);
    attachInterruptArg(Button::PrzyciskEkran, isrButton, (void*)Button::PrzyciskEkran, CHANGE);
    
    //tablica do przechowywania ostatnich czasów naciśnięć przycisków 
    uint32_t lastPress[22] = {0};
    ButtonRAW buttonRAW; 
    PID_data pid_data;
    Preferences preferences;
    Screen current_screen;
    EditVar current_var;
    Mode mode;

    current_var = EditVar::Setpoint;
    current_screen = Screen::Main;
    mode = Mode::Mode_1;


    for(;;){

    xQueueReceive(xButtonQueue, &buttonRAW, portMAX_DELAY);

    //Next strona
    if(buttonRAW.timestamp - lastPress[buttonRAW.pin] > Timing::DEBOUNCE_MS && buttonRAW.edge == ButtonEdge::PRESSED && buttonRAW.pin == Button::Enter){
       lastPress[buttonRAW.pin] = buttonRAW.timestamp;
       current_screen = static_cast<Screen>((static_cast<uint8_t>(current_screen) + 1) % static_cast<uint8_t>(Screen::COUNT));
    }

    //Tryby pracy - kiedyś to zrobię 
    if(buttonRAW.timestamp - lastPress[buttonRAW.pin] > Timing::DEBOUNCE_MS && buttonRAW.edge == ButtonEdge::PRESSED && buttonRAW.pin == Button::Mode){
       lastPress[buttonRAW.pin] = buttonRAW.timestamp;
       mode = static_cast<Mode>((static_cast<uint8_t>(mode) + 1) % static_cast<uint8_t>(Mode::COUNT));
    }


    // Next - zmienna w zależności od aktualnego ekranu
    if(buttonRAW.timestamp - lastPress[buttonRAW.pin] > Timing::DEBOUNCE_MS && buttonRAW.edge == ButtonEdge::PRESSED && buttonRAW.pin == Button::PrzyciskEkran){
        
        lastPress[buttonRAW.pin] = buttonRAW.timestamp;
        
        switch(current_screen){

            case Screen::Main:
                if(current_var == EditVar::Setpoint) current_var = EditVar::Time;
                else current_var = EditVar::Setpoint;
                break;

            case Screen::PID_cook:

                if(current_var == EditVar::Kp) current_var = EditVar::Ki;
                else if(current_var == EditVar::Ki) current_var = EditVar::Kd;
                else current_var = EditVar::Kp;
            
                break;
        
        }

    }

    //UP
    if(buttonRAW.timestamp - lastPress[buttonRAW.pin] > Timing::DEBOUNCE_MS && buttonRAW.edge == ButtonEdge::PRESSED && buttonRAW.pin == Button::Increase){
        lastPress[buttonRAW.pin] = buttonRAW.timestamp;

        switch(current_screen){

            case Screen::Main:
                switch(current_var){
                    case EditVar::Setpoint:
                        pid_data.Setpoint += 5.0;
                        if(pid_data.Setpoint > 100.0) pid_data.Setpoint = 100.0;
                        LOG("Setpoint is equal to: %f", pid_data.Setpoint);
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;

                    case EditVar::Time:
                        // Implementacja zwiększania czasu
                        break;
                }
            break;

            case Screen::PID_cook:
                switch(current_var){
                    case EditVar::Kp:
                        pid_data.Kp += 0.1;
                        if(pid_data.Kp > pid_data.Kp_max) pid_data.Kp = 0.0;
                        //zapis do pamięci flash
                        preferences.begin("Kp", false);
                        preferences.putFloat("Kp",pid_data.Kp);
                        preferences.end();
                        LOG("Kp is equal to: %f", pid_data.Kp);
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Ki:
                        pid_data.Ki += 0.01;
                        if(pid_data.Ki > pid_data.Ki_max) pid_data.Ki = 0.0;
                       //zapis do pamięci flash 
                        preferences.begin("Ki", false);
                        preferences.putFloat("Ki",pid_data.Ki);
                        preferences.end();
                        LOG("Ki is equal to: %f", pid_data.Ki);
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Kd:
                        pid_data.Kd += 0.01;
                        if(pid_data.Kd > pid_data.Kd_max) pid_data.Kd = 0.0;
                        //zapis do pamięci flash
                        preferences.begin("Kd", false); //fals -> odczyt i zapis, true -> tylko odczyt
                        preferences.putFloat("Kd",pid_data.Kd);
                        preferences.end();
                        LOG("Kd is equal to: %f", pid_data.Kd);
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                }
            break;
        }
    }

    //DOWN
    if(buttonRAW.timestamp - lastPress[buttonRAW.pin] > Timing::DEBOUNCE_MS && buttonRAW.edge == ButtonEdge::PRESSED && buttonRAW.pin == Button::Decrease){
        lastPress[buttonRAW.pin] = buttonRAW.timestamp;

        switch(current_screen){

            case Screen::Main:
                switch(current_var){
                    case EditVar::Setpoint:
                        pid_data.Setpoint -= 5.0;
                        if(pid_data.Setpoint < 0.0) pid_data.Setpoint = 0.0;
                        LOG("Setpoint is equal to: %f", pid_data.Setpoint);
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Time:
                        // Implementacja zmniejszania czasu
                        break;
                }
            break;
            
            case Screen::PID_cook:
                switch(current_var){
                    case EditVar::Kp:
                        pid_data.Kp -= 0.1;
                        LOG("Kp is equal to: %f", pid_data.Kp);
                        //zapis do pamięci flash
                        preferences.begin("Kp", false);
                        preferences.putFloat("Kp",pid_data.Kp);
                        preferences.end();
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Ki:
                        pid_data.Ki -= 0.01;
                        LOG("Ki is equal to: %f", pid_data.Ki);
                        //zapis do pamięci flash
                        preferences.begin("Ki", false);
                        preferences.putFloat("Ki",pid_data.Ki);
                        preferences.end();
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Kd:
                        pid_data.Kd -= 0.01;
                        LOG("Kd is equal to: %f", pid_data.Kd);
                        //zapis do pamięci flash
                        preferences.begin("Kd", false);
                        preferences.putFloat("Kd",pid_data.Kd);
                        preferences.end();
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                }
            break;
        }
    
    }
 


/*
Dodać kolejke do przesłania danych do taska z wyświetlaczem 
min. setpoin i nowe wartości dla PID,
dodać zapis tych wartośći Pid do Pamięci flash
*/




  }
 
}


