#ifndef EEPROM_MANAGER_H
#define EEPROM_MANAGER_H

#include <Arduino.h>
#include <EEPROM.h>
#include "system_state.h"

// EEPROM Layout
#define EEPROM_VERSION 1

// EEPROM Addresses
#define EEPROM_VERSION_ADDR 0
#define EEPROM_CHECKSUM_ADDR 1
#define EEPROM_CONFIG_START 10
#define EEPROM_PROFILES_START 100
#define EEPROM_POSITIONS_START 500
#define EEPROM_LOGS_START 1000
#define EEPROM_LOG_COUNT_ADDR 2000
#define EEPROM_LOG_ENTRY_SIZE 256

// EEPROM Flags
#define EEPROM_VALID_FLAG 0xAA
#define EEPROM_PROFILE_VALID_FLAG 0xBB
#define EEPROM_POSITION_VALID_FLAG 0xCC
#define EEPROM_LOG_VALID_FLAG 0xDD

// EEPROM Sizes
#define EEPROM_CONFIG_SIZE sizeof(SystemConfig)
#define EEPROM_PROFILE_SIZE sizeof(PowderProfile)
#define EEPROM_POSITION_SIZE sizeof(MotorPositions)
#define MAX_PROFILES 10
#define MAX_LOGS 100

// Powder Profile Structure
struct PowderProfile {
    char name[32];
    float multiplier;
    float rotationCount;
    float totalWeight;
    bool isValid;
    uint8_t checksum;
};

// Motor Position Structure
struct MotorPositions {
    long xPos1;
    long xPos4;
    long zPos2;
    long zPos3;
    long zPos5;
    long gripperPosA;
    long gripperPosB;
    bool isValid;
    uint8_t checksum;
};

// EEPROM Manager Functions
void initEEPROM();
bool validateEEPROM();
void formatEEPROM();
uint8_t calculateChecksum();

// System Configuration Functions
bool saveSystemConfig(const SystemConfig& config);
bool loadSystemConfig(SystemConfig& config);
bool clearSystemConfig();

// Profile Management Functions
bool savePowderProfile(int index, const PowderProfile& profile);
bool loadPowderProfile(int index, PowderProfile& profile);
bool clearPowderProfile(int index);
bool clearAllProfiles();
bool isProfileValid(int index);

// Motor Position Functions
bool saveMotorPositions(const MotorPositions& positions);
bool loadMotorPositions(MotorPositions& positions);
bool clearMotorPositions();
bool isPositionsValid();

// Log Management Functions
bool saveLogEntry(const char* logEntry);
bool clearLogs();
int getLogCount();
bool getLogEntry(int index, char* buffer, int bufferSize);

// EEPROM Health Functions
bool isEEPROMValid();
bool needsEEPROMFormat();
void repairEEPROM();

// Utility Functions
bool writeEEPROMBytes(int address, const uint8_t* data, int length);
bool readEEPROMBytes(int address, uint8_t* data, int length);
bool writeEEPROMString(int address, const char* str);
bool readEEPROMString(int address, char* str, int maxLength);

// External variable declarations
extern SystemConfig systemConfig;
extern PowderProfile powderProfiles[MAX_PROFILES];
extern MotorPositions motorPositions;

#endif // EEPROM_MANAGER_H 