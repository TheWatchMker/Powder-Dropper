#ifndef POWDER_PROFILE_H
#define POWDER_PROFILE_H

#include <Arduino.h>

// Maximum number of powder profiles that can be stored
#define MAX_POWDER_PROFILES 20

// Profile validation constants
#define MIN_MULTIPLIER 0.1
#define MAX_MULTIPLIER 2.0
#define MIN_VIBRATOR_SPEED 0
#define MAX_VIBRATOR_SPEED 255
#define MIN_UNDERSHOOT_PERCENTAGE 0
#define MAX_UNDERSHOOT_PERCENTAGE 50
#define MIN_TOLERANCE_PERCENTAGE 0
#define MAX_TOLERANCE_PERCENTAGE 10

// EEPROM addresses for profile storage
#define EEPROM_PROFILES_START 100
#define EEPROM_PROFILE_VALID_FLAG 0xAA

// Powder Profile Structure
struct PowderProfile {
    char name[32];              // Fixed size name buffer
    float multiplier;           // Powder flow rate multiplier
    bool averagingEnabled;      // Whether powder averaging is enabled
    unsigned long rotationCount; // Number of rotations for averaging
    float totalWeight;          // Total weight dispensed for averaging
    int vibratorBaseSpeed;      // Default vibrator speed (0-255)
    int vibratorHighSpeedLimit; // Maximum vibrator speed (0-255)
    int successfulCharges;      // Number of successful charges
    bool autoLearningEnabled;   // Whether automatic learning is enabled
    float currentFlowRate;      // Current measured flow rate
    bool isActive;              // Whether this profile slot is used
    float undershootPercentage; // Percentage to undershoot target
    float tolerancePercentage;  // Allowed weight variation percentage
};

// Profile Management Functions
void initPowderProfiles();
bool addPowderProfile(const char* name);
bool deletePowderProfile(int index);
bool updatePowderProfile(int index, const PowderProfile& profile);
bool loadPowderProfiles();
bool savePowderProfiles();
int findProfileByName(const char* name);
int getNextAvailableProfileSlot();
bool validatePowderProfile(int index);
void resetPowderProfile(int index);

// Profile Analytics Functions
void updateProfileAnalytics(int index, float weight, unsigned long rotations);
float calculateAverageFlowRate(int index);
float calculateRequiredRotations(int index, float targetWeight);
bool isProfileCalibrated(int index);
void enableProfileAveraging(int index);
void disableProfileAveraging(int index);
void setProfileAutoLearning(int index, bool enabled);
void updateProfileMultiplier(int index, float newMultiplier);

// Profile Data Access Functions
const PowderProfile* getProfile(int index);
const char* getProfileName(int index);
float getProfileMultiplier(int index);
bool isProfileActive(int index);
int getProfileVibratorSpeed(int index);
float getProfileTolerance(int index);
int getProfileSuccessfulCharges(int index);
bool isProfileAveragingEnabled(int index);

// External variable declarations
extern PowderProfile powderProfiles[MAX_POWDER_PROFILES];
extern int currentProfileIndex;

#endif // POWDER_PROFILE_H 