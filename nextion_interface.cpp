#include "nextion_interface.h"
#include "motor_control.h"
#include "system_state.h"

// Initialize Nextion object
Nextion nextion(Serial1);

// Initialize display update structure
DisplayUpdate currentDisplayUpdate;

void initNextion() {
    Serial1.begin(NEXTION_BAUD);
    nextion.init();
    clearNextionDisplay();
    
    // Initialize display update structure
    currentDisplayUpdate.currentWeight = 0.0;
    currentDisplayUpdate.targetWeight = 0.0;
    currentDisplayUpdate.flowRate = 0.0;
    currentDisplayUpdate.completedCases = 0;
    currentDisplayUpdate.totalCases = 0;
    currentDisplayUpdate.isStable = false;
    currentDisplayUpdate.isMoving = false;
    currentDisplayUpdate.isError = false;
    currentDisplayUpdate.isAnalyzing = false;
    currentDisplayUpdate.isJogging = false;
    currentDisplayUpdate.selectedProfile = -1;
    currentDisplayUpdate.profileCount = 0;
    currentDisplayUpdate.jogDistance = JOG_DEFAULT_DISTANCE;
    
    strcpy(currentDisplayUpdate.statusMessage, "System Ready");
    strcpy(currentDisplayUpdate.errorMessage, "");
    strcpy(currentDisplayUpdate.profileName, "");
}

bool checkNextionCommunication() {
    bool success = true;
    
    // Try reading numeric values
    uint32_t value;
    if (!nextion.getNumericValue(NUM_TARGET_WEIGHT, &value)) success = false;
    if (!nextion.getNumericValue(NUM_TOLERANCE, &value)) success = false;
    
    // Try reading text values
    char testBuffer[32];
    if (!nextion.getText(TXT_STATUS, testBuffer, sizeof(testBuffer))) success = false;
    
    // Try sending a command
    sendNextionCommand("page 0");
    delay(100);
    
    return success;
}

void updateNextionDisplay() {
    // Update numeric values
    setNextionNumericValue(NUM_CURRENT_WEIGHT, currentDisplayUpdate.currentWeight);
    setNextionNumericValue(NUM_TARGET_WEIGHT, currentDisplayUpdate.targetWeight);
    setNextionNumericValue(NUM_FLOW_RATE, currentDisplayUpdate.flowRate);
    setNextionNumericValue(NUM_MOVEMENT_STATUS, currentDisplayUpdate.isMoving ? 1 : 0);
    setNextionNumericValue(NUM_POSITION_STATUS, currentDisplayUpdate.isStable ? 1 : 0);
    setNextionNumericValue(NUM_PROFILE_SELECT, currentDisplayUpdate.selectedProfile);
    setNextionNumericValue(NUM_PROFILE_COUNT, currentDisplayUpdate.profileCount);
    setNextionNumericValue(NUM_JOG_DISTANCE, currentDisplayUpdate.jogDistance);
    
    // Update text displays
    setNextionText(TXT_STATUS, currentDisplayUpdate.statusMessage);
    if (currentDisplayUpdate.isError) {
        setNextionText(TXT_ERROR_MSG, currentDisplayUpdate.errorMessage);
    }
    
    // Update progress
    char progressMsg[32];
    snprintf(progressMsg, sizeof(progressMsg), "%d/%d", 
             currentDisplayUpdate.completedCases, 
             currentDisplayUpdate.totalCases);
    setNextionText(TXT_SUCCESS, progressMsg);
    
    // Update profile information
    if (currentDisplayUpdate.selectedProfile >= 0) {
        setNextionText(TXT_PROFILE_NAME, currentDisplayUpdate.profileName);
        setNextionText(TXT_PROFILE_STATUS, currentDisplayUpdate.isAnalyzing ? "Analysis Active" : "Analysis Inactive");
    }
    
    // Update jog status
    setNextionText(TXT_JOG_STATUS, currentDisplayUpdate.isJogging ? "Jogging Active" : "Jogging Inactive");
    
    // Update position status
    char posMsg[32];
    snprintf(posMsg, sizeof(posMsg), "X: %ld Z: %ld G: %ld", 
             stepperX.currentPosition(),
             stepperZ.currentPosition(),
             stepperGripper.currentPosition());
    setNextionText(TXT_POSITION_STATUS, posMsg);
}

void updateNextionVariables() {
    // Read numeric values from display
    uint32_t value;
    if (nextion.getNumericValue(NUM_TARGET_WEIGHT, &value)) {
        currentDisplayUpdate.targetWeight = value;
    }
    if (nextion.getNumericValue(NUM_TOLERANCE, &value)) {
        // Update tolerance value
    }
    if (nextion.getNumericValue(NUM_CASE_QUANTITY, &value)) {
        currentDisplayUpdate.totalCases = value;
    }
    if (nextion.getNumericValue(NUM_JOG_DISTANCE, &value)) {
        currentDisplayUpdate.jogDistance = value;
    }
}

void sendNextionCommand(const String& command) {
    nextion.sendCommand(command);
    delay(10);
}

void handleNextionVariables() {
    // Check for button presses
    if (isNextionButtonPressed(BTN_START)) {
        // Handle start button
        setNextionPage(PAGE_MAIN);
        strcpy(currentDisplayUpdate.statusMessage, "Starting Operation");
    }
    if (isNextionButtonPressed(BTN_STOP)) {
        // Handle stop button
        setNextionPage(PAGE_MAIN);
        strcpy(currentDisplayUpdate.statusMessage, "Operation Stopped");
    }
    if (isNextionButtonPressed(BTN_HOME)) {
        // Handle home button
        setNextionPage(PAGE_MAIN);
        strcpy(currentDisplayUpdate.statusMessage, "Homing Motors");
    }
    if (isNextionButtonPressed(BTN_ZERO)) {
        // Handle zero button
        setNextionPage(PAGE_MAIN);
        strcpy(currentDisplayUpdate.statusMessage, "Zeroing Scale");
    }
    if (isNextionButtonPressed(BTN_PRIME)) {
        // Handle prime button
        setNextionPage(PAGE_MAIN);
        strcpy(currentDisplayUpdate.statusMessage, "Prime Mode Active");
    }
    if (isNextionButtonPressed(BTN_SETTINGS)) {
        // Handle settings button
        setNextionPage(PAGE_SETTINGS);
    }
    
    // Handle jog buttons
    if (isNextionButtonPressed(BTN_JOG_X_POS)) handleJogMovement(0, 1);
    if (isNextionButtonPressed(BTN_JOG_X_NEG)) handleJogMovement(0, -1);
    if (isNextionButtonPressed(BTN_JOG_Y_POS)) handleJogMovement(1, 1);
    if (isNextionButtonPressed(BTN_JOG_Y_NEG)) handleJogMovement(1, -1);
    if (isNextionButtonPressed(BTN_JOG_Z_POS)) handleJogMovement(2, 1);
    if (isNextionButtonPressed(BTN_JOG_Z_NEG)) handleJogMovement(2, -1);
    
    // Handle position save buttons
    if (isNextionButtonPressed(BTN_SAVE_POS_X1)) saveCurrentPosition(1);
    if (isNextionButtonPressed(BTN_SAVE_POS_X4)) saveCurrentPosition(4);
    if (isNextionButtonPressed(BTN_SAVE_POS_Z2)) saveCurrentPosition(2);
    if (isNextionButtonPressed(BTN_SAVE_POS_Z3)) saveCurrentPosition(3);
    if (isNextionButtonPressed(BTN_SAVE_POS_Z5)) saveCurrentPosition(5);
    if (isNextionButtonPressed(BTN_SAVE_POS_GRIP_A)) saveCurrentPosition(6);
    if (isNextionButtonPressed(BTN_SAVE_POS_GRIP_B)) saveCurrentPosition(7);
    
    // Handle profile buttons
    if (isNextionButtonPressed(BTN_ADD_PROFILE)) {
        setNextionPage(PAGE_PROFILE);
        strcpy(currentDisplayUpdate.statusMessage, "Adding New Profile");
    }
    if (isNextionButtonPressed(BTN_DELETE_PROFILE)) {
        if (currentDisplayUpdate.selectedProfile >= 0) {
            deleteProfile(currentDisplayUpdate.selectedProfile);
        }
    }
    if (isNextionButtonPressed(BTN_TOGGLE_ANALYSIS)) {
        if (currentDisplayUpdate.selectedProfile >= 0) {
            toggleProfileAnalysis(currentDisplayUpdate.selectedProfile);
        }
    }
    
    // Update variables from display
    updateNextionVariables();
}

void setNextionPage(uint8_t pageId) {
    char command[16];
    snprintf(command, sizeof(command), "page %d", pageId);
    sendNextionCommand(command);
}

void setNextionNumericValue(uint8_t componentId, float value) {
    char command[32];
    snprintf(command, sizeof(command), "n%d.val=%d", componentId, (int)value);
    sendNextionCommand(command);
}

void setNextionText(uint8_t componentId, const char* text) {
    char command[128];
    snprintf(command, sizeof(command), "t%d.txt=\"%s\"", componentId, text);
    sendNextionCommand(command);
}

bool isNextionButtonPressed(uint8_t buttonId) {
    uint32_t value;
    if (nextion.getNumericValue(buttonId, &value)) {
        return value == 1;
    }
    return false;
}

void updateDisplayStatus(const String& message) {
    strncpy(currentDisplayUpdate.statusMessage, message.c_str(), sizeof(currentDisplayUpdate.statusMessage) - 1);
    nextion.sendCommand("t" + String(TXT_STATUS) + ".txt=\"" + message + "\"");
}

void updateDisplayError(const String& message) {
    strncpy(currentDisplayUpdate.errorMessage, message.c_str(), sizeof(currentDisplayUpdate.errorMessage) - 1);
    nextion.sendCommand("t" + String(TXT_ERROR) + ".txt=\"" + message + "\"");
}

void updateDisplayWeight(float weight) {
    currentDisplayUpdate.currentWeight = weight;
    nextion.sendCommand("n" + String(NUM_CURRENT_WEIGHT) + ".val=" + String(weight, 2));
}

void updateDisplayFlowRate(float flowRate) {
    currentDisplayUpdate.flowRate = flowRate;
    nextion.sendCommand("n" + String(NUM_CURRENT_FLOW_RATE) + ".val=" + String(flowRate, 2));
}

void updateDisplayCaseCount(int completed, int total) {
    currentDisplayUpdate.completedCases = completed;
    currentDisplayUpdate.totalCases = total;
    nextion.sendCommand("n" + String(NUM_CASE_QUANTITY) + ".val=" + String(completed));
}

void updateDisplayProfile(const String& name) {
    strncpy(currentDisplayUpdate.profileName, name.c_str(), sizeof(currentDisplayUpdate.profileName) - 1);
    nextion.sendCommand("t" + String(TXT_PROFILE) + ".txt=\"" + name + "\"");
}

void updateDisplayJogDistance(float distance) {
    currentDisplayUpdate.jogDistance = distance;
    nextion.sendCommand("n" + String(NUM_JOG_DISTANCE) + ".val=" + String(distance));
}

void clearNextionDisplay() {
    nextion.sendCommand("cls");
    delay(100);
}

void refreshNextionDisplay() {
    sendNextionCommand("ref 0");
}

// Profile Management Functions
void updateProfileList() {
    // Update profile count
    setNextionNumericValue(NUM_PROFILE_COUNT, currentDisplayUpdate.profileCount);
    
    // Update profile selection if valid
    if (currentDisplayUpdate.selectedProfile >= 0) {
        setNextionNumericValue(NUM_PROFILE_SELECT, currentDisplayUpdate.selectedProfile);
    }
}

void selectProfile(int index) {
    if (index >= 0 && index < currentDisplayUpdate.profileCount) {
        currentDisplayUpdate.selectedProfile = index;
        // Load profile data and update display
        updateProfileDisplay(powderProfiles[index].name,
                           powderProfiles[index].multiplier,
                           powderProfiles[index].rotationCount,
                           powderProfiles[index].totalWeight);
    }
}

void addProfile(const char* name) {
    if (currentDisplayUpdate.profileCount < MAX_PROFILES) {
        strcpy(powderProfiles[currentDisplayUpdate.profileCount].name, name);
        powderProfiles[currentDisplayUpdate.profileCount].multiplier = 1.0;
        powderProfiles[currentDisplayUpdate.profileCount].rotationCount = 0;
        powderProfiles[currentDisplayUpdate.profileCount].totalWeight = 0;
        powderProfiles[currentDisplayUpdate.profileCount].isValid = true;
        currentDisplayUpdate.profileCount++;
        updateProfileList();
    }
}

void deleteProfile(int index) {
    if (index >= 0 && index < currentDisplayUpdate.profileCount) {
        // Shift remaining profiles
        for (int i = index; i < currentDisplayUpdate.profileCount - 1; i++) {
            powderProfiles[i] = powderProfiles[i + 1];
        }
        currentDisplayUpdate.profileCount--;
        if (currentDisplayUpdate.selectedProfile == index) {
            currentDisplayUpdate.selectedProfile = -1;
        }
        updateProfileList();
    }
}

void toggleProfileAnalysis(int index) {
    if (index >= 0 && index < currentDisplayUpdate.profileCount) {
        currentDisplayUpdate.isAnalyzing = !currentDisplayUpdate.isAnalyzing;
        setNextionText(TXT_ANALYSIS_STATUS, 
                      currentDisplayUpdate.isAnalyzing ? "Analysis Active" : "Analysis Inactive");
    }
}

void updateProfileDisplay(const char* name, float multiplier, float rotationCount, float totalWeight) {
    strcpy(currentDisplayUpdate.profileName, name);
    setNextionNumericValue(NUM_PROFILE_MULTIPLIER, multiplier);
    setNextionNumericValue(NUM_PROFILE_ROTATION_COUNT, rotationCount);
    setNextionNumericValue(NUM_PROFILE_TOTAL_WEIGHT, totalWeight);
}

// Jog Control Functions
void handleJogMovement(int axis, int direction) {
    currentDisplayUpdate.isJogging = true;
    handleJogMovement(axis, direction);
    updateJogStatus(true);
}

void updateJogStatus(bool isJogging) {
    currentDisplayUpdate.isJogging = isJogging;
    setNextionText(TXT_JOG_STATUS, isJogging ? "Jogging Active" : "Jogging Inactive");
}

void updatePositionDisplay(long xPos, long zPos, long gripperPos) {
    char posMsg[32];
    snprintf(posMsg, sizeof(posMsg), "X: %ld Z: %ld G: %ld", xPos, zPos, gripperPos);
    setNextionText(TXT_POSITION_STATUS, posMsg);
}

void saveCurrentPosition(int positionId) {
    switch (positionId) {
        case 1: // X_POS1
            setNextionNumericValue(NUM_X_POS1, stepperX.currentPosition());
            break;
        case 4: // X_POS4
            setNextionNumericValue(NUM_X_POS4, stepperX.currentPosition());
            break;
        case 2: // Z_POS2
            setNextionNumericValue(NUM_Z_POS2, stepperZ.currentPosition());
            break;
        case 3: // Z_POS3
            setNextionNumericValue(NUM_Z_POS3, stepperZ.currentPosition());
            break;
        case 5: // Z_POS5
            setNextionNumericValue(NUM_Z_POS5, stepperZ.currentPosition());
            break;
        case 6: // GRIPPER_POS_A
            setNextionNumericValue(NUM_GRIPPER_POS_A, stepperGripper.currentPosition());
            break;
        case 7: // GRIPPER_POS_B
            setNextionNumericValue(NUM_GRIPPER_POS_B, stepperGripper.currentPosition());
            break;
    }
    strcpy(currentDisplayUpdate.statusMessage, "Position Saved");
}

void handleNextionEvent() {
    uint8_t event;
    if (nextion.getEvent(&event)) {
        switch (event) {
            case BTN_START:
                if (isSystemReady()) {
                    transitionTo(STATE_HOMING);
                }
                break;
            case BTN_STOP:
                pauseSystem();
                break;
            case BTN_HOME:
                if (isSystemPaused()) {
                    transitionTo(STATE_HOMING);
                }
                break;
            case BTN_ZERO:
                if (isSystemReady()) {
                    transitionTo(STATE_ZEROING_SCALE);
                }
                break;
            case BTN_PRIME:
                if (isSystemReady()) {
                    transitionTo(STATE_PRIME_MODE);
                }
                break;
            case BTN_ACKNOWLEDGE:
                if (isSystemError()) {
                    clearSystemError();
                }
                break;
            // Add other button handlers as needed
        }
    }
} 