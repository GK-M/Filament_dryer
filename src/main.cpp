#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "secrets.h"
#include "rtos_handles.h"
#include "tasks/sensor_task.h"
#include "tasks/control_task.h"
#include "tasks/display_task.h"
#include "tasks/button_task.h"
#include "tasks/log_task.h"


TaskHandle_t xTempSensorTaskHandle  = NULL;
TaskHandle_t xControlTaskHandle = NULL;
TaskHandle_t xDisplayTaskHandle = NULL;
TaskHandle_t xButtonTaskHandle  = NULL;
TaskHandle_t xLogTaskHandle     = NULL;
TaskHandle_t xHumTempSensorTask = NULL;

QueueHandle_t xI2CsensorsQueue  = NULL;
QueueHandle_t xDS18B20Queue     = NULL;
QueueHandle_t xSetpointQueue    = NULL;
QueueHandle_t xLogQueue         = NULL;
QueueHandle_t xControlDataQueue = NULL;
QueueHandle_t xButtonQueue      = NULL;



void setup() {
    Serial.begin(115200);

    xDS18B20Queue     = xQueueCreate(1,   sizeof(DS_sensors));
    xI2CsensorsQueue  = xQueueCreate(1,   sizeof(I2C_sensors));
    xSetpointQueue    = xQueueCreate(1,   sizeof(PID_data));
    xControlDataQueue = xQueueCreate(1,   sizeof(Control_status));
    xLogQueue         = xQueueCreate(10,    LOG_MSG_LEN);
    xButtonQueue      = xQueueCreate(10,   sizeof(ButtonRAW));
  

    xTaskCreate(vLogTask,     "Log",     2048, NULL, 1, &xLogTaskHandle);
    xTaskCreate(vTempSensorTask,  "Sensor",  2048, NULL, 3, &xTempSensorTaskHandle);
    xTaskCreate(vHumTempSensorTask, "BMP280", 2048, NULL, 5, &xHumTempSensorTask);
    xTaskCreate(vControlTask, "Control", 2048, NULL, 3, &xControlTaskHandle);
    xTaskCreate(vDisplayTask, "Display", 3072, NULL, 2, &xDisplayTaskHandle);
    xTaskCreate(vButtonTask,  "Button",  1024, NULL, 4, &xButtonTaskHandle);

}

void loop() {
    vTaskDelete(NULL);
}
