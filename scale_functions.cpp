#include "scale_functions.h"
#include <SD.h>

// Define EEPROM address for scale calibration
#define SCALE_CAL_EEPROM_ADDR 0x100  // Starting address for scale calibration data

// FX-120i Commands
const char* CMD_ZERO = "Z\r\n";    // Zero command
const char* CMD_PRINT = "Q\r\n";   // Request weight data
const char* CMD_UNITS = "U\r\n";   // Toggle units (g/mg)
const char* CMD_POWER = "P\r\n";   // Power On/Off
const char* CMD_TARE = "T\r\n";    // Tare command
const char* CMD_CAL = "C\r\n";     // Enter calibration mode

// Global variables
ScaleCalibration scaleCal;
ScaleStatus scaleStatus;

// Core scale functions
bool initializeScale() {
    // Initialize serial communication
    Serial2.begin(SCALE_BAUD, SCALE_DATA_BITS);
    
    // Wait for scale to initialize
    delay(1000);
    
    // Check communication
    if (!checkScaleCommunication()) {
        return false;
    }
    
    // Zero scale
    if (!zeroScale()) {
        return false;
    }
    
    // Load calibration
    loadCalibration();
    
    scaleStatus.isConnected = true;
    return true;
}

void readScale() {
    if (!scaleStatus.isConnected) {
        return;
    }
    
    // Send print command
    Serial2.print(CMD_PRINT);
    
    // Wait for response
    unsigned long startTime = millis();
    while (!Serial2.available() && millis() - startTime < SCALE_TIMEOUT) {
        delay(10);
    }
    
    if (!Serial2.available()) {
        scaleStatus.isStable = false;
        return;
    }
    
    // Read response
    String response = Serial2.readStringUntil('\n');
    
    // Process data
    processScaleData(response.c_str());
}

float readScaleWeight() {
    if (!scaleStatus.isConnected) {
        return 0.0;
    }
    
    // Send print command
    Serial2.print(CMD_PRINT);
    
    // Wait for response
    unsigned long startTime = millis();
    while (!Serial2.available() && millis() - startTime < SCALE_TIMEOUT) {
        delay(10);
    }
    
    if (!Serial2.available()) {
        return 0.0;
    }
    
    // Read response
    String response = Serial2.readStringUntil('\n');
    
    // Parse weight value
    if (response.length() > 0) {
        float weight = response.toFloat();
        if (scaleCal.isCalibrated) {
            weight *= scaleCal.calibrationFactor;
        }
        return weight;
    }
    
    return 0.0;
}

void processScaleData(const char* data) {
    if (strlen(data) == 0) {
        return;
    }
    
    // Parse weight value
    float weight = atof(data);
    
    // Apply calibration if available
    if (scaleCal.isCalibrated) {
        weight *= scaleCal.calibrationFactor;
    }
    
    // Update scale status
    scaleStatus.currentWeight = weight;
    scaleStatus.lastUpdate = millis();
    
    // Check stability
    if (isWeightStable()) {
        scaleStatus.isStable = true;
        scaleStatus.lastStableWeight = weight;
        scaleStatus.lastStableReading = millis();
    } else {
        scaleStatus.isStable = false;
    }
}

bool zeroScale() {
    if (!scaleStatus.isConnected) {
        return false;
    }
    
    // Send zero command
    Serial2.print(CMD_ZERO);
    
    // Wait for response
    unsigned long startTime = millis();
    while (!Serial2.available() && millis() - startTime < SCALE_TIMEOUT) {
        delay(10);
    }
    
    if (!Serial2.available()) {
        return false;
    }
    
    // Read response
    String response = Serial2.readStringUntil('\n');
    
    // Check if response is valid
    if (response.length() > 0) {
        scaleStatus.isScaleZeroed = true;
        return true;
    }
    
    return false;
}

bool calibrateScale(float knownWeight) {
    if (!scaleStatus.isConnected) {
        return false;
    }
    
    // Enter calibration mode
    Serial2.print(CMD_CAL);
    delay(1000);
    
    // Read current weight
    float measuredWeight = readScaleWeight();
    
    if (measuredWeight <= 0) {
        return false;
    }
    
    // Calculate calibration factor
    scaleCal.knownWeight = knownWeight;
    scaleCal.measuredWeight = measuredWeight;
    scaleCal.calibrationFactor = knownWeight / measuredWeight;
    scaleCal.isCalibrated = true;
    scaleCal.lastCalibration = millis();
    
    // Save calibration
    saveCalibration();
    
    return true;
}

bool isWeightStable() {
    static float lastWeight = 0;
    static int stableCount = 0;
    
    float currentWeight = scaleStatus.currentWeight;
    
    // Check if weight is within stability range
    if (abs(currentWeight - lastWeight) <= systemConfig.accuracyRange) {
        stableCount++;
        if (stableCount >= NUM_STABILITY_SAMPLES) {
            return true;
        }
    } else {
        stableCount = 0;
    }
    
    lastWeight = currentWeight;
    return false;
}

bool checkScaleCommunication() {
    // Send print command
    Serial2.print(CMD_PRINT);
    
    // Wait for response
    unsigned long startTime = millis();
    while (!Serial2.available() && millis() - startTime < SCALE_TIMEOUT) {
        delay(10);
    }
    
    if (!Serial2.available()) {
        return false;
    }
    
    // Read response
    String response = Serial2.readStringUntil('\n');
    
    // Check if response is valid
    return response.length() > 0;
}

void saveCalibration() {
    // Open calibration file
    File calFile = SD.open("scale_cal.txt", FILE_WRITE);
    if (!calFile) {
        return;
    }
    
    // Write calibration data
    calFile.write((const uint8_t*)&scaleCal, sizeof(ScaleCalibration));
    
    // Close file
    calFile.close();
}

void loadCalibration() {
    // Open calibration file
    File calFile = SD.open("scale_cal.txt", FILE_READ);
    if (!calFile) {
        return;
    }
    
    // Read calibration data
    if (calFile.read((uint8_t*)&scaleCal, sizeof(ScaleCalibration)) == sizeof(ScaleCalibration)) {
        // Validate calibration data
        if (scaleCal.knownWeight <= 0 || scaleCal.measuredWeight <= 0 || 
            scaleCal.calibrationFactor <= 0) {
            scaleCal.isCalibrated = false;
        }
    } else {
        scaleCal.isCalibrated = false;
    }
    
    // Close file
    calFile.close();
}

float getWeight() {
    return scaleStatus.currentWeight;
}

float getLastStableWeight() {
    return scaleStatus.lastStableWeight;
}

bool isScaleConnected() {
    return scaleStatus.isConnected;
}

bool isScaleStable() {
    return scaleStatus.isStable;
} 