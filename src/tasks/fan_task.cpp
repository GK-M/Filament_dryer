#include <Arduino.h>
#include "config.h"
#include "tasks/log_task.h"
#include "tasks/sensor_task.h"
#include "rtos_handles.h"



void vFanTask(void *pvParameters){
    pinMode(Pin::FAN_PWM,OUTPUT);
    ledcAttachPin(Pin::FAN_PWM,2);
    ledcChangeFrequency(2,Led_data::freq, Led_data::rozdzielczosc);
    ledcWrite(2,100);

    I2C_sensors i2c;

    for(;;){

        xQueuePeek(xI2CsensorsQueue, &i2c,pdMS_TO_TICKS(Timing::HOLD_MS));
        if (i2c.hum_aht > 20) ledcWrite(2, 1023);
        if (i2c.hum_aht <=20) ledcWrite(2,800);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}