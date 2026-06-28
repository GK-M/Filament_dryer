#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

#include "tasks/display_task.h"
#include "rtos_handles.h"
#include "config.h"
#include "tasks/log_task.h"


void vDisplayTask(void *pvParameters) {
 
I2C_sensors i2c_sensors;
DS_sensors ds_sensors;

    for (;;) {
        if(!xQueuePeek(xI2CsensorsQueue,&i2c_sensors, pdMS_TO_TICKS(1000))){
            LOG("Bład przesłaia danych z kolejki xI2CsensorsQueue");
        }
        if(!xQueuePeek(xDS18B20Queue,&ds_sensors, pdMS_TO_TICKS(1000))){
            LOG("Bład przesłaia danych z kolejki xDS18B20Queue");
        }
    }

    
}
