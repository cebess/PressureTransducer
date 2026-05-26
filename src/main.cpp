#include <Arduino.h>
#include "PressureTransducer.h"
#define ADC_PIN 34

#ifndef UNIT_TEST
//define the my_transducer pointer to avoid requiring a default constructor at global scope
static PressureTransducer* my_transducer = nullptr;

void setup() {
  Serial.begin(115200);
  my_transducer = new PressureTransducer(ADC_PIN);
  my_transducer->calibrate(); // calibrate the sensor to find the baseline offset
  float pressure = my_transducer->readPressureCorrected();
  Serial.print("First Corrected Pressure Reading: ");
  Serial.println(pressure);
}

void loop() {
  float pressure = my_transducer->readPressureCorrected();
  Serial.print("Corrected Pressure: ");
  Serial.println(pressure);
  delay(1000); // wait between measurements
}
#endif // UNIT_TEST