/*
  PressureTransducer class implementation to monitor a differential pressure transducer using an ESP32 ADC input. The class provides methods to read the current pressure, calibrate the sensor to establish a baseline offset, and return corrected pressure readings based on that calibration.

  Functions:
    - PressureTransducer(int pin):
        Initialize the transducer instance with the ADC pin used for sensor input.
        Configures the pin as an input.

    - float readPressure():
        Sample the ADC multiple times, remove the highest and lowest readings to help reduce noise
        average the remaining values, convert to sensor voltage, and return
        a pressure value in kPa relative to the transducer baseline.

    - float readPressureCorrected():
        Read the current pressure and subtract the stored baseline offset.
        Use this when a prior calibration has established the zero-pressure level.

    - void calibrate():
        Measure the current raw pressure value when there is no differential
        pressure and store it as the baseline offset for corrected readings.

    - float baselineOffset():
        Return the stored baseline offset value used for corrected pressure output.
*/

#include "PressureTransducer.h"

PressureTransducer::PressureTransducer(int pin) : adcPin(pin) {
    pinMode(adcPin, INPUT);
}

#define NUMBER_OF_SAMPLES 64

float PressureTransducer::readPressure() {
    float samples[NUMBER_OF_SAMPLES];
    float minSample = 4095.0;
    float maxSample = 0.0;
    int minIndex = 0;
    int maxIndex = 0;
    float sum = 0.0;

    for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
        float temp = analogRead(adcPin);
        samples[i] = temp;
        if (temp < minSample) {
            minSample = temp;
            minIndex = i;
        }
        if (temp > maxSample) {
            maxSample = temp;
            maxIndex = i;
        }
        yield(); // give some time to the environment between samples on the ESP32
    }

    // If the minimum and maximum values occur at the same index, choose a second
    // index so that we still remove exactly one min and one max sample.
    if (minIndex == maxIndex) {
        maxIndex = (minIndex == 0) ? 1 : 0;
    }

    for (int i = 0; i < NUMBER_OF_SAMPLES; i++) {
        if (i == minIndex || i == maxIndex) {
            continue;
        }
        sum += samples[i];
    }

    float avg = sum / (NUMBER_OF_SAMPLES - 2);
    float adcVoltage = (avg / 4095.0) * 3.3;
    float sensorVoltage = adcVoltage * 1.55;
    float pressure_kPa = (sensorVoltage - 2.5);
    return pressure_kPa;
}

float PressureTransducer::readPressureCorrected() {
    // run this when there is no differential pressure to determine the baseline offset, we can take multiple readings and average them to reduce noise
    float initalReading = this->readPressure(); // read the pressure value before offset correction
    return initalReading - this->baselineOffsetValue    ; // subtract the baseline offset from the reading to get the corrected pressure value
}

void PressureTransducer::calibrate() {
    // run this when there is no differential pressure to determine the baseline offset, we can take multiple readings and average them to reduce noise
    float baseline = this->readPressure(); // find the baseline
    this->baselineOffsetValue = baseline; // store the baseline offset for future readings
}

float PressureTransducer::baselineOffset()
{
    return this->baselineOffsetValue;
}

void PressureTransducer::setBaselineOffset(float offset) {
    this->baselineOffsetValue = offset;
}
