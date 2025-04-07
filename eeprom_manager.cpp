#include "eeprom_manager.h"
#include "system_state.h"

// Initialize global variables
SystemConfig systemConfig;
PowderProfile powderProfiles[MAX_PROFILES];
MotorPositions motorPositions;

void initEEPROM() {
    if (!validateEEPROM()) {
        formatEEPROM();
    }
}

bool validateEEPROM() {
    // Check version
    uint8_t version;
    EEPROM.get(EEPROM_VERSION_ADDR, version);
    if (version != EEPROM_VERSION) return false;
    
    // Check checksum
    uint8_t storedChecksum;
    EEPROM.get(EEPROM_CHECKSUM_ADDR, storedChecksum);
    uint8_t calculatedChecksum = calculateChecksum();
    if (storedChecksum != calculatedChecksum) return false;
    
    return true;
}

void formatEEPROM() {
    // Write version
    EEPROM.put(EEPROM_VERSION_ADDR, EEPROM_VERSION);
    
    // Initialize all data areas with default values
    SystemConfig defaultConfig;
    defaultConfig.targetWeight = 0.0;
    defaultConfig.tolerancePercentage = 1.0;
    defaultConfig.caseQuantity = 0;
    defaultConfig.vibratorySpeed = 128;
    defaultConfig.scaleSettleTime = 1000;
    defaultConfig.accuracyRange = 0.1;
    defaultConfig.continuousMode = false;
    defaultConfig.primeMode = false;
    defaultConfig.autoLearningEnabled = true;
    defaultConfig.averagingEnabled = true;
    defaultConfig.checksum = 0;
    
    // Save default configuration
    saveSystemConfig(defaultConfig);
    
    // Clear all profiles
    clearAllProfiles();
    
    // Clear motor positions
    clearMotorPositions();
    
    // Clear logs
    clearLogs();
    
    // Update checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.put(EEPROM_CHECKSUM_ADDR, checksum);
    EEPROM.commit();
}

uint8_t calculateChecksum() {
    uint8_t checksum = 0;
    for (int i = 0; i < EEPROM.length(); i++) {
        if (i != EEPROM_CHECKSUM_ADDR) {  // Skip checksum byte
            checksum ^= EEPROM.read(i);
        }
    }
    return checksum;
}

bool saveSystemConfig(const SystemConfig& config) {
    // Save configuration
    EEPROM.put(EEPROM_CONFIG_START, config);
    
    // Update checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.put(EEPROM_CHECKSUM_ADDR, checksum);
    
    // Commit changes
    return EEPROM.commit();
}

bool loadSystemConfig(SystemConfig& config) {
    // Load configuration
    EEPROM.get(EEPROM_CONFIG_START, config);
    
    // Validate checksum
    uint8_t storedChecksum;
    EEPROM.get(EEPROM_CHECKSUM_ADDR, storedChecksum);
    uint8_t calculatedChecksum = calculateChecksum();
    
    return storedChecksum == calculatedChecksum;
}

bool clearSystemConfig() {
    SystemConfig emptyConfig;
    memset(&emptyConfig, 0, sizeof(SystemConfig));
    return saveSystemConfig(emptyConfig);
}

bool savePowderProfile(int index, const PowderProfile& profile) {
    if (index < 0 || index >= MAX_PROFILES) return false;
    
    // Calculate profile address
    int profileAddr = EEPROM_PROFILES_START + (index * EEPROM_PROFILE_SIZE);
    
    // Save profile
    EEPROM.put(profileAddr, profile);
    
    // Update checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.put(EEPROM_CHECKSUM_ADDR, checksum);
    
    return EEPROM.commit();
}

bool loadPowderProfile(int index, PowderProfile& profile) {
    if (index < 0 || index >= MAX_PROFILES) return false;
    
    // Calculate profile address
    int profileAddr = EEPROM_PROFILES_START + (index * EEPROM_PROFILE_SIZE);
    
    // Load profile
    EEPROM.get(profileAddr, profile);
    
    return profile.isValid;
}

bool clearPowderProfile(int index) {
    if (index < 0 || index >= MAX_PROFILES) return false;
    
    PowderProfile emptyProfile;
    memset(&emptyProfile, 0, sizeof(PowderProfile));
    emptyProfile.isValid = false;
    
    return savePowderProfile(index, emptyProfile);
}

bool clearAllProfiles() {
    for (int i = 0; i < MAX_PROFILES; i++) {
        if (!clearPowderProfile(i)) return false;
    }
    return true;
}

bool isProfileValid(int index) {
    if (index < 0 || index >= MAX_PROFILES) return false;
    
    PowderProfile profile;
    return loadPowderProfile(index, profile) && profile.isValid;
}

bool saveMotorPositions(const MotorPositions& positions) {
    // Save positions
    EEPROM.put(EEPROM_POSITIONS_START, positions);
    
    // Update checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.put(EEPROM_CHECKSUM_ADDR, checksum);
    
    return EEPROM.commit();
}

bool loadMotorPositions(MotorPositions& positions) {
    // Load positions
    EEPROM.get(EEPROM_POSITIONS_START, positions);
    
    return positions.isValid;
}

bool clearMotorPositions() {
    MotorPositions emptyPositions;
    memset(&emptyPositions, 0, sizeof(MotorPositions));
    emptyPositions.isValid = false;
    
    return saveMotorPositions(emptyPositions);
}

void clearLogs() {
    // Reset log count
    EEPROM.put(EEPROM_LOG_COUNT_ADDR, 0);
    
    // Update checksum
    uint8_t checksum = calculateChecksum();
    EEPROM.put(EEPROM_CHECKSUM_ADDR, checksum);
    
    EEPROM.commit();
}

bool saveLogEntry(const char* logEntry) {
    // Get current log count
    int logCount;
    EEPROM.get(EEPROM_LOG_COUNT_ADDR, logCount);
    
    if (logCount >= MAX_LOGS) {
        // Shift all logs up by one
        for (int i = 0; i < MAX_LOGS - 1; i++) {
            char tempBuffer[EEPROM_LOG_ENTRY_SIZE];
            getLogEntry(i + 1, tempBuffer, EEPROM_LOG_ENTRY_SIZE);
            writeEEPROMString(EEPROM_LOGS_START + (i * EEPROM_LOG_ENTRY_SIZE), tempBuffer);
        }
        logCount = MAX_LOGS - 1;
    }
    
    // Write new log entry
    bool success = writeEEPROMString(EEPROM_LOGS_START + (logCount * EEPROM_LOG_ENTRY_SIZE), logEntry);
    
    if (success) {
        logCount++;
        EEPROM.put(EEPROM_LOG_COUNT_ADDR, logCount);
        uint8_t checksum = calculateChecksum();
        EEPROM.put(EEPROM_CHECKSUM_ADDR, checksum);
    }
    
    return success;
}

int getLogCount() {
    int count;
    EEPROM.get(EEPROM_LOG_COUNT_ADDR, count);
    return count;
}

bool getLogEntry(int index, char* buffer, int bufferSize) {
    if (index < 0 || index >= MAX_LOGS) return false;
    
    return readEEPROMString(EEPROM_LOGS_START + (index * EEPROM_LOG_ENTRY_SIZE), buffer, bufferSize);
}

bool isEEPROMValid() {
    return validateEEPROM();
}

bool needsEEPROMFormat() {
    uint8_t version;
    EEPROM.get(EEPROM_VERSION_ADDR, version);
    return version != EEPROM_VERSION;
}

void repairEEPROM() {
    formatEEPROM();
}

bool writeEEPROMBytes(int address, const uint8_t* data, int length) {
    if (address + length > EEPROM.length()) return false;
    
    for (int i = 0; i < length; i++) {
        EEPROM.put(address + i, data[i]);
    }
    return true;
}

bool readEEPROMBytes(int address, uint8_t* data, int length) {
    if (address + length > EEPROM.length()) return false;
    
    for (int i = 0; i < length; i++) {
        data[i] = EEPROM.read(address + i);
    }
    return true;
}

bool writeEEPROMString(int address, const char* str) {
    if (str == nullptr) return false;
    
    int length = strlen(str);
    if (length >= EEPROM_LOG_ENTRY_SIZE) return false;
    
    // Write length first
    EEPROM.put(address, (uint8_t)length);
    
    // Write string
    for (int i = 0; i < length; i++) {
        EEPROM.put(address + sizeof(uint8_t) + i, str[i]);
    }
    
    // Write null terminator
    EEPROM.put(address + sizeof(uint8_t) + length, '\0');
    
    return true;
}

bool readEEPROMString(int address, char* str, int maxLength) {
    if (str == nullptr) return false;
    
    uint8_t length;
    EEPROM.get(address, length);
    
    if (length >= maxLength) return false;
    
    // Read string
    for (int i = 0; i < length; i++) {
        str[i] = EEPROM.read(address + sizeof(uint8_t) + i);
    }
    
    // Add null terminator
    str[length] = '\0';
    
    return true;
} 