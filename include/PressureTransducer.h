#pragma once
#include <Arduino.h>

class PressureTransducer {
public:
    PressureTransducer(int pin);
    float readPressure();
    float readPressureCorrected();
    void calibrate();
    float baselineOffset();
    void setBaselineOffset(float offset);
private:
    int adcPin;
    float baselineOffsetValue = 0.0;
};

