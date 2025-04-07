#include "motor_control.h"
#include <EEPROM.h>

// Define EEPROM address for motor positions
#define MOTOR_POS_EEPROM_ADDR 0x200  // Starting address for motor positions

// Initialize stepper motors
AccelStepper stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN);
AccelStepper stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN);
AccelStepper stepperZ(AccelStepper::DRIVER, Z_STEP_PIN, Z_DIR_PIN);
AccelStepper stepperGripper(AccelStepper::DRIVER, GRIPPER_STEP_PIN, GRIPPER_DIR_PIN);
AccelStepper stepperCaseFeeder(AccelStepper::DRIVER, CASE_FEEDER_STEP_PIN, CASE_FEEDER_DIR_PIN);
AccelStepper stepperPowderDropper(AccelStepper::DRIVER, POWDER_DROPPER_STEP_PIN, POWDER_DROPPER_DIR_PIN);

// Initialize TMC2209 drivers
TMC2209 tmc2209X;
TMC2209 tmc2209Y;
TMC2209 tmc2209Z;
TMC2209 tmc2209Gripper;
TMC2209 tmc2209CaseFeeder;
TMC2209 tmc2209PowderDropper;

// Initialize settings structures
SpeedSettings speedSettings = {
    .xSpeed = 1000,
    .ySpeed = 1000,
    .zSpeed = 1000,
    .gripperSpeed = 500,
    .powderDropperSpeed = 200,
    .caseFeederSpeed = 1000,
    .vibratorySpeed = 128,
    .powderCoarseSpeed = 800,
    .powderFineSpeed = 200,
    .homingSpeed = 500,
    .homingAcceleration = 250
};

MotorPositions motorPos = {
    .xPos1 = 0,
    .xPos4 = 0,
    .zPos2 = 0,
    .zPos3 = 0,
    .zPos5 = 0,
    .gripperPosA = 0,
    .gripperPosB = 0,
    .isValid = false
};

JogState jogState = {
    .xPos = 0,
    .yPos = 0,
    .zPos = 0,
    .jogDistance = JOG_DEFAULT_DISTANCE,
    .isJogging = false
};

bool initializeMotors() {
    // Configure stepper motors
    stepperX.setEnablePin(X_EN_PIN);
    stepperX.setPinsInverted(false, false, true);
    stepperX.setMaxSpeed(speedSettings.xSpeed);
    stepperX.setAcceleration(speedSettings.xSpeed / 2);
    
    stepperY.setEnablePin(Y_EN_PIN);
    stepperY.setPinsInverted(false, false, true);
    stepperY.setMaxSpeed(speedSettings.ySpeed);
    stepperY.setAcceleration(speedSettings.ySpeed / 2);
    
    stepperZ.setEnablePin(Z_EN_PIN);
    stepperZ.setPinsInverted(false, false, true);
    stepperZ.setMaxSpeed(speedSettings.zSpeed);
    stepperZ.setAcceleration(speedSettings.zSpeed / 2);
    
    stepperGripper.setEnablePin(GRIPPER_EN_PIN);
    stepperGripper.setPinsInverted(false, false, true);
    stepperGripper.setMaxSpeed(speedSettings.gripperSpeed);
    stepperGripper.setAcceleration(speedSettings.gripperSpeed / 2);
    
    stepperCaseFeeder.setEnablePin(CASE_FEEDER_EN_PIN);
    stepperCaseFeeder.setPinsInverted(false, false, true);
    stepperCaseFeeder.setMaxSpeed(speedSettings.caseFeederSpeed);
    stepperCaseFeeder.setAcceleration(speedSettings.caseFeederSpeed / 2);
    
    stepperPowderDropper.setEnablePin(POWDER_DROPPER_EN_PIN);
    stepperPowderDropper.setPinsInverted(false, false, true);
    stepperPowderDropper.setMaxSpeed(speedSettings.powderDropperSpeed);
    stepperPowderDropper.setAcceleration(speedSettings.powderDropperSpeed / 2);
    
    // Configure TMC2209 drivers
    configureTMC2209Drivers();
    
    // Load saved motor positions
    loadMotorPositions();
    
    // Validate motor positions
    if (!validateMotorPositions()) {
        return false;
    }
    
    return true;
}

void stopAllMotors() {
    stepperX.stop();
    stepperY.stop();
    stepperZ.stop();
    stepperGripper.stop();
    stepperCaseFeeder.stop();
    stepperPowderDropper.stop();
    
    while (stepperX.isRunning() || stepperY.isRunning() || 
           stepperZ.isRunning() || stepperGripper.isRunning() || 
           stepperCaseFeeder.isRunning() || stepperPowderDropper.isRunning()) {
        delay(10);
    }
}

bool moveToPosition(AccelStepper& stepper, long targetPos) {
    if (!isValidPosition(stepper, targetPos)) return false;
    if (checkForObstacles(stepper, targetPos)) return false;
    
    stepper.moveTo(targetPos);
    while (stepper.distanceToGo() != 0) {
        stepper.run();
        if (!monitorMovement(stepper)) {
            stopAllMotors();
            return false;
        }
    }
    
    return true;
}

bool isValidPosition(AccelStepper& stepper, long targetPos) {
    if (&stepper == &stepperX) {
        return targetPos >= X_MIN_POS && targetPos <= X_MAX_POS;
    } else if (&stepper == &stepperY) {
        return targetPos >= Y_MIN_POS && targetPos <= Y_MAX_POS;
    } else if (&stepper == &stepperZ) {
        return targetPos >= Z_MIN_POS && targetPos <= Z_MAX_POS;
    } else if (&stepper == &stepperGripper) {
        return targetPos >= GRIPPER_MIN_POS && targetPos <= GRIPPER_MAX_POS;
    }
    return false;
}

bool checkForObstacles(AccelStepper& stepper, long targetPos) {
    if (checkLimitSwitches()) return true;
    
    if (&stepper == &stepperX) {
        if (stepperY.currentPosition() < Y_SAFE_POSITION) return true;
    } else if (&stepper == &stepperY) {
        if (stepperX.currentPosition() < X_SAFE_POSITION) return true;
    } else if (&stepper == &stepperZ) {
        if (stepperX.currentPosition() < X_SAFE_POSITION) return true;
    }
    
    return false;
}

bool monitorMovement(AccelStepper& stepper) {
    static unsigned long lastCheck = 0;
    static long lastPosition = 0;
    
    if (millis() - lastCheck >= SAFE_POSITION_CHECK_INTERVAL) {
        if (stepper.currentPosition() == lastPosition) {
            return false;  // Motor is stuck
        }
        
        if (stepper.currentPosition() != lastPosition) {
            uint32_t drv_status = 0;
            if (&stepper == &stepperX) {
                drv_status = tmc2209X.getDRV_STATUS();
            } else if (&stepper == &stepperY) {
                drv_status = tmc2209Y.getDRV_STATUS();
            } else if (&stepper == &stepperZ) {
                drv_status = tmc2209Z.getDRV_STATUS();
            }
            
            // Check for driver errors
            if (drv_status & 0x00010000) {  // Driver error bit
                emergencyStop();
                return false;
            }
        }
        
        lastPosition = stepper.currentPosition();
        lastCheck = millis();
    }
    
    return true;
}

bool checkMotorOperation(AccelStepper& stepper) {
    // Check if motor is enabled
    if (!stepper.isEnabled()) {
        return false;
    }
    
    // Check if motor is running
    if (stepper.isRunning()) {
        // Check if motor is actually moving
        static long lastPos = 0;
        if (stepper.currentPosition() == lastPos) {
            return false;
        }
        lastPos = stepper.currentPosition();
    }
    
    return true;
}

bool checkLimitSwitches() {
    if (digitalRead(X_LIMIT_PIN) == LOW) return true;
    if (digitalRead(Y_LIMIT_PIN) == LOW) return true;
    if (digitalRead(Z_LIMIT_PIN) == LOW) return true;
    if (digitalRead(GRIPPER_LIMIT_PIN) == LOW) return true;
    if (digitalRead(CASE_FEEDER_LIMIT_PIN) == LOW) return true;
    return false;
}

bool checkMotorCollisions(long xPos, long zPos) {
    // Check if X and Z positions would cause a collision
    if (xPos < X_SAFE_POSITION && zPos < Z_SAFE_POSITION) {
        return true;
    }
    return false;
}

void handleJogMovement(int axis, int direction) {
    if (!jogState.isJogging) {
        jogState.isJogging = true;
    }
    
    AccelStepper* stepper = nullptr;
    long* currentPos = nullptr;
    
    switch (axis) {
        case 0: // X axis
            stepper = &stepperX;
            currentPos = &jogState.xPos;
            break;
        case 1: // Y axis
            stepper = &stepperY;
            currentPos = &jogState.yPos;
            break;
        case 2: // Z axis
            stepper = &stepperZ;
            currentPos = &jogState.zPos;
            break;
        default:
            return;
    }
    
    if (stepper != nullptr) {
        long targetPos = *currentPos + (direction * jogState.jogDistance);
        if (isValidPosition(*stepper, targetPos)) {
            moveToPosition(*stepper, targetPos);
            *currentPos = targetPos;
        }
    }
}

bool resetMotorsToSafePositions() {
    bool success = true;
    
    // Move Z to safe position first
    if (!moveToPosition(stepperZ, Z_SAFE_POSITION)) {
        success = false;
    }
    
    // Move X to safe position
    if (!moveToPosition(stepperX, X_SAFE_POSITION)) {
        success = false;
    }
    
    // Move Y to safe position
    if (!moveToPosition(stepperY, Y_SAFE_POSITION)) {
        success = false;
    }
    
    return success;
}

void configureTMC2209Driver(TMC2209& driver) {
    driver.setRunCurrent(50);  // 50% of max current
    driver.setHoldCurrent(25); // 25% of max current
    driver.setStandstillMode(TMC2209::NORMAL);
    driver.enableAutomaticCurrentScaling();
    driver.enableAutomaticGradientAdaptation();
}

void configureTMC2209Drivers() {
    configureTMC2209Driver(tmc2209X);
    configureTMC2209Driver(tmc2209Y);
    configureTMC2209Driver(tmc2209Z);
    configureTMC2209Driver(tmc2209Gripper);
    configureTMC2209Driver(tmc2209CaseFeeder);
    configureTMC2209Driver(tmc2209PowderDropper);
}

void homeSteppers() {
    // Home X axis
    stepperX.setSpeed(-speedSettings.homingSpeed);
    while (digitalRead(X_LIMIT_PIN) == HIGH) {
        stepperX.runSpeed();
    }
    stepperX.setCurrentPosition(0);
    stepperX.setSpeed(0);
    
    // Home Y axis
    stepperY.setSpeed(-speedSettings.homingSpeed);
    while (digitalRead(Y_LIMIT_PIN) == HIGH) {
        stepperY.runSpeed();
    }
    stepperY.setCurrentPosition(0);
    stepperY.setSpeed(0);
    
    // Home Z axis
    stepperZ.setSpeed(-speedSettings.homingSpeed);
    while (digitalRead(Z_LIMIT_PIN) == HIGH) {
        stepperZ.runSpeed();
    }
    stepperZ.setCurrentPosition(0);
    stepperZ.setSpeed(0);
}

void updateSpeedSettings() {
    stepperX.setMaxSpeed(speedSettings.xSpeed);
    stepperX.setAcceleration(speedSettings.xSpeed / 2);
    
    stepperY.setMaxSpeed(speedSettings.ySpeed);
    stepperY.setAcceleration(speedSettings.ySpeed / 2);
    
    stepperZ.setMaxSpeed(speedSettings.zSpeed);
    stepperZ.setAcceleration(speedSettings.zSpeed / 2);
    
    stepperGripper.setMaxSpeed(speedSettings.gripperSpeed);
    stepperGripper.setAcceleration(speedSettings.gripperSpeed / 2);
    
    stepperCaseFeeder.setMaxSpeed(speedSettings.caseFeederSpeed);
    stepperCaseFeeder.setAcceleration(speedSettings.caseFeederSpeed / 2);
    
    stepperPowderDropper.setMaxSpeed(speedSettings.powderDropperSpeed);
    stepperPowderDropper.setAcceleration(speedSettings.powderDropperSpeed / 2);
}

void saveMotorPositions() {
    motorPos.isValid = true;
    EEPROM.put(MOTOR_POS_EEPROM_ADDR, motorPos);
}

void loadMotorPositions() {
    EEPROM.get(MOTOR_POS_EEPROM_ADDR, motorPos);
}

bool validateMotorPositions() {
    if (!motorPos.isValid) {
        return false;
    }
    
    // Validate X positions
    if (motorPos.xPos1 < X_MIN_POS || motorPos.xPos1 > X_MAX_POS ||
        motorPos.xPos4 < X_MIN_POS || motorPos.xPos4 > X_MAX_POS) {
        return false;
    }
    
    // Validate Z positions
    if (motorPos.zPos2 < Z_MIN_POS || motorPos.zPos2 > Z_MAX_POS ||
        motorPos.zPos3 < Z_MIN_POS || motorPos.zPos3 > Z_MAX_POS ||
        motorPos.zPos5 < Z_MIN_POS || motorPos.zPos5 > Z_MAX_POS) {
        return false;
    }
    
    // Validate gripper positions
    if (motorPos.gripperPosA < GRIPPER_MIN_POS || motorPos.gripperPosA > GRIPPER_MAX_POS ||
        motorPos.gripperPosB < GRIPPER_MIN_POS || motorPos.gripperPosB > GRIPPER_MAX_POS) {
        return false;
    }
    
    return true;
}

void emergencyStop() {
    stopAllMotors();
    // Additional emergency stop procedures can be added here
} 