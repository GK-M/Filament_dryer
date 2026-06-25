#ifndef CONFIG_H
#define CONFIG_H


// PIN SETTINGS
namespace Pin {
    constexpr uint8_t ONE_WIRE_BUS = 3;
    constexpr uint8_t FAN_PWM = 4;
    constexpr uint8_t COOK_PWM  = 2;
}


namespace Button{

    constexpr uint8_t PrzyciskEkran = 10; //Long press turn on/off screen
    constexpr uint8_t Increase = 21;
    constexpr uint8_t Decrease = 20;
    constexpr uint8_t Mode= 5; //Change cooking mode
    constexpr uint8_t Enter = 1; //Subbmit changes
    constexpr uint8_t LedPin = 7;  
}


namespace I2C {

    constexpr uint8_t SDA = 8;
    constexpr uint8_t SCL = 9;
    constexpr uint8_t LCD = 0x27; 
}


// LCD icons

inline uint8_t Stopnie[8] = {
  0b11000,
  0b11000,
  0b00111,
  0b01000,
  0b01000,
  0b01000,
  0b01000,
  0b00111
};

inline uint8_t Dzwonek[8] = {
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