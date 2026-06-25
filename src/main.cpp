#include <Arduino.h>
#include <WiFi.h>


#include "config.h"
#include "secrets.h"

// Ctrl + Shift + Space <- Podpowiedz


// put function declarations here:
int myFunction(int, int);

void setup() {

WiFi.begin()
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}
