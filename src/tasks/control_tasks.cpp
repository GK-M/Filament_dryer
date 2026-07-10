#include <Arduino.h>
#include <PID_v1.h>
#include <Preferences.h>


#include "tasks/control_task.h"
#include "rtos_handles.h"
#include "config.h"
#include "tasks/log_task.h"

void vControlTask(void *pvParameters) {

    I2C_sensors i2c_sensors;
    DS_sensors ds_sensors;

    PID_data pid_data;    
    Control_status control_status;
    Preferences preferences;

    preferences.begin("Kp", false);
    pid_data.Kp = preferences.getFloat("Kp", pid_data.Kp);
    preferences.end();
    
    preferences.begin("Ki", false);
    pid_data.Ki = preferences.getFloat("Ki", pid_data.Ki);
    preferences.end();

    preferences.begin("Kd", false);
    pid_data.Kd = preferences.getFloat("Kd", pid_data.Kd);
    preferences.end();

    PID pid(&pid_data.Input, &pid_data.Output, &pid_data.Setpoint, pid_data.Kp, pid_data.Ki, pid_data.Kd, DIRECT);
    
    pid.SetOutputLimits(0, 1023);  
    pid.SetSampleTime(pid_data.SetSampleTime);       
    pid.SetMode(AUTOMATIC);

    ledcSetup(0, pid_data.freq, pid_data.rozdzielczosc);// kanał, freq, rozdzielczość
    ledcAttachPin(Pin::COOK_PWM, 0); // pin, kanał


    for (;;) {

        xQueuePeek(xSetpointQueue, &pid_data, pdMS_TO_TICKS(250)); 
        
        // Dane z czujników I2C      
        if(!xQueuePeek(xI2CsensorsQueue,&i2c_sensors, pdMS_TO_TICKS(Timing::HOLD_MS))){
            LOG("Bład przesłania danych z kolejki xI2CsensorsQueue do ControlTask");
        }
        
        // Dane z one-Wire (DS18B20)
        if(!xQueuePeek(xDS18B20Queue,&ds_sensors, pdMS_TO_TICKS(Timing::HOLD_MS))){
            LOG("Bład przesłania danych z kolejki xDS18B20Queue do ControlTask");
        }

        /* Zmina w trakcie działania suszarki */
        pid.SetTunings(pid_data.Kp, pid_data.Ki, pid_data.Kd);
        pid_data.Input = i2c_sensors.temp_aht;

        pid.Compute();
        ledcWrite(0, pid_data.Output); // kanał 0 jest przypisany do pinu COOK_PWM
        //LOG("Output is equail: %.1f",control_status.Output);
        pid_data.Output = map(pid_data.Output, 0, 1023, 0, 100);


        if(pid_data.Output > 0) control_status.active = true;
        else control_status.active = false;

        //Zabezpiecznie przed zbyt wysoką temp. na matach
        if(ds_sensors.DS_1 > Calibration::Max_DS_temperature ||
            ds_sensors.DS_2 > Calibration::Max_DS_temperature ||
            ds_sensors.DS_3 > Calibration::Max_DS_temperature) {
            pid_data.Setpoint = 0.0;
            ledcWrite(0,0);
            pid_data.error = true;
            LOG("Zabezpieczenie przed zbyt dużą temperaturą aktywowane pid_data.error = %d",pid_data.error);
        }else if (ds_sensors.DS_1 < Calibration::Temp_after_error &&
                  ds_sensors.DS_2 < Calibration::Temp_after_error &&
                  ds_sensors.DS_3 < Calibration::Temp_after_error && pid_data.error == true)
        {
            pid_data.error = false;
            LOG("Mata wystygła do zadanej temperatury %.1f pid_data.error = %d",Calibration::Temp_after_error,pid_data.error);
        }
        
        control_status.Output = pid_data.Output;
        LOG("Output is equail: %.1f",control_status.Output);
        control_status.Setpoint = pid_data.Setpoint;

        //Wysyłane dane z PID
        xQueueOverwrite(xControlDataQueue,&control_status);

        vTaskDelay(pdMS_TO_TICKS(Timing::PID_COMPUTING));
       
    }
}

