#include "powder_profile.h"
#include <EEPROM.h>

// Initialize global variables
PowderProfile powderProfiles[MAX_POWDER_PROFILES];
int currentProfileIndex = -1;

void initPowderProfiles() {
    // Initialize all profiles as inactive
    for (int i = 0; i < MAX_POWDER_PROFILES; i++) {
        powderProfiles[i].isActive = false;
        powderProfiles[i].averagingEnabled = false;
        powderProfiles[i].autoLearningEnabled = false;
        powderProfiles[i].rotationCount = 0;
        powderProfiles[i].totalWeight = 0.0;
        powderProfiles[i].multiplier = 1.0;
        powderProfiles[i].vibratorBaseSpeed = 128;  // Default 50%
        powderProfiles[i].vibratorHighSpeedLimit = 255;  // Default 100%
        powderProfiles[i].successfulCharges = 0;
        powderProfiles[i].currentFlowRate = 0.0;
        powderProfiles[i].undershootPercentage = 5.0;  // Default 5%
        powderProfiles[i].tolerancePercentage = 1.0;   // Default 1%
    }
    
    // Load saved profiles from EEPROM
    loadPowderProfiles();
}

bool addPowderProfile(const char* name) {
    int slot = getNextAvailableProfileSlot();
    if (slot == -1) return false;
    
    strncpy(powderProfiles[slot].name, name, sizeof(powderProfiles[slot].name) - 1);
    powderProfiles[slot].name[sizeof(powderProfiles[slot].name) - 1] = '\0';
    powderProfiles[slot].isActive = true;
    
    // Save to EEPROM
    savePowderProfiles();
    return true;
}

bool deletePowderProfile(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return false;
    
    resetPowderProfile(index);
    savePowderProfiles();
    return true;
}

bool updatePowderProfile(int index, const PowderProfile& profile) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return false;
    
    powderProfiles[index] = profile;
    savePowderProfiles();
    return true;
}

bool loadPowderProfiles() {
    // Check if valid data exists
    uint8_t flag;
    EEPROM.get(EEPROM_PROFILES_START, flag);
    
    if (flag == EEPROM_PROFILE_VALID_FLAG) {
        // Load all profiles
        for (int i = 0; i < MAX_POWDER_PROFILES; i++) {
            EEPROM.get(EEPROM_PROFILES_START + sizeof(uint8_t) + i * sizeof(PowderProfile), 
                      powderProfiles[i]);
        }
        return true;
    }
    return false;
}

bool savePowderProfiles() {
    // Write valid data flag
    EEPROM.put(EEPROM_PROFILES_START, EEPROM_PROFILE_VALID_FLAG);
    
    // Save all profiles
    for (int i = 0; i < MAX_POWDER_PROFILES; i++) {
        EEPROM.put(EEPROM_PROFILES_START + sizeof(uint8_t) + i * sizeof(PowderProfile), 
                  powderProfiles[i]);
    }
    return true;
}

int findProfileByName(const char* name) {
    for (int i = 0; i < MAX_POWDER_PROFILES; i++) {
        if (powderProfiles[i].isActive && 
            strcmp(powderProfiles[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int getNextAvailableProfileSlot() {
    for (int i = 0; i < MAX_POWDER_PROFILES; i++) {
        if (!powderProfiles[i].isActive) {
            return i;
        }
    }
    return -1;
}

bool validatePowderProfile(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return false;
    
    PowderProfile& profile = powderProfiles[index];
    
    // Validate ranges
    if (profile.multiplier < MIN_MULTIPLIER || profile.multiplier > MAX_MULTIPLIER) return false;
    if (profile.vibratorBaseSpeed < MIN_VIBRATOR_SPEED || 
        profile.vibratorBaseSpeed > MAX_VIBRATOR_SPEED) return false;
    if (profile.vibratorHighSpeedLimit < MIN_VIBRATOR_SPEED || 
        profile.vibratorHighSpeedLimit > MAX_VIBRATOR_SPEED) return false;
    if (profile.undershootPercentage < MIN_UNDERSHOOT_PERCENTAGE || 
        profile.undershootPercentage > MAX_UNDERSHOOT_PERCENTAGE) return false;
    if (profile.tolerancePercentage < MIN_TOLERANCE_PERCENTAGE || 
        profile.tolerancePercentage > MAX_TOLERANCE_PERCENTAGE) return false;
    
    return true;
}

void resetPowderProfile(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return;
    
    powderProfiles[index].isActive = false;
    powderProfiles[index].averagingEnabled = false;
    powderProfiles[index].autoLearningEnabled = false;
    powderProfiles[index].rotationCount = 0;
    powderProfiles[index].totalWeight = 0.0;
    powderProfiles[index].multiplier = 1.0;
    powderProfiles[index].vibratorBaseSpeed = 128;
    powderProfiles[index].vibratorHighSpeedLimit = 255;
    powderProfiles[index].successfulCharges = 0;
    powderProfiles[index].currentFlowRate = 0.0;
    powderProfiles[index].undershootPercentage = 5.0;
    powderProfiles[index].tolerancePercentage = 1.0;
}

void updateProfileAnalytics(int index, float weight, unsigned long rotations) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return;
    
    PowderProfile& profile = powderProfiles[index];
    profile.totalWeight += weight;
    profile.rotationCount += rotations;
    
    if (profile.rotationCount > 0) {
        profile.currentFlowRate = profile.totalWeight / profile.rotationCount;
    }
}

float calculateAverageFlowRate(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return 0.0;
    
    PowderProfile& profile = powderProfiles[index];
    if (profile.rotationCount == 0) return 0.0;
    
    return profile.totalWeight / profile.rotationCount;
}

float calculateRequiredRotations(int index, float targetWeight) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return 0.0;
    
    PowderProfile& profile = powderProfiles[index];
    if (profile.currentFlowRate <= 0) return 0.0;
    
    return targetWeight / profile.currentFlowRate;
}

bool isProfileCalibrated(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return false;
    
    PowderProfile& profile = powderProfiles[index];
    return profile.rotationCount >= 1000;  // Consider calibrated after 1000 rotations
}

void enableProfileAveraging(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return;
    powderProfiles[index].averagingEnabled = true;
}

void disableProfileAveraging(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return;
    powderProfiles[index].averagingEnabled = false;
}

void setProfileAutoLearning(int index, bool enabled) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return;
    powderProfiles[index].autoLearningEnabled = enabled;
}

void updateProfileMultiplier(int index, float newMultiplier) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return;
    
    if (newMultiplier >= MIN_MULTIPLIER && newMultiplier <= MAX_MULTIPLIER) {
        powderProfiles[index].multiplier = newMultiplier;
        savePowderProfiles();
    }
}

const PowderProfile* getProfile(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return nullptr;
    return &powderProfiles[index];
}

const char* getProfileName(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return nullptr;
    return powderProfiles[index].name;
}

float getProfileMultiplier(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return 1.0;
    return powderProfiles[index].multiplier;
}

bool isProfileActive(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return false;
    return powderProfiles[index].isActive;
}

int getProfileVibratorSpeed(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return 128;
    return powderProfiles[index].vibratorBaseSpeed;
}

float getProfileTolerance(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return 1.0;
    return powderProfiles[index].tolerancePercentage;
}

int getProfileSuccessfulCharges(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return 0;
    return powderProfiles[index].successfulCharges;
}

bool isProfileAveragingEnabled(int index) {
    if (index < 0 || index >= MAX_POWDER_PROFILES) return false;
    return powderProfiles[index].averagingEnabled;
} 