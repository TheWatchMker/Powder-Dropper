#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#include <Arduino.h>

// System State Enums
enum SystemState {
    STATE_IDLE = 0,
    STATE_HOMING,
    STATE_ZEROING_SCALE,
    STATE_LOADING_CASE,
    STATE_MOVING_TO_CASE,
    STATE_GRIPPING_CASE,
    STATE_MOVING_TO_SCALE,
    STATE_DROPPING_CASE,
    STATE_DISPENSING_POWDER,
    STATE_FINISHING_DROP,
    STATE_MOVING_TO_START,
    STATE_ERROR,
    STATE_PAUSED,
    STATE_PRIME_MODE
};

enum ErrorCode {
    ERROR_NONE = 0,
    ERROR_SCALE_COMMUNICATION,
    ERROR_SCALE_TIMEOUT,
    ERROR_MOTOR_TIMEOUT,
    ERROR_LIMIT_SWITCH,
    ERROR_WEIGHT_OVER_TOLERANCE,
    ERROR_NO_WEIGHT_DETECTED,
    ERROR_EEPROM_CORRUPTION,
    ERROR_PROFILE_INVALID,
    ERROR_SD_CARD,
    ERROR_MOTOR_FAULT,
    ERROR_SYSTEM_FAULT
};

// System Configuration Structure
struct SystemConfig {
    float targetWeight;
    float tolerancePercentage;
    int caseQuantity;
    uint8_t vibratorySpeed;
    uint16_t scaleSettleTime;
    float accuracyRange;
    bool continuousMode;
    bool primeMode;
    bool autoLearningEnabled;
    bool averagingEnabled;
    uint8_t checksum;
};

// System Status Structure
struct SystemStatus {
    SystemState currentState;
    ErrorCode lastError;
    bool isHomed;
    bool isScaleZeroed;
    int completedCases;
    float currentWeight;
    float currentFlowRate;
    unsigned long lastOperationTime;
    bool isPaused;
    bool isPrimeMode;
    bool isEmergencyStop;
};

// System State Management Functions
void initSystemState();
void updateSystemState(SystemState newState);
void handleSystemError(ErrorCode error);
void clearSystemError();
void pauseSystem();
void resumeSystem();
void emergencyStop();
void resetSystem();

// System Configuration Functions
void loadSystemConfig();
void saveSystemConfig();
void updateSystemConfig(const SystemConfig& config);
const SystemConfig& getSystemConfig();

// System Status Functions
const SystemStatus& getSystemStatus();
bool isSystemReady();
bool isSystemError();
bool isSystemPaused();
bool isSystemPrimeMode();
bool isSystemEmergencyStop();

// State Transition Functions
bool canTransitionTo(SystemState newState);
void transitionTo(SystemState newState);
void handleStateTransition();

// Error Handling Functions
void logError(ErrorCode error, const char* message);
void handleErrorState();
bool isErrorRecoverable(ErrorCode error);
void recoverFromError();

// External variable declarations
extern SystemConfig systemConfig;
extern SystemStatus systemStatus;
extern const char* ERROR_MESSAGES[];

#endif // SYSTEM_STATE_H 