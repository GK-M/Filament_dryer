# Filament Dryer 


<p align="center">
  <a href="photo.jpg">
    <img src="photo.jpg" alt="Filament Dryer" width="200">
  </a><br>
  <sub>Filament Dryer </sub>
</p>

## Key info

* **Heating:** 3× heating mats (24V), total **250W**, with radiators/heatsinks -> max temp. 70°C
* **Control:** **ESP32-C3** + MOSFET modules, **PID** control with **PWM**
* **Power rails:** 24V (mats), 12V (fan + LEDs), 5V (ESP32) via DC-DC step-down converters
* **Sensors:** DS18B20 1-wire sensors attached to the mats (over-temp safety shutdown) + chamber temperature BMP280 & humidity AHT10 sensor (used for control)
* **UI:** LCD **16×4** + buttons for setpoint/status
* **Drying timer:** built-in timer to set the **drying duration**
* **Fan control:** fan PWM adjusted based on humidity inside the dryer

## Operation

* Set **drying temperature** and **drying time** (timer)
* Live preview of all key values on the LCD:
  * readings from **all sensors**
  * **PWM duty cycle** (heater/fan)
  * current system status (heating/holding/safety state)

## Planned features

* Communication with a server via **MQTT**
* Multiple **operating modes** (presets / profiles)
* **Remote programming / updates** for the ESP32 (OTA)

## Components

* **ESP32-C3**
* **3× MOSFET module**
* **3× step-down converter**
* **3× DS18B20**
* **1× AHT10**
* **1× BMP280**
* **LCD 16×4**
* **Buttons** (menu) + **ON/OFF switch**
* **Prototype board**
* **LED strip**
* **Wires**
* **Power supply** (250W)
* **Heating mats**
  * **2× 100W / 24V**
  * **1× 50W / 24V**
* XT60 connector
* Fan 120mm 
* 3× radiator/heatsink 
* 1kg PETG
* Plexi 300x300


## Safety
If any heating mat temperature exceeds the defined limit, the heater is turned off automatically.



