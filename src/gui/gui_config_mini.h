// gui_config_mini.h
#pragma once

#include "../../include/external_config.h"

// axis length [mm] - PRINTER MINI
static const uint8_t X_LEN = X_LENGTH;
static const uint8_t Y_LEN = Y_LENGTH;
static const uint8_t Z_LEN = Z_LENGTH;

// tolerance (common for all axes)
static const uint8_t LEN_TOL_ABS = 15; // length absolute tolerance (+-5mm)
static const uint8_t LEN_TOL_REV = 13; // length tolerance in reversed direction (3mm)

// static const float Z_OFFSET_STEP = 0.0025F; //calculated
static const float Z_OFFSET_MIN = -2.0F;
static const float Z_OFFSET_MAX = 0.0F;
