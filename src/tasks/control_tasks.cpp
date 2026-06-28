#include <Arduino.h>
#include <PID_v1.h>

#include "tasks/control_task.h"
#include "rtos_handles.h"
#include "config.h"
#include "tasks/log_task.h"

void vControlTask(void *pvParameters) {

    I2C_sensors i2c_sensors;
    DS_sensors ds_sensors;

    PID_data pid_data;    
    Control_status control_status;

    PID pid(&pid_data.Input, &pid_data.Output, &pid_data.Setpoint, pid_data.Kp, pid_data.Ki, pid_data.Kd, DIRECT);
    
    pid.SetOutputLimits(0, 1023);  // WYJŚCIE 0..100
    pid.SetSampleTime(pid_data.SetSampleTime);       // ms; 
    pid.SetMode(AUTOMATIC);

    ledcSetup(0, pid_data.freq, pid_data.rozdzielczosc);        // kanał, freq, rozdzielczość
    ledcAttachPin(Pin::COOK_PWM, 0); // pin, kanał


    for (;;) {

        xQueuePeek(xSetpointQueue, &pid_data.Setpoint, pdMS_TO_TICKS(250));
        
        if(!xQueuePeek(xI2CsensorsQueue,&i2c_sensors, pdMS_TO_TICKS(1000))){
            LOG("Bład przesłaia danych z kolejki xI2CsensorsQueue");
        }
        if(!xQueuePeek(xDS18B20Queue,&ds_sensors, pdMS_TO_TICKS(1000))){
            LOG("Bład przesłaia danych z kolejki xDS18B20Queue");
        }

        pid.SetTunings(pid_data.Kp, pid_data.Ki, pid_data.Kd);
        pid_data.Input = i2c_sensors.temp_aht;
        pid.Compute();
        ledcWrite(0, pid_data.Output);

        if(pid_data.Output > 0) control_status.active = true;
        else control_status.active = false;

        pid_data.Output = map(pid_data.Output, 0, 1023, 0, 100);
        control_status.Output = pid_data.Output;
        control_status.Setpoint = pid_data.Setpoint;

        //Wysyłane dane z PID
        xQueueOverwrite(xControlDataQueue,&control_status);

        vTaskDelay(pdMS_TO_TICKS(1000));
        



       
    }
}

