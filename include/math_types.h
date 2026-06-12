#pragma once 
#include <cmath>
#include <iostream>

struct Vector {
    double x = 0;
    double y = 0;
    double z = 0;

    double length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    Vector operator+(const Vector& other) const {
        return { x + other.x, y + other.y, z + other.z };
    }

    Vector operator-(const Vector& other) const {
        return { x - other.x, y - other.y, z - other.z };
    }

    Vector operator*(const double c) const {
        return { x * c, y * c, z * c };
    }

    Vector operator/(const double c) const {
        return { x / c, y / c, z / c };
    }
};

inline Vector operator*(double c, const Vector& v) {
    return v * c;
}

inline std::ostream& operator<<(std::ostream& os, const Vector& v) {
    os << "[" << v.x << ", " << v.y << ", " << v.z << "]";
    return os;
}

struct State {
    Vector position;   // (X, Y, Z)
    Vector velocity;   // (Vx, Vy, Vz)
    double time = 0.0;
    std::string date;
};