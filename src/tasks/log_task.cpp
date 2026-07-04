#include <Arduino.h>
#include "tasks/log_task.h"
#include "rtos_handles.h"
#include "tasks/log_task.h"


void vLogTask(void *pvParameters) {
    char msg[LOG_MSG_LEN];

    for (;;) {
        if (xQueueReceive(xLogQueue, msg, portMAX_DELAY)) {
            Serial.println(msg);
        }
    }
}
