#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Wire.h>


#include "config.h"
#include "secrets.h"
#include "rtos_handles.h"
#include "tasks/sensor_task.h"
#include "tasks/control_task.h"
#include "tasks/display_task.h"
#include "tasks/button_task.h"
#include "tasks/log_task.h"
#include "tasks/led_task.h" 
#include "tasks/fan_task.h"
#include "secrets.h"


TaskHandle_t xTempSensorTaskHandle  = NULL;
TaskHandle_t xControlTaskHandle = NULL;
TaskHandle_t xDisplayTaskHandle = NULL;
TaskHandle_t xButtonTaskHandle  = NULL;
TaskHandle_t xLogTaskHandle     = NULL;
TaskHandle_t xHumTempSensorTask = NULL;
TaskHandle_t xLedTaskHandle     = NULL;
TaskHandle_t xFanTaskHandle     = NULL;
TaskHandle_t xOTAUpdateTaskHandle = NULL;


SemaphoreHandle_t xI2CMutex = NULL;

QueueHandle_t xI2CsensorsQueue  = NULL;
QueueHandle_t xDS18B20Queue     = NULL;
QueueHandle_t xSetpointQueue    = NULL;
QueueHandle_t xLogQueue         = NULL;
QueueHandle_t xControlDataQueue = NULL;
QueueHandle_t xButtonQueue      = NULL;
QueueHandle_t xTimerQueue = NULL;
QueueHandle_t xDisplayQueue = NULL;


void vOtaTask(void* pvParameters) {
    for(;;) {
        ArduinoOTA.handle();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void setup() {
    Serial.begin(115200);
    Wire.begin(I2C::SDA, I2C::SCL, I2C::frequency);
    

    IPAddress local_IP(192, 168, 1, 15);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.config(local_IP, gateway, subnet);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) delay(500);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    //Metoda od ArduinoOTA do obsługi OTA (Over-the-Air) aktualizacji
    ArduinoOTA.setHostname("Filament Dryer");
    ArduinoOTA.begin();

    

    xI2CMutex         = xSemaphoreCreateMutex();

    xDS18B20Queue     = xQueueCreate(1,   sizeof(DS_sensors));
    xI2CsensorsQueue  = xQueueCreate(1,   sizeof(I2C_sensors));
    xSetpointQueue    = xQueueCreate(1,   sizeof(PID_data));
    xControlDataQueue = xQueueCreate(1,   sizeof(Control_status));
    xLogQueue         = xQueueCreate(10,    LOG_MSG_LEN);
    xButtonQueue      = xQueueCreate(10,   sizeof(ButtonRAW));
    xTimerQueue       = xQueueCreate(1, sizeof(Timer_data));
    xDisplayQueue     = xQueueCreate(1, sizeof(Display_data));
  
    xTaskCreate(vOtaTask, "OTA", 2048, NULL, 20, &xOTAUpdateTaskHandle);
    xTaskCreate(vLogTask,     "Log",     2048, NULL, 11, &xLogTaskHandle);
    xTaskCreate(vTempSensorTask,  "Sensor",  2048, NULL, 6, &xTempSensorTaskHandle);
    xTaskCreate(vHumTempSensorTask, "BMP280", 4096, NULL, 5, &xHumTempSensorTask);
    xTaskCreate(vControlTask, "Control", 4096, NULL, 8, &xControlTaskHandle);
    xTaskCreate(vDisplayTask, "Display", 8192, NULL, 15, &xDisplayTaskHandle);
    xTaskCreate(vButtonTask,  "Button",  8192, NULL, 10, &xButtonTaskHandle);
    xTaskCreate(vLedTask,     "Led",     1024, NULL, 2, &xLedTaskHandle);
    xTaskCreate(vFanTask,     "Fan",     1024, NULL, 3, &xFanTaskHandle);

}

void loop() {
    vTaskDelete(NULL);
}
