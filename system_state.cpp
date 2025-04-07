#include "system_state.h"
#include <EEPROM.h>

// External variable definitions
SystemConfig systemConfig;
SystemStatus systemStatus;
const char* ERROR_MESSAGES[] = {
    "No error",
    "Scale communication error",
    "Scale timeout error",
    "Motor timeout error",
    "Limit switch triggered",
    "Weight over tolerance",
    "No weight detected",
    "EEPROM corruption",
    "Invalid profile",
    "SD card error",
    "Motor fault",
    "System fault"
};

// EEPROM addresses for system configuration
#define EEPROM_CONFIG_START 200
#define EEPROM_CONFIG_VALID_FLAG 0xBB

void initSystemState() {
    // Initialize system status
    systemStatus.currentState = STATE_IDLE;
    systemStatus.lastError = ERROR_NONE;
    systemStatus.isHomed = false;
    systemStatus.isScaleZeroed = false;
    systemStatus.completedCases = 0;
    systemStatus.currentWeight = 0.0;
    systemStatus.currentFlowRate = 0.0;
    systemStatus.lastOperationTime = 0;
    systemStatus.isPaused = false;
    systemStatus.isPrimeMode = false;
    systemStatus.isEmergencyStop = false;
    
    // Load system configuration
    loadSystemConfig();
}

void updateSystemState(SystemState newState) {
    if (canTransitionTo(newState)) {
        transitionTo(newState);
    }
}

void handleSystemError(ErrorCode error) {
    systemStatus.lastError = error;
    systemStatus.currentState = STATE_ERROR;
    logError(error, ERROR_MESSAGES[error]);
}

void clearSystemError() {
    systemStatus.lastError = ERROR_NONE;
    systemStatus.currentState = STATE_IDLE;
}

void pauseSystem() {
    systemStatus.isPaused = true;
    systemStatus.currentState = STATE_PAUSED;
}

void resumeSystem() {
    systemStatus.isPaused = false;
    systemStatus.currentState = STATE_IDLE;
}

void emergencyStop() {
    systemStatus.isEmergencyStop = true;
    systemStatus.currentState = STATE_ERROR;
    handleSystemError(ERROR_SYSTEM_FAULT);
}

void resetSystem() {
    systemStatus.isEmergencyStop = false;
    systemStatus.isPaused = false;
    systemStatus.currentState = STATE_IDLE;
    systemStatus.lastError = ERROR_NONE;
}

void loadSystemConfig() {
    // Load configuration from EEPROM
    EEPROM.get(EEPROM_CONFIG_START, systemConfig);
    
    // Validate configuration
    if (systemConfig.checksum != calculateChecksum(systemConfig)) {
        // Load default configuration
        systemConfig.targetWeight = 0.0;
        systemConfig.tolerancePercentage = 0.1;
        systemConfig.caseQuantity = 0;
        systemConfig.vibratorySpeed = 0;
        systemConfig.scaleSettleTime = 1000;
        systemConfig.accuracyRange = 0.1;
        systemConfig.continuousMode = false;
        systemConfig.primeMode = false;
        systemConfig.autoLearningEnabled = true;
        systemConfig.averagingEnabled = true;
        systemConfig.checksum = calculateChecksum(systemConfig);
        
        // Save default configuration
        saveSystemConfig();
    }
}

void saveSystemConfig() {
    systemConfig.checksum = calculateChecksum(systemConfig);
    EEPROM.put(EEPROM_CONFIG_START, systemConfig);
    EEPROM.commit();
}

void updateSystemConfig(const SystemConfig& config) {
    systemConfig = config;
    saveSystemConfig();
}

const SystemConfig& getSystemConfig() {
    return systemConfig;
}

const SystemStatus& getSystemStatus() {
    return systemStatus;
}

bool isSystemReady() {
    return systemStatus.currentState == STATE_IDLE && 
           !systemStatus.isPaused && 
           !systemStatus.isPrimeMode && 
           !systemStatus.isEmergencyStop;
}

bool isSystemError() {
    return systemStatus.currentState == STATE_ERROR;
}

bool isSystemPaused() {
    return systemStatus.isPaused;
}

bool isSystemPrimeMode() {
    return systemStatus.isPrimeMode;
}

bool isSystemEmergencyStop() {
    return systemStatus.isEmergencyStop;
}

bool canTransitionTo(SystemState newState) {
    // Check if transition is allowed based on current state
    switch (systemStatus.currentState) {
        case STATE_ERROR:
            return newState == STATE_IDLE;
        case STATE_PAUSED:
            return newState == STATE_IDLE;
        case STATE_PRIME_MODE:
            return newState == STATE_IDLE;
        case STATE_IDLE:
            return newState != STATE_ERROR && 
                   newState != STATE_PAUSED && 
                   newState != STATE_PRIME_MODE;
        default:
            return false;
    }
}

void transitionTo(SystemState newState) {
    systemStatus.currentState = newState;
    systemStatus.lastOperationTime = millis();
}

void handleStateTransition() {
    // Handle state transitions based on current state
    switch (systemStatus.currentState) {
        case STATE_HOMING:
            if (systemStatus.isHomed) {
                transitionTo(STATE_IDLE);
            }
            break;
        case STATE_ZEROING_SCALE:
            if (systemStatus.isScaleZeroed) {
                transitionTo(STATE_IDLE);
            }
            break;
        case STATE_LOADING_CASE:
            // Handle case loading state
            break;
        case STATE_MOVING_TO_CASE:
            // Handle moving to case state
            break;
        case STATE_GRIPPING_CASE:
            // Handle case gripping state
            break;
        case STATE_MOVING_TO_SCALE:
            // Handle moving to scale state
            break;
        case STATE_DROPPING_CASE:
            // Handle case dropping state
            break;
        case STATE_DISPENSING_POWDER:
            // Handle powder dispensing state
            break;
        case STATE_FINISHING_DROP:
            // Handle finishing drop state
            break;
        case STATE_MOVING_TO_START:
            // Handle moving to start state
            break;
    }
}

void logError(ErrorCode error, const char* message) {
    // Log error to SD card
    char logEntry[256];
    snprintf(logEntry, sizeof(logEntry), "ERROR: %s (Code: %d) - %s", 
             ERROR_MESSAGES[error], error, message);
    writeToLogFile("error_log.txt", logEntry);
}

void handleErrorState() {
    // Stop all motors
    stopAllMotors();
    
    // Update display with error
    updateDisplayStatus("Error: " + String(ERROR_MESSAGES[systemStatus.lastError]));
    
    // Log error
    logError(systemStatus.lastError, ERROR_MESSAGES[systemStatus.lastError]);
    
    // Check if error is recoverable
    if (isErrorRecoverable(systemStatus.lastError)) {
        // Attempt recovery
        if (attemptErrorRecovery()) {
            clearSystemError();
            return;
        }
    }
}

bool isErrorRecoverable(ErrorCode error) {
    switch (error) {
        case ERROR_SCALE_COMMUNICATION:
        case ERROR_SCALE_TIMEOUT:
        case ERROR_MOTOR_TIMEOUT:
        case ERROR_WEIGHT_OVER_TOLERANCE:
        case ERROR_NO_WEIGHT_DETECTED:
            return true;
        default:
            return false;
    }
}

void recoverFromError() {
    // Attempt to recover from error state
    if (isErrorRecoverable(systemStatus.lastError)) {
        // Reinitialize components
        if (!reinitializeComponents()) {
            return;
        }
        
        // Validate system state
        if (!validateSystemState()) {
            return;
        }
        
        // Clear error and return to idle state
        clearSystemError();
    }
} 