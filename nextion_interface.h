#ifndef NEXTION_INTERFACE_H
#define NEXTION_INTERFACE_H

#include <Arduino.h>
#include <Nextion.h>
#include "system_state.h"

// Nextion Display Communication Settings
#define NEXTION_BAUD 9600
#define NEXTION_TX 11
#define NEXTION_RX 12

// Nextion Display Elements
#define TXT_ERROR 1
#define TXT_STATUS 2
#define NUM_CURRENT_WEIGHT 1
#define NUM_CURRENT_FLOW_RATE 2
#define NUM_CASE_QUANTITY 3
#define NUM_TARGET_WEIGHT 4
#define NUM_TOLERANCE 5
#define NUM_VIBRATORY_SPEED 6
#define NUM_SCALE_SETTLE_TIME 7
#define NUM_ACCURACY_RANGE 8

// Nextion Pages
#define PAGE_MAIN 0
#define PAGE_SETTINGS 1
#define PAGE_ERROR 2
#define PAGE_CONFIRMATION 3
#define PAGE_CALIBRATION 4
#define PAGE_LOG 5
#define PAGE_PROFILE 6
#define PAGE_JOG 7

// Nextion Buttons
#define BTN_START 1
#define BTN_STOP 2
#define BTN_HOME 3
#define BTN_ZERO 4
#define BTN_PRIME 5
#define BTN_SETTINGS 6
#define BTN_ACKNOWLEDGE 21
#define BTN_CAL_CONFIRM 22
#define BTN_SAVE 23
#define BTN_CANCEL 24
#define BTN_YES 25
#define BTN_NO 26
#define BTN_JOG_X_POS 27
#define BTN_JOG_X_NEG 28
#define BTN_JOG_Y_POS 29
#define BTN_JOG_Y_NEG 30
#define BTN_JOG_Z_POS 31
#define BTN_JOG_Z_NEG 32
#define BTN_SAVE_POS_X1 33
#define BTN_SAVE_POS_X4 34
#define BTN_SAVE_POS_Z2 35
#define BTN_SAVE_POS_Z3 36
#define BTN_SAVE_POS_Z5 37
#define BTN_SAVE_POS_GRIP_A 38
#define BTN_SAVE_POS_GRIP_B 39
#define BTN_ADD_PROFILE 40
#define BTN_DELETE_PROFILE 41
#define BTN_TOGGLE_ANALYSIS 42

// Numeric Input IDs
#define NUM_TARGET_WEIGHT 1
#define NUM_TOLERANCE 2
#define NUM_CASE_QUANTITY 3
#define NUM_VIBRATORY_SPEED 4
#define NUM_CURRENT_WEIGHT 5
#define NUM_FLOW_RATE 6
#define NUM_X_POS1 7
#define NUM_X_POS4 8
#define NUM_Z_POS2 9
#define NUM_Z_POS3 10
#define NUM_Z_POS5 11
#define NUM_GRIPPER_POS_A 12
#define NUM_GRIPPER_POS_B 13
#define NUM_SCALE_SETTLE_TIME 14
#define NUM_ACCURACY_RANGE 15
#define NUM_PROFILE_MULTIPLIER 16
#define NUM_PROFILE_ROTATION_COUNT 17
#define NUM_PROFILE_TOTAL_WEIGHT 18
#define NUM_POWDER_ANALYSIS_ENABLED 19
#define NUM_WEIGHT_TOLERANCE 60
#define NUM_STABILITY_SAMPLES 61
#define NUM_STABILITY_DELAY 62
#define NUM_STABILITY_TOLERANCE 63
#define NUM_X_POS 42
#define NUM_Z_POS 44
#define NUM_GRIPPER_POS 45
#define NUM_MOVEMENT_STATUS 46
#define NUM_COLLISION_STATUS 47
#define NUM_POSITION_STATUS 48
#define NUM_PROFILE_SELECT 49
#define NUM_JOG_DISTANCE 50
#define NUM_PROFILE_COUNT 51

// Text Display IDs
#define TXT_STATUS 1
#define TXT_ERROR 2
#define TXT_SUCCESS 3
#define TXT_ERROR_MSG 4
#define TXT_CAL_MESSAGE 5
#define TXT_PROFILE_NAME 6
#define TXT_PROFILE_STATUS 7
#define TXT_JOG_STATUS 8
#define TXT_POSITION_STATUS 9
#define TXT_ANALYSIS_STATUS 10
#define TXT_LOG_ENTRY 11

// Display Update Structure
struct DisplayUpdate {
    float currentWeight;
    float targetWeight;
    float flowRate;
    int completedCases;
    int totalCases;
    bool isStable;
    bool isMoving;
    bool isError;
    bool isAnalyzing;
    bool isJogging;
    int selectedProfile;
    int profileCount;
    float jogDistance;
    char statusMessage[64];
    char errorMessage[64];
    char profileName[32];
};

// Function declarations
void initNextion();
bool checkNextionCommunication();
void updateDisplayStatus(const String& message);
void updateDisplayError(const String& message);
void updateDisplayWeight(float weight);
void updateDisplayFlowRate(float flowRate);
void updateDisplayCaseCount(int completed, int total);
void updateDisplayProfile(const String& name);
void updateDisplayJogDistance(float distance);
void clearNextionDisplay();
void sendNextionCommand(const String& command);
bool getNextionNumericValue(uint8_t componentId, uint32_t* value);
bool getNextionText(uint8_t componentId, char* buffer, size_t bufferSize);
void handleNextionEvent();

// Profile Management Functions
void updateProfileList();
void selectProfile(int index);
void addProfile(const char* name);
void deleteProfile(int index);
void toggleProfileAnalysis(int index);
void updateProfileDisplay(const char* name, float multiplier, float rotationCount, float totalWeight);

// Jog Control Functions
void handleJogMovement(int axis, int direction);
void updateJogStatus(bool isJogging);
void updatePositionDisplay(long xPos, long zPos, long gripperPos);
void saveCurrentPosition(int positionId);

// External variable declarations
extern Nextion nextion;
extern DisplayUpdate currentDisplayUpdate;

#endif // NEXTION_INTERFACE_H 