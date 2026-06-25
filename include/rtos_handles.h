#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>


// Uchwyty tasków
extern TaskHandle_t xSensorTaskHandle;
extern TaskHandle_t xControlTaskHandle;
extern TaskHandle_t xDisplayTaskHandle;
extern TaskHandle_t xButtonTaskHandle;

// Kolejki do komunikacji między taskami
extern QueueHandle_t xTemperatureQueue;  
extern QueueHandle_t xSetpointQueue;     
extern QueueHandle_t xLogQueue;          