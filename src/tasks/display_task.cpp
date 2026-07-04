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

        if(!xQueuePeek(xI2CsensorsQueue,&i2c_sensors, pdMS_TO_TICKS(100))){
            LOG("Bład przesłaia danych z kolejki xI2CsensorsQueue");
        }
        if(!xQueuePeek(xDS18B20Queue,&ds_sensors, pdMS_TO_TICKS(100))){
            LOG("Bład przesłaia danych z kolejki xDS18B20Queue");
        }
        if(!xQueuePeek(xControlDataQueue,&control_status, pdMS_TO_TICKS(100))){
            LOG("Bład przesłaia danych z kolejki xControlDataQueue");
        }
    
    
    
    
    
    
    
    
        vTaskDelay(pdMS_TO_TICKS(200));
    }

    
}
