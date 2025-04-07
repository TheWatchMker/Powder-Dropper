#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <AccelStepper.h>
#include <TMC2209.h>

// Motor pin definitions
#define X_STEP_PIN 2
#define X_DIR_PIN 3
#define X_EN_PIN 4
#define X_LIMIT_PIN 5
#define Y_STEP_PIN 22
#define Y_DIR_PIN 23
#define Y_EN_PIN 24
#define Y_LIMIT_PIN 25
#define Z_STEP_PIN 6
#define Z_DIR_PIN 7
#define Z_EN_PIN 8
#define Z_LIMIT_PIN 9
#define GRIPPER_STEP_PIN 26
#define GRIPPER_DIR_PIN 27
#define GRIPPER_EN_PIN 28
#define GRIPPER_LIMIT_PIN 29
#define CASE_FEEDER_STEP_PIN 19
#define CASE_FEEDER_DIR_PIN 20
#define CASE_FEEDER_LIMIT_PIN 33
#define POWDER_DROPPER_STEP_PIN 16
#define POWDER_DROPPER_DIR_PIN 17
#define POWDER_DROPPER_EN_PIN 18

// Motor position limits
#define X_MIN_POS 0
#define X_MAX_POS 235000  // 235mm * 80 steps/mm
#define Y_MIN_POS 0
#define Y_MAX_POS 235000  // 235mm * 80 steps/mm
#define Z_MIN_POS 0
#define Z_MAX_POS 250000  // 250mm * 80 steps/mm
#define GRIPPER_MIN_POS 0
#define GRIPPER_MAX_POS 1000

// Safe positions
#define X_SAFE_POSITION 117500  // X_MAX_POS / 2
#define Y_SAFE_POSITION 117500  // Y_MAX_POS / 2
#define Z_SAFE_POSITION 125000  // Z_MAX_POS / 2

// Motor timing constants
#define MOTOR_TIMEOUT 10000  // 10 seconds
#define SAFE_POSITION_CHECK_INTERVAL 100  // Check every 100ms
#define MAX_MOTOR_CURRENT 2000  // Maximum allowed motor current in mA

// Jogging constants
#define JOG_SPEED 1000  // Steps per second
#define JOG_ACCELERATION 500  // Steps per second squared
#define JOG_DEFAULT_DISTANCE 1000  // Default jog distance in steps

// Add these constants for TMC2209 configuration
#define TMC2209_RUN_CURRENT 50    // 50% of max current
#define TMC2209_HOLD_CURRENT 25   // 25% of max current
#define TMC2209_MICROSTEPS 16     // Microstepping setting

// Motor speed settings structure
struct SpeedSettings {
    long xSpeed;
    long ySpeed;
    long zSpeed;
    long gripperSpeed;
    long powderDropperSpeed;
    long caseFeederSpeed;
    uint8_t vibratorySpeed;
    long powderCoarseSpeed;
    long powderFineSpeed;
    long homingSpeed;
    long homingAcceleration;
};

// Motor positions structure
struct MotorPositions {
    long xPos1;  // Above case
    long xPos4;  // Center of scale
    long zPos2;  // Case pickup
    long zPos3;  // Above bed
    long zPos5;  // Case drop-off
    long gripperPosA;  // Open position
    long gripperPosB;  // Closed position
    bool isValid;
};

// Jogging state structure
struct JogState {
    long xPos;
    long yPos;
    long zPos;
    long jogDistance;
    bool isJogging;
};

// External declarations for global variables
extern AccelStepper stepperX;
extern AccelStepper stepperY;
extern AccelStepper stepperZ;
extern AccelStepper stepperGripper;
extern AccelStepper stepperCaseFeeder;
extern AccelStepper stepperPowderDropper;

extern TMC2209 tmc2209X;
extern TMC2209 tmc2209Y;
extern TMC2209 tmc2209Z;
extern TMC2209 tmc2209Gripper;
extern TMC2209 tmc2209CaseFeeder;
extern TMC2209 tmc2209PowderDropper;

extern SpeedSettings speedSettings;
extern MotorPositions motorPos;
extern JogState jogState;

// Function declarations
void stopAllMotors();
bool moveToPosition(AccelStepper& stepper, long targetPos);
bool isValidPosition(AccelStepper& stepper, long targetPos);
bool checkForObstacles(AccelStepper& stepper, long targetPos);
bool monitorMovement(AccelStepper& stepper);
bool checkMotorOperation(AccelStepper& stepper);
bool checkLimitSwitches();
bool checkMotorCollisions(long xPos, long zPos);
void handleJogMovement(int axis, int direction);
bool resetMotorsToSafePositions();
void configureTMC2209Driver(TMC2209& driver);
void configureTMC2209Drivers();
void homeSteppers();
void updateSpeedSettings();
bool initializeMotors();
void saveMotorPositions();
void loadMotorPositions();
bool validateMotorPositions();
void emergencyStop();

#endif // MOTOR_CONTROL_H 