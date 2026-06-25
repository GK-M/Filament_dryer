#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

#include "tasks/log_task.h"
#include "tasks/sensor_task.h"
#include "rtos_handles.h"
#include "config.h"

void vTempSensorTask(void *pvParameters) {
    
    OneWire oneWire(Pin::ONE_WIRE_BUS);
    DallasTemperature sensors(&oneWire);
    sensors.begin();
    sensors.setWaitForConversion(false);

    DeviceAddress Heater_1 = {  };
    DeviceAddress Heater_2 = {  };
    DeviceAddress Heater_3 = {  };
    
    float DS_1 = 0.0f;
    float DS_2 = 0.0f;
    float DS_3 = 0.0f;

    for (;;) {

    sensors.requestTemperatures(); 
    DS_1 = sensors.getTempC(Heater_1) + Calibration::DS_1;
    DS_2 = sensors.getTempC(Heater_2) + Calibration::DS_2;
    DS_3 = sensors.getTempC(Heater_3) + Calibration::DS_3;
    LOG("Temp DS_1: %.1f", DS_1 );
    LOG("Temp DS_2: %.1f", DS_2 );
    LOG("Temp DS_3: %.1f", DS_3 ); 
    vTaskDelay(pdMS_TO_TICKS(1000));
   
    }

}

void vHumTempSensorTask(void *pvParameters){

    Wire.begin(I2C::SDA, I2C::SCL, I2C::frequency);

    //AHT10
    Adafruit_AHTX0 aht;
    sensors_event_t hum, temp;

    //BMP280
    Adafruit_BMP280 bmp;
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                Adafruit_BMP280::SAMPLING_X1,  // temp
                Adafruit_BMP280::SAMPLING_X1,  // pressure
                Adafruit_BMP280::FILTER_OFF);
                
    float temp_aht = 0.0f;
    float hum_aht = 0.0f;
    float t_bmp = 0.0f;
    float p_hPa = 0.0f;

    for(;;){

    bmp.takeForcedMeasurement();   
    aht.getEvent(&hum, &temp);

    vTaskDelay(pdMS_TO_TICKS(500));

    temp_aht = temp.temperature + Calibration::AHT10;
    hum_aht = hum.relative_humidity;

    t_bmp = bmp.readTemperature() + Calibration::BMP280;
    p_hPa = bmp.readPressure() / 100.0f;


    }
}