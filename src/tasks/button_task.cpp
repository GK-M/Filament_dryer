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
    Timer_data timer_data;

    Display_data display_data;
    
    

    uint32_t active_held_pin = 0;       
    uint32_t hold_start_time = 0;       
    uint32_t last_repeat_time = 0;      
    

    current_var = EditVar::Setpoint;
    current_screen = Screen::Main;
    mode = Mode::Mode_1;


    for(;;){

    if(xQueueReceive(xButtonQueue, &buttonRAW, pdMS_TO_TICKS(200)) == pdTRUE){

    if(buttonRAW.timestamp - lastPress[buttonRAW.pin] < Timing::DEBOUNCE_MS) {
        continue; 
    }

    if (buttonRAW.edge == ButtonEdge::RELEASED) {
        if (active_held_pin == buttonRAW.pin) active_held_pin = 0; 
        continue; 
    }

    lastPress[buttonRAW.pin] = buttonRAW.timestamp;
    active_held_pin = buttonRAW.pin;
    hold_start_time = buttonRAW.timestamp;
    last_repeat_time = buttonRAW.timestamp;

    //Next strona
    if(buttonRAW.pin == Button::Enter){
       lastPress[buttonRAW.pin] = buttonRAW.timestamp;
       current_screen = static_cast<Screen>((static_cast<uint8_t>(current_screen) + 1) % static_cast<uint8_t>(Screen::COUNT));
       LOG("Aktualna strona to: %d", (int)current_screen);
       display_data.screen  = current_screen;
       display_data.editvar = current_var;
       display_data.mode    = mode;
       xQueueOverwrite(xDisplayQueue, &display_data);
    }

    //Tryby pracy - kiedyś to zrobię
    if(buttonRAW.pin == Button::Mode){
       lastPress[buttonRAW.pin] = buttonRAW.timestamp;
       mode = static_cast<Mode>((static_cast<uint8_t>(mode) + 1) % static_cast<uint8_t>(Mode::COUNT));
       LOG("Aktualny Tryb pracy to: %d", (int)mode);
       display_data.screen  = current_screen;
       display_data.editvar = current_var;
       display_data.mode    = mode;
       xQueueOverwrite(xDisplayQueue, &display_data);
    }

    // Next - zmienna w zależności od aktualnego ekranu
    if(buttonRAW.pin == Button::PrzyciskEkran){
        
        lastPress[buttonRAW.pin] = buttonRAW.timestamp;
        
        switch(current_screen){

            case Screen::Main:
                if(current_var == EditVar::Setpoint) current_var = EditVar::Time;
                else current_var = EditVar::Setpoint;
                LOG("Edytujesz zminną : %d", (int)current_var);
                display_data.screen  = current_screen;
                display_data.editvar = current_var;
                display_data.mode    = mode;
                xQueueOverwrite(xDisplayQueue, &display_data);
                break;

            case Screen::PID_cook:

                if(current_var == EditVar::Kp) current_var = EditVar::Ki;
                else if(current_var == EditVar::Ki) current_var = EditVar::Kd;
                else current_var = EditVar::Kp;
                LOG("Edytujesz zminną : %d", (int)current_var);
                display_data.screen  = current_screen;
                display_data.editvar = current_var;
                display_data.mode    = mode;
                xQueueOverwrite(xDisplayQueue, &display_data);
                break;
        
        }

    }

    //UP
    if(buttonRAW.pin == Button::Increase){
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
                        timer_data.SetCzas = timer_data.SetCzas + 900000UL;
                        timer_data.SetCzas = constrain(timer_data.SetCzas, 0UL, 86400000UL); // ograniczenie do 24 godzin
                        timer_data.SetCzasMin = (timer_data.SetCzas / 60000UL) % 60U;
                        timer_data.SetCzasGodz = (timer_data.SetCzas / 3600000UL) % 24U; 
                        LOG("SetCzas is equal to: %d:%d", timer_data.SetCzasGodz, timer_data.SetCzasMin);
                        xQueueOverwrite(xTimerQueue, &timer_data);         
                        break;
                }
            break;

            case Screen::PID_cook:
                switch(current_var){
                    case EditVar::Kp:
                        pid_data.Kp += 0.1;
                        if(pid_data.Kp > pid_data.Kp_max) pid_data.Kp = 0.0;
                        if(pid_data.Kp < 0) pid_data.Kp = 0.0;
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
                        if(pid_data.Ki < 0) pid_data.Ki = 0.0;
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
                        if(pid_data.Kd < 0) pid_data.Kd = 0.0;
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
    if(buttonRAW.pin == Button::Decrease){
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
                        if(timer_data.SetCzas >= 900000UL) timer_data.SetCzas = timer_data.SetCzas - 900000UL;
                        else timer_data.SetCzas = 0UL;
                        timer_data.SetCzas = constrain(timer_data.SetCzas, 0UL, 86400000UL); // ograniczenie do 24 godzin
                        timer_data.SetCzasMin = (timer_data.SetCzas / 60000UL) % 60U;
                        timer_data.SetCzasGodz = (timer_data.SetCzas / 3600000UL) % 24U; 
                        LOG("SetCzas is equal to: %d:%d", timer_data.SetCzasGodz, timer_data.SetCzasMin);
                        xQueueOverwrite(xTimerQueue, &timer_data);                    
                        break;             
                }
            
            break;
            
            case Screen::PID_cook:
                switch(current_var){
                    case EditVar::Kp:
                        pid_data.Kp -= 0.1;
                        if(pid_data.Kp < 0) pid_data.Kp = 0.0;
                        LOG("Kp is equal to: %f", pid_data.Kp);
                        //zapis do pamięci flash
                        preferences.begin("Kp", false);
                        preferences.putFloat("Kp",pid_data.Kp);
                        preferences.end();
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Ki:
                        pid_data.Ki -= 0.01;
                        if(pid_data.Ki < 0) pid_data.Ki = 0.0;
                        LOG("Ki is equal to: %f", pid_data.Ki);
                        //zapis do pamięci flash
                        preferences.begin("Ki", false);
                        preferences.putFloat("Ki",pid_data.Ki);
                        preferences.end();
                        xQueueOverwrite(xSetpointQueue, &pid_data);
                        break;
                    case EditVar::Kd:
                        pid_data.Kd -= 0.01;
                        if(pid_data.Kd < 0) pid_data.Kd = 0.0;
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



  }
  else{
    if (active_held_pin != 0) {
                uint32_t current_time = millis();
        
        if(digitalRead(active_held_pin) == HIGH) active_held_pin = 0;
        else{
            
        if (current_time - hold_start_time >= Timing::HOLD_MS) {
            
            if (current_time - last_repeat_time >= Timing::Repeat_button_time) {
                last_repeat_time = current_time;

                if (active_held_pin == Button::Increase) {
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
                                    timer_data.SetCzas += 900000UL;
                                    if(timer_data.SetCzas > 86400000UL) timer_data.SetCzas = 86400000UL;
                                    timer_data.SetCzasMin = (timer_data.SetCzas / 60000UL) % 60U;
                                    timer_data.SetCzasGodz = (timer_data.SetCzas / 3600000UL) % 24U;
                                    LOG("SetCzas is equal to: %d:%d", timer_data.SetCzasGodz, timer_data.SetCzasMin);
                                    xQueueOverwrite(xTimerQueue, &timer_data);
                                    break;
                            }
                            break;
                        case Screen::PID_cook:
                            switch(current_var){
                                case EditVar::Kp:
                                    pid_data.Kp += 0.1;
                                    if(pid_data.Kp > pid_data.Kp_max) pid_data.Kp = 0.0;
                                    LOG("Kp is equal to: %f", pid_data.Kp);
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                                case EditVar::Ki:
                                    pid_data.Ki += 0.01;
                                    if(pid_data.Ki > pid_data.Ki_max) pid_data.Ki = 0.0;
                                    LOG("Ki is equal to: %f", pid_data.Ki);
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                                case EditVar::Kd:
                                    pid_data.Kd += 0.01;
                                    if(pid_data.Kd > pid_data.Kd_max) pid_data.Kd = 0.0;
                                    LOG("Kd is equal to: %f", pid_data.Kd);
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                            }
                        break;
                    }
                }
                else if (active_held_pin == Button::Decrease) {
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
                                    if (timer_data.SetCzas >= 900000UL) timer_data.SetCzas -= 900000UL;
                                    else timer_data.SetCzas = 0;
                                    timer_data.SetCzasMin = (timer_data.SetCzas / 60000UL) % 60U;
                                    timer_data.SetCzasGodz = (timer_data.SetCzas / 3600000UL) % 24U;
                                    LOG("SetCzas is equal to: %d:%d", timer_data.SetCzasGodz, timer_data.SetCzasMin);
                                    xQueueOverwrite(xTimerQueue, &timer_data);
                                    break;
                            }
                            break;
                        case Screen::PID_cook:
                            switch(current_var){
                                case EditVar::Kp:
                                    pid_data.Kp -= 0.1;
                                    if(pid_data.Kp < 0) pid_data.Kp = 0.0;
                                    LOG("Kp is equal to: %f", pid_data.Kp);
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                                case EditVar::Ki:
                                    pid_data.Ki -= 0.01;
                                    if(pid_data.Ki < 0) pid_data.Ki = 0.0;
                                    LOG("Ki is equal to: %f", pid_data.Ki);
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                                case EditVar::Kd:
                                    pid_data.Kd -= 0.01;
                                    if(pid_data.Kd < 0) pid_data.Kd = 0.0;
                                    LOG("Kd is equal to: %f", pid_data.Kd);
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                                    xQueueOverwrite(xSetpointQueue, &pid_data);
                                    break;
                            }
                        break;
                    }
                }
            }
        }
       } 
    }
  }
 }
}


