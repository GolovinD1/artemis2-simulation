#include "physics.h"

Vector computeAcceleration(const Vector& pos_ship, const Vector& pos_moon) {

    double r_earth_norm = pos_ship.length();
    Vector a_earth = -MU_EARTH * pos_ship / (r_earth_norm * r_earth_norm * r_earth_norm);

    Vector r_moon_to_ship = pos_ship - pos_moon;
    double r_moon_norm = r_moon_to_ship.length();

    Vector a_moon = -MU_MOON * r_moon_to_ship / (r_moon_norm * r_moon_norm * r_moon_norm);

    return a_earth + a_moon;
}

State rk4_step(const State& current, const Vector& pos_moon, double dt) {
    // k1
    Vector k1_r = current.velocity;
    Vector k1_v = computeAcceleration(current.position, pos_moon);

    // k2 
    Vector pos_mid1 = current.position + k1_r * (dt / 2.0);
    Vector vel_mid1 = current.velocity + k1_v * (dt / 2.0);
    Vector k2_r = vel_mid1;
    Vector k2_v = computeAcceleration(pos_mid1, pos_moon);

    // k3
    Vector pos_mid2 = current.position + k2_r * (dt / 2.0);
    Vector vel_mid2 = current.velocity + k2_v * (dt / 2.0);
    Vector k3_r = vel_mid2;
    Vector k3_v = computeAcceleration(pos_mid2, pos_moon);

    // k4
    Vector pos_end = current.position + k3_r * dt;
    Vector vel_end = current.velocity + k3_v * dt;
    Vector k4_r = vel_end;
    Vector k4_v = computeAcceleration(pos_end, pos_moon);

    State next;
    // y = y + (k1 + 2*k2 + 2*k3 + k4) * (dt / 6.0)
    next.position = current.position + (k1_r + k2_r * 2.0 + k3_r * 2.0 + k4_r) * (dt / 6.0);
    next.velocity = current.velocity + (k1_v + k2_v * 2.0 + k3_v * 2.0 + k4_v) * (dt / 6.0);
    next.time = current.time + dt;

    return next;
}

