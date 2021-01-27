#include "external_config_vars.h"

// The physical length of the axis
#define X_LENGTH 255
#define Y_LENGTH 218
#define Z_LENGTH 210

// The size of the print bed
#define X_BED_SIZE 250
#define Y_BED_SIZE 212

// Travel limits (mm) after homing, corresponding to endstop positions.
#define X_MIN_POS X_LENGTH - X_BED_SIZE - 5 // 0
#define Y_MIN_POS Y_LENGTH - Y_BED_SIZE - 10 // -4
#define Z_MIN_POS 0
#define X_MAX_POS X_BED_SIZE
#define Y_MAX_POS Y_BED_SIZE
#define Z_MAX_POS 210

#define X_STEPPER_RES STEPPER_RES_1_8 // STEPPER_RES_0_9
#define Y_STEPPER_RES STEPPER_RES_1_8
#define Z_STEPPER_RES STEPPER_RES_1_8
#define E_STEPPER_RES STEPPER_RES_1_8

// Extruder steps per unit
#define E_STEPS_PER_UNIT 140

#define E_COOLING_FAN_NOCTUA
