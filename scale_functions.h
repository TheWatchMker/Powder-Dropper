#ifndef SCALE_FUNCTIONS_H
#define SCALE_FUNCTIONS_H

#include <Arduino.h>
#include "system_state.h"

// FX-120i Scale Communication Settings
#define SCALE_BAUD 2400        // FX-120i uses 2400 baud by default
#define SCALE_DATA_BITS SERIAL_7E1  // 7 data bits, even parity, 1 stop bit
#define SCALE_TIMEOUT 1000     // 1 second timeout
#define SCALE_SETTLE_TIME 1500 // 1.5 seconds for settling
#define NUM_STABILITY_SAMPLES 5 // Number of samples for stability check

// Scale Error Codes
#define SCALE_ERROR 1
#define SCALE_COMMUNICATION_ERROR 2
#define SCALE_CALIBRATION_ERROR 3
#define SCALE_TIMEOUT_ERROR 4

// FX-120i Commands
extern const char* CMD_ZERO;    // Zero command
extern const char* CMD_PRINT;   // Request weight data
extern const char* CMD_UNITS;   // Toggle units (g/mg)
extern const char* CMD_POWER;   // Power On/Off
extern const char* CMD_TARE;    // Tare command
extern const char* CMD_CAL;     // Enter calibration mode

// Scale calibration structure
struct ScaleCalibration {
    float knownWeight;         // Known calibration weight in grams
    float measuredWeight;      // Raw measured weight during calibration
    float calibrationFactor;   // Calibration factor (known/measured)
    bool isCalibrated;        // Whether scale is calibrated
    uint32_t lastCalibration; // Timestamp of last calibration
    
    ScaleCalibration() : 
        knownWeight(0.0),
        measuredWeight(0.0),
        calibrationFactor(1.0),
        isCalibrated(false),
        lastCalibration(0) {}
};

// Scale status structure
struct ScaleStatus {
    bool isConnected;
    bool isStable;
    float currentWeight;
    float lastStableWeight;
    uint32_t lastUpdate;
    uint32_t lastStableReading;
    
    ScaleStatus() :
        isConnected(false),
        isStable(false),
        currentWeight(0.0),
        lastStableWeight(0.0),
        lastUpdate(0),
        lastStableReading(0) {}
};

extern ScaleCalibration scaleCal;
extern ScaleStatus scaleStatus;

// Core scale functions
bool initializeScale();
void readScale();
float readScaleWeight();
void processScaleData(const char* data);
bool zeroScale();
bool calibrateScale(float knownWeight);
bool isWeightStable();
bool checkScaleCommunication();
void saveCalibration();
void loadCalibration();
float getWeight();
float getLastStableWeight();
bool isScaleConnected();
bool isScaleStable();

#endif // SCALE_FUNCTIONS_H 