#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "tasks/display_task.h"
#include "rtos_handles.h"
#include "config.h"
#include "tasks/log_task.h"


void vDisplayTask(void *pvParameters) {
 
I2C_sensors i2c_sensors;
DS_sensors ds_sensors;
Control_status control_status;
Timer_data timer_data;
Display_data display_data;

LiquidCrystal_I2C lcd(0x27, 20, 4);

lcd.init();       // Włącz ekran
lcd.backlight();  // Ekran ON

/*
        lcd.setCursor(0, 0);
        lcd.printf("Tryb: %d      ", tryb);
        lcd.setCursor(0, 1);
        lcd.print("      DRYING          ");
        Serial.printf("TRYB: %d\n", tryb);
*/

    for (;;) {

        if(!xQueuePeek(xI2CsensorsQueue,&i2c_sensors, pdMS_TO_TICKS(Timing::Display_data_timeout))){
            LOG("Bład przesłania danych z kolejki xI2CsensorsQueue do DisplayTask");
        }
        if(!xQueuePeek(xDS18B20Queue,&ds_sensors, pdMS_TO_TICKS(Timing::Display_data_timeout))){
            LOG("Bład przesłania danych z kolejki xDS18B20Queue do DisplayTask");
        }
        if(!xQueuePeek(xControlDataQueue,&control_status, pdMS_TO_TICKS(Timing::Display_data_timeout))){
            LOG("Bład przesłania danych z kolejki xControlDataQueue do DisplayTask");
        }
        if(!xQueuePeek(xButtonQueue,&b, pdMS_TO_TICKS(Timing::Display_data_timeout))){
            LOG("Bład przesłania danych z kolejki xControlDataQueue do DisplayTask");
        }
        if(!xQueuePeek(xTimerQueue,&timer_data, pdMS_TO_TICKS(Timing::Display_data_timeout))){
            LOG("Bład przesłania danych z kolejki xTimerQueue  do DisplayTask");
        }
        if(!xQueuePeek(xDisplayQueue,&display_data, pdMS_TO_TICKS(Timing::Display_data_timeout))){
            LOG("Bład przesłania danych z kolejki xDisplayQueu do DisplayTask");
        }
        
        switch (display_data.screen){
            
        

        case Screen::Main:

        lcd.setCursor(0,0);
    
    
        }
    
    
    
    
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    
}
