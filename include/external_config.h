#include "external_config_vars.h"

// The physical length of the axis
#define X_LENGTH 255
#define Y_LENGTH 218
#define Z_LENGTH 204

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

/**
 *   Z Probe to nozzle (X,Y) offset, relative to (0, 0).
 *   X and Y offsets must be integers.
 *
 *   In the following example the X and Y offsets are both positive:
 *   #define X_PROBE_OFFSET_FROM_EXTRUDER 10
 *   #define Y_PROBE_OFFSET_FROM_EXTRUDER 10
 *
 *      +-- BACK ---+
 *      |           |
 *    L |    (+) P  | R <-- probe (20,20)
 *    E |           | I
 *    F | (-) N (+) | G <-- nozzle (10,10)
 *    T |           | H
 *      |    (-)    | T
 *      |           |
 *      O-- FRONT --+
 *    (0,0)
 */
#define NOZZLE_TO_PROBE_OFFSET \
    { 23, 5, 0 }

// Certain types of probes need to stay away from edges
#define MIN_PROBE_EDGE 5

#define X_STEPPER_RES STEPPER_RES_1_8 // STEPPER_RES_0_9
#define Y_STEPPER_RES STEPPER_RES_1_8
#define Z_STEPPER_RES STEPPER_RES_1_8
#define E_STEPPER_RES STEPPER_RES_1_8

#define X_STEPPER_CURRENT 380
#define Y_STEPPER_CURRENT 400
#define Z_STEPPER_CURRENT 450
#define E_STEPPER_CURRENT 500

// Extruder steps per unit
#define E_STEPS_PER_UNIT 140

#define E_COOLING_FAN_NOCTUA
