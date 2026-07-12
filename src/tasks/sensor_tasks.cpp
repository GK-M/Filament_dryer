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
    DS_sensors DS_sensors;

    const uint8_t*  Heater_1 = adrr_DS::addr1;
    const uint8_t*  Heater_2 = adrr_DS::addr2;
    const uint8_t*  Heater_3 = adrr_DS::addr3;

    for (;;) {

    sensors.requestTemperatures();
    //error flags
    DS_sensors.error = false;

    DS_sensors.DS_1 = sensors.getTempC(Heater_1) + Calibration::DS_1;
    DS_sensors.DS_2 = sensors.getTempC(Heater_2) + Calibration::DS_2;
    DS_sensors.DS_3 = sensors.getTempC(Heater_3) + Calibration::DS_3;

    if(DS_sensors.DS_1 == -127.0){
        LOG("Heater_1: brak czujnika / błąd czujnika");
        DS_sensors.error = true;
    }
    if (DS_sensors.DS_2 == -127.0){
        LOG("Heater_2: brak czujnika / błąd czujnika");
        DS_sensors.error = true;
    }
    if (DS_sensors.DS_3 == -127.0){
        LOG("Heater_3: brak czujnika / błąd czujnika");
        DS_sensors.error = true;
    }

    LOG("Temp DS_1: %.1f", DS_sensors.DS_1 );
    LOG("Temp DS_2: %.1f", DS_sensors.DS_2 );
    LOG("Temp DS_3: %.1f", DS_sensors.DS_3 );

    xQueueOverwrite(xDS18B20Queue, &DS_sensors);
    vTaskDelay(pdMS_TO_TICKS(Timing::SENSOR_READ_MS));

    }
}

void vHumTempSensorTask(void *pvParameters){

    //Wire.begin(I2C::SDA, I2C::SCL, I2C::frequency);
    I2C_sensors I2C_sensors;

    //AHT10
    Adafruit_AHTX0 aht;
    sensors_event_t hum, temp;
    if(!aht.begin()){
        LOG("Bład inicjalizacji czujnika wilgotności");
    }
    

    //BMP280
    Adafruit_BMP280 bmp;

    if(!bmp.begin()){
        LOG("Bład inicjalizacji czujnika bmp280");
    }
    bmp.setSampling(Adafruit_BMP280::MODE_FORCED,
                Adafruit_BMP280::SAMPLING_X1,  // temp
                Adafruit_BMP280::SAMPLING_X1,  // pressure
                Adafruit_BMP280::FILTER_OFF);  
    for(;;){

    xSemaphoreTake(xI2CMutex, portMAX_DELAY);
    bmp.takeForcedMeasurement();
    aht.getEvent(&hum, &temp);
    I2C_sensors.temp_aht = temp.temperature + Calibration::AHT10;
    I2C_sensors.hum_aht = hum.relative_humidity;
    I2C_sensors.t_bmp = bmp.readTemperature() + Calibration::BMP280;
    I2C_sensors.p_hPa = bmp.readPressure() / 100.0f;
    xSemaphoreGive(xI2CMutex);

    LOG("Data from AHT10: %.1f%% %.1f C ",I2C_sensors.temp_aht, I2C_sensors.hum_aht);
    LOG("Data from BMP280: %.1f C %.1f hPa ",I2C_sensors.t_bmp, I2C_sensors.p_hPa);
    xQueueOverwrite(xI2CsensorsQueue, &I2C_sensors);

    vTaskDelay(pdMS_TO_TICKS(Timing::SENSOR_I2C_READ_MS));
    }
}