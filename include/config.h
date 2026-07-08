#ifndef CONFIG_H
#define CONFIG_H


// PIN SETTINGS
namespace Pin {
    constexpr uint8_t ONE_WIRE_BUS = 3;
    constexpr uint8_t FAN_PWM = 4;
    constexpr uint8_t COOK_PWM  = 2;
    constexpr uint8_t LedPin = 7;  
}
namespace Button{
    constexpr uint8_t PrzyciskEkran = 10; //Long press turn on/off screen
    constexpr uint8_t Increase = 21;
    constexpr uint8_t Decrease = 20;
    constexpr uint8_t Mode = 5; //Change cooking mode
    constexpr uint8_t Enter = 1; 
}
namespace I2C {
    constexpr uint8_t SDA = 8;
    constexpr uint8_t SCL = 9;
    constexpr uint32_t frequency = 400000UL;
    constexpr uint8_t LCD = 0x27; 
}

//Rózne zmienne od czasów
namespace Timing {
    constexpr uint32_t DEBOUNCE_MS    = 115;
    constexpr uint32_t HOLD_MS        = 500;
    constexpr uint32_t SENSOR_READ_MS = 1000;
    constexpr uint32_t SENSOR_I2C_READ_MS = 500;
    constexpr uint32_t PID_COMPUTING   = 1000;
    constexpr uint32_t Display_data_timeout = 200;
    constexpr uint32_t Repeat_button_time = 400;
}

//Kalibracja czujników
namespace Calibration {
    constexpr float DS_1 = 0.0f;
    constexpr float DS_2 = 0.0f;
    constexpr float DS_3 = 0.0f;
    constexpr float AHT10 = 0.0f;
    constexpr float BMP280 = 0.0f;
}

//DS18B20 adresses
namespace adrr_DS{
    constexpr uint8_t addr1[8] = { 0x28, 0x46, 0xC3, 0xBC, 0x00, 0x00, 0x00, 0x36 };
    constexpr uint8_t addr2[8] = { 0x28, 0x7F, 0xC0, 0xBC, 0x00, 0x00, 0x00, 0x03 };\
    //Temp podzespołów 
    constexpr uint8_t addr3[8] = { 0x28, 0x27, 0xD6, 0xBC, 0x00, 0x00, 0x00, 0x6C };
};

// Nastawy PID
struct PID_data{
    double Setpoint = 0;  // zadana 
    double Input = 0;     // pomiar
    double Output = 0.0;  // wyjście PID
    int16_t SetSampleTime = 1000;
    int16_t freq = 1000;
    float Kp = 2.25, Ki = 0.05, Kd = 0.0;
    uint8_t rozdzielczosc = 10;
    float Kp_max = 20.0, Ki_max = 10.0, Kd_max = 5.0;
};  

//Zmienne do Timera
struct Timer_data{

    uint32_t SetCzas = 0;

    uint32_t SetCzasMin = 0;   
    uint32_t SetCzasGodz = 0;

    uint32_t StoperCzasSek = 0;
    uint32_t StoperCzasMin = 0;
    uint32_t StoperCzasGodz = 0;
};

// LCD icons
inline uint8_t Stopnie[] = {
  0b11000,
  0b11000,
  0b00111,
  0b01000,
  0b01000,
  0b01000,
  0b01000,
  0b00111
};

inline uint8_t Dzwonek[] = {
  0b01110,
  0b01010,
  0b01010,
  0b11111,
  0b11111,
  0b00000,
  0b00100,
  0b00000
};

#endif