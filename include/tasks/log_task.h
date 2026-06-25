#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define LOG_MSG_LEN 80

void vLogTask(void *pvParameters);



// Makro do logowania
#define LOG(fmt, ...) do { \
    char _buf[LOG_MSG_LEN]; \
    snprintf(_buf, sizeof(_buf), fmt, ##__VA_ARGS__); \
    xQueueSend(xLogQueue, _buf, 0); \
} while(0)
