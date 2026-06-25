#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void vTempSensorTask(void *pvParameters);
void vHumTempSensorTask(void *pvParameters);
