#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>


// Uchwyty tasków
extern TaskHandle_t xTempSensorTaskHandle;
extern TaskHandle_t xHumTempSensorTask;
extern TaskHandle_t xControlTaskHandle;
extern TaskHandle_t xDisplayTaskHandle;
extern TaskHandle_t xButtonTaskHandle;
extern TaskHandle_t xLogTaskHandle;
extern TaskHandle_t xLedTaskHandle;
extern TaskHandle_t xFanTaskHandle;

// Kolejki do komunikacji między taskami
extern QueueHandle_t xDS18B20Queue;
extern QueueHandle_t xI2CsensorsQueue;
extern QueueHandle_t xSetpointQueue;
extern QueueHandle_t xLogQueue;
extern QueueHandle_t xControlDataQueue;
extern QueueHandle_t xButtonQueue;
extern QueueHandle_t xTimerQueue;


    struct I2C_sensors{
        float temp_aht = 0.0f;
        float hum_aht = 0.0f;
        float t_bmp = 0.0f;
        float p_hPa = 0.0f;
    };
    
    struct DS_sensors{
        float DS_1 = 0.0f;
        float DS_2 = 0.0f;
        float DS_3 = 0.0f;
        bool error = false;
    };
    


    struct Control_status
    {
        float Output = 0.0;
        float Setpoint = 0.0;
        bool active = false;
    };
    
    // Taski Button 

    enum class ButtonEdge {PRESSED, RELEASED};

    struct ButtonRAW{
        uint8_t pin= 0;
        ButtonEdge edge;
        uint32_t timestamp;
    };


    enum class ButtonAction {
        INCREASE,
        DECREASE,
        MODE,
        ENTER,
        PRZYCISK_EKRAN // potwierdzenie, zmiana co edytowane
    };

    enum class Screen {
        Main,
        Sensors_data,
        PID_cook,
        PID_fan,
        COUNT 
    };

    enum class EditVar {
        Setpoint,
        Time,
        Kp,
        Ki,
        Kd,
    };

    enum class Mode {
        Mode_1,
        Mode_2,
        Mode_3,
        Mode_4,
        COUNT,
    };
