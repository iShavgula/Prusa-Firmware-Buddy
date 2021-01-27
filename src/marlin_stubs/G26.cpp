#include <algorithm>

#include "../../lib/Marlin/Marlin/src/inc/MarlinConfig.h"
#include "../../lib/Marlin/Marlin/src/feature/host_actions.h"
#include "../../lib/Marlin/Marlin/src/feature/safety_timer.h"
#include "../../lib/Marlin/Marlin/src/gcode/gcode.h"
#include "../../lib/Marlin/Marlin/src/module/motion.h"
#include "../../lib/Marlin/Marlin/src/module/temperature.h"
#include "../../lib/Marlin/Marlin/src/Marlin.h"
#include "marlin_server.hpp"
#include "client_fsm_types.h"
#include "PrusaGcodeSuite.hpp"
#include "G26.hpp"
#include "cmath_ext.h"

static const constexpr float filamentD = 1.75f;
static const constexpr float layerHeight = 0.2f;
static const constexpr float threadWidth = 0.5f;

static const constexpr float pi = 3.1415926535897932384626433832795f;

/// Path of Manhattan snake
/// Alternate X and Y coordinates.
/// It cannot print diagonals etc.
/// First X and Y are a starting point.
static const constexpr float snake1[] = {
    /// use 0.5 extrusion width
    10,  /// start X
    150, /// start Y
    170, /// X
    130, /// Y
    10,  /// X
    110, /// ...
    170,
    90,
    10,
    70,
    170,
    50,
    ///
    /// frame around
    9.5,
    17,
    30.5,
    30.5,
    10,
    30,
    ///
    /// infill
    30,
    29.5,
    10,
    29,
    30,
    28.5,
    10,
    28,
    30,
    27.5,
    10,
    27,
    30,
    26.5,
    10,
    26,
    30,
    25.5,
    10,
    25,
    30,
    24.5,
    10,
    24,
    30,
    23.5,
    10,
    23,
    30,
    22.5,
    10,
    22,
    30,
    21.5,
    10,
    21,
    30,
    20.5,
    10,
    20,
    30,
    19.5,
    10,
    19,
    30,
    18.5,
    10,
    18,
    30,
    17.5,
    10,
};

/// Path of Manhattan snake
/// Alternate X and Y coordinates.
/// It cannot print diagonals etc.
/// First X and Y are a starting point.
static const constexpr float snake2[] = {
    /// use 0.5 extrusion width
    10,        /// start X
    212 - 162, /// start Y
    240,       /// X
    212 - 132, /// Y
    10,        /// X
    212 - 112, /// ...
    240,
    212 - 92,
    10,
    212 - 72,
    240,
    212 - 52,
    ///
    /// frame around
    9.5,
    212 - 17,
    30.5,
    212 - 30.5,
    10,
    212 - 30,
    ///
    /// infill
    30,
    212 - 29.5,
    10,
    212 - 29,
    30,
    212 - 28.5,
    10,
    212 - 28,
    30,
    212 - 27.5,
    10,
    212 - 27,
    30,
    212 - 26.5,
    10,
    212 - 26,
    30,
    212 - 25.5,
    10,
    212 - 25,
    30,
    212 - 24.5,
    10,
    212 - 24,
    30,
    212 - 23.5,
    10,
    212 - 23,
    30,
    212 - 22.5,
    10,
    212 - 22,
    30,
    212 - 21.5,
    10,
    212 - 21,
    30,
    212 - 20.5,
    10,
    212 - 20,
    30,
    212 - 19.5,
    10,
    212 - 19,
    30,
    212 - 18.5,
    10,
    212 - 18,
    30,
    212 - 17.5,
    10,
};

bool FirstLayer::isPrinting_ = false;

void FirstLayer::finish_printing() {
    current_line = 1;
    total_lines = 1; /// don't set 0 to avoid division by zero
}

void FirstLayer::plan_destination(const float x, const float y, const float z, const float e, const float f) {
    if (isfinite(x))
        destination[0] = x;
    else
        destination[0] = current_position[0];

    if (isfinite(y))
        destination[1] = y;
    else
        destination[1] = current_position[1];

    if (isfinite(z))
        destination[2] = z;
    else
        destination[2] = current_position[2];

    if (isfinite(e))
        destination[3] = current_position[3] + e;
    else
        destination[3] = current_position[3];

    if (isfinite(f))
        feedrate_mm_s = f / 60.f;

    prepare_move_to_destination();
}

void FirstLayer::inc_progress() {
    current_line++;
    const uint8_t progress = std::min(uint8_t(100), uint8_t(100.f * current_line / (float)total_lines));
    if (last_progress != progress) {
        last_progress = progress;
        set_var_sd_percent_done(progress);
    }

    // const variant8_t var = variant8_i8(100 * current_line / (float)total_lines);
    // marlin_set_var(MARLIN_VAR_SD_PDONE, var);
}

void FirstLayer::go_to_destination(const float x, const float y, const float z, const float e, const float f) {
    plan_destination(x, y, z, e, f);
    wait_for_move();
    inc_progress();
}

constexpr float FirstLayer::extrusion(const float x1, const float y1, const float x2, const float y2, const float layerHeight, const float threadWidth) const {
    const float length = sqrt(SQR(x2 - x1) + SQR(y2 - y1));
    return length * layerHeight * threadWidth / (pi * SQR(filamentD / 2));
}

constexpr float FirstLayer::extrusion_Manhattan(const float *path, const uint32_t position, const float last) const {
    if (position % 2 == 0) {
        const float x = path[position];
        const float y = path[position - 2];
        return extrusion(x, y, last, y, layerHeight, threadWidth);
    } else {
        const float x = path[position - 1];
        const float y = path[position];
        return extrusion(x, y, x, last, layerHeight, threadWidth);
    }
}

void FirstLayer::print_snake(const float *snake, const size_t snake_size, const float speed) {

    /// move to start
    plan_destination(snake[0], snake[1], 0); // Process X Y Z E F parameters
    float last_x = snake[0];
    float last_y = snake[1];
    /// iterate positions
    size_t i;
    for (i = 2; i < snake_size - 1; i += 2) { /// snake_size-1 because we need 2 items
        go_to_destination(snake[i], NAN, NAN, extrusion_Manhattan(snake, i, last_x), speed);
        last_x = snake[i];
        go_to_destination(NAN, snake[i + 1], NAN, extrusion_Manhattan(snake, i + 1, last_y), speed);
        last_y = snake[i + 1];
    }
    if (i == snake_size - 1) { /// process last X movement
        plan_destination(snake[i], NAN, NAN, extrusion_Manhattan(snake, i, last_x), speed);
    }
}

void FirstLayer::print_shape_1() {
    total_lines = 11 + ARRAY_SIZE(snake1);
    current_line = 0;

    /// print purge line
    go_to_destination(NAN, NAN, 4.f, NAN, 1000.f);
    go_to_destination(0.f, -2.f, 0.2f, NAN, 3000.f);
    go_to_destination(NAN, NAN, NAN, 6.f, 2000.f);
    go_to_destination(60.f, NAN, NAN, 9.f, 1000.f);
    go_to_destination(100.f, NAN, NAN, 12.5f, 1000.f);
    go_to_destination(NAN, NAN, 2.f, -6.f, 2100.f);

    /// go to starting point and de-retract
    go_to_destination(10.f, 150.f, 0.2f, NAN, 3000.f);
    go_to_destination(NAN, NAN, NAN, 6.f, 2000.f);
    go_to_destination(NAN, NAN, NAN, NAN, 1000.f);

    print_snake(snake1, ARRAY_SIZE(snake1), 1000.f);

    /// finish printing
    go_to_destination(NAN, NAN, 2.f, -6.f, 2100.f);
    go_to_destination(178.f, 0.f, 10.f, NAN, 3000.f);

    finish_printing();
}

void FirstLayer::print_shape_2() {
    enable_all_steppers();
    //M221 S100 ; reset flow
    planner.flow_percentage[0] = 100;
    planner.refresh_e_factor(0);
    /// fixed lines - constant to show 100% at the end + calibration pattern
    total_lines = 8 - 3 + ARRAY_SIZE(snake2);
    current_line = 0;

    /// print purge line
    go_to_destination(NAN, NAN, 4.f, NAN, 1000.f);
    go_to_destination(NAN, -2.f, 0.2f, NAN, 3000.f);
    go_to_destination(0.f, NAN, 0.2f, NAN, 3000.f);
    go_to_destination(NAN, NAN, NAN, 6.f, 2000.f);
    go_to_destination(60.f, NAN, NAN, 9.f, 1000.f);
    go_to_destination(100.f, NAN, NAN, 12.5f, 1000.f);
    go_to_destination(10.f, 30.f, NAN, extrusion(100.0f, -2.f, 10.f, 30.f), 1000.f);

    print_snake(snake2, ARRAY_SIZE(snake2), 1000.f);

    /// finish printing
    //go_to_destination(NAN, NAN, 2.f, -6.f, 2100.f);
    //go_to_destination(178.f, 180.f, 10.f, NAN, 3000.f);

    //TYPE:Custom
    // Filament-specific end gcode
    //TODO setprecent? ////M73 P94 R0
    go_to_destination(NAN, NAN, NAN, -1.f, 2100.f);    // G1 E-1 F2100 ; retract
    go_to_destination(NAN, NAN, 2.2f, NAN, 720.f);     // G1 Z2.2 F720 ; Move print head up
    go_to_destination(178.f, 178.f, NAN, NAN, 4200.f); // G1 X178 Y178 F4200 ; park print head
    //TODO setprecent? ////M73 P96 R0
    go_to_destination(NAN, NAN, 30.2f, NAN, 720.f); // G1 Z30.2 F720 ; Move print head further up
    planner.synchronize();                          // G4 ; wait .. finish moves == M400
    thermalManager.setTargetHotend(0, 0);           // M104 S0 ; turn off temperature
    thermalManager.setTargetBed(0);                 // M140 S0 ; turn off heatbed
    thermalManager.set_fan_speed(0, 0);             //M107 ; turn off fan

    //no need lro reset linear advance, was not set // M900 K0 ; reset LA
    planner.finish_and_disable(); // M84 ; disable motors
    //TODO setprecent? // M73 P100 R0
    finish_printing();
}

void PrusaGcodeSuite::G26() {
    if (all_axes_known()) { /// checks if axes are calibrated (homed) before
        FirstLayer fl;
        //fl.print_shape_1();
        fl.print_shape_2();
    }
}
