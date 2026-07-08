#include <OneWire.h>
#include <DallasTemperature.h>

#define PIN_ONEWIRE 3

OneWire oneWire(PIN_ONEWIRE);
DallasTemperature sensors(&oneWire);

DeviceAddress addr; 

void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(PIN_ONEWIRE, INPUT_PULLUP);

  sensors.begin();

  uint8_t count = sensors.getDeviceCount();
  Serial.print("Znaleziono czujnikow: ");
  Serial.println(count);

  for (uint8_t i = 0; i < count; i++) {
    if (sensors.getAddress(addr, i)) {
      Serial.print("Czujnik ");
      Serial.print(i);
      Serial.print(": ");
      printAddress(addr);
    } else {
      Serial.print("BŁąd ");
      Serial.println(i);
    }
  }
}

void loop() {
}
