#include <Arduino.h>
#include <unity.h>
#include "PressureTransducer.h"
#define ADC_PIN 34
static PressureTransducer* my_transducer = nullptr;
//The sensor is designed with a 2.5V baseline offset. If it is powered by a standard 5V supply, the returned voltage ranges between 0.5V and 4.5V: -2kPa should outputs approximately 0.5V, 0kPa when there is no pressure differential and approximately 2.5V when there is a +2 kPa pressure differential: Outputs approximately 4.5V.

void test_histogram() {
 //take 1000 readings and count how many fall into each 10% range
    const int numSamples = 1000;
    float readings[numSamples];
    float minVal = 4095.0;
    float maxVal = 0.0;

    // Collect actual data points to determine the spread
    for (int i = 0; i < numSamples; i++) {
        readings[i] = (float)analogRead(ADC_PIN);
        if (readings[i] < minVal) minVal = readings[i];
        if (readings[i] > maxVal) maxVal = readings[i];
        yield(); 
    }

    int histogram[10] = {0};
    float range = maxVal - minVal;
    if (range < 1.0) range = 1.0; // Avoid division by zero if the signal is perfectly flat

    for (int i = 0; i < numSamples; i++) {
        int index = (int)((readings[i] - minVal) / range * 10.0);
        if (index > 9) index = 9;
        histogram[index]++;
    }

    //print the histogram
    Serial.print("Histogram of collected values (ADC Range: ");
    Serial.print(minVal);
    Serial.print(" to ");
    Serial.print(maxVal);
    Serial.println("):");

    for (int i = 0; i < 10; i++) {
        float binStartRaw = minVal + (range * i / 10.0);
        float binEndRaw = minVal + (range * (i + 1) / 10.0);
        float lowKpa = ((binStartRaw / 4095.0) * 3.3 * 1.55) - 2.5;
        float highKpa = ((binEndRaw / 4095.0) * 3.3 * 1.55) - 2.5;

        Serial.print("Bin ");
        Serial.print(i);
        Serial.print(": [");
        Serial.print(lowKpa, 3);
        Serial.print(" to ");
        Serial.print(highKpa, 3);
        Serial.print(" kPa]: ");
        Serial.println(histogram[i]);
    }
    Serial.println("Review the histogram output above to analyze noise distribution.");
}

void test_pressure_transducer() {
    // Test the pressure transducer functionality
    float pressure = my_transducer->readPressureCorrected();
    Serial.print("First Corrected Pressure Reading: ");
    Serial.println(pressure);
    TEST_ASSERT_TRUE(pressure > -0.1 and pressure < 0.1); // it should be close to 0 kPa since there is no pressure differential, we are performing multiple readings and throwing out the high and low values and taking the average to reduce noise, so we can allow for a very small margin of error
}

void test_calibrate() {
    float baseline = my_transducer->baselineOffset();
    if (baseline != 0.0) {
        String failMsg = "Initial baseline offset is " + String(baseline) + ", before calibration testing. This may indicate that the baseline offset is not being initialized correctly.";
        TEST_FAIL_MESSAGE(failMsg.c_str());
        return;
    }   

    float rawPressure = my_transducer->readPressure();
    Serial.print("Raw Pressure Before Calibration: ");
    Serial.println(rawPressure);

    my_transducer->calibrate();
    Serial.println("Baseline after calibration: " + String(my_transducer->baselineOffset()));
    if (my_transducer->baselineOffset() == 0.0) {
        TEST_FAIL_MESSAGE("Baseline offset is zero after calibration. This may indicate that the baseline offset is not being updated correctly.");
        return;
    }   

}

void test_set_baseline_offset() {
    float testOffset = 0.5f;
    my_transducer->setBaselineOffset(testOffset);
    TEST_ASSERT_EQUAL_FLOAT_MESSAGE(testOffset, my_transducer->baselineOffset(), "The baseline offset should match the value just set.");
}

void setup() {
    delay(2000); // allow serial to connect
    Serial.begin(115200);
    my_transducer = new PressureTransducer(ADC_PIN);
    UNITY_BEGIN();
  
    RUN_TEST(test_calibrate);
    RUN_TEST(test_pressure_transducer);
    RUN_TEST(test_histogram);
    RUN_TEST(test_set_baseline_offset);
    UNITY_END();
}
void loop() {
    // not used
}