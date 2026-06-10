#pragma once
#include "math_types.h"

const double MU_EARTH = 3.986004418e14;
const double MU_MOON = 4.902800066e12;

Vector computeAcceleration(const Vector& pos_ship, const Vector& pos_moon);

struct StepResult {
    State state;
    double next_dt;
};

State rk4_step(const State& current, const Vector& pos_moon, double dt);