/*
 * Vec3.h - 3D Vector class for mathematical operations
 */

#ifndef VEC3_H
#define VEC3_H

#include <cmath>

 // Define M_PI if not defined (MSVC doesn't define it by default)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Vec3 {
public:
    float x, y, z;

    // Constructors
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    // Vector operations
    Vec3 operator+(const Vec3& v) const {
        return Vec3(x + v.x, y + v.y, z + v.z);
    }

    Vec3 operator-(const Vec3& v) const {
        return Vec3(x - v.x, y - v.y, z - v.z);
    }

    // Scalar multiplication
    Vec3 operator*(float s) const {
        return Vec3(x * s, y * s, z * s);
    }

    // Component-wise vector multiplication
    Vec3 operator*(const Vec3& v) const {
        return Vec3(x * v.x, y * v.y, z * v.z);
    }

    Vec3 operator/(float s) const {
        return Vec3(x / s, y / s, z / s);
    }

    Vec3& operator+=(const Vec3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Vec3& operator-=(const Vec3& v) {
        x -= v.x; y -= v.y; z -= v.z;
        return *this;
    }

    Vec3& operator*=(float s) {
        x *= s; y *= s; z *= s;
        return *this;
    }

    // Component-wise multiplication assignment
    Vec3& operator*=(const Vec3& v) {
        x *= v.x; y *= v.y; z *= v.z;
        return *this;
    }

    Vec3& operator/=(float s) {
        x /= s; y /= s; z /= s;
        return *this;
    }

    // Dot product
    float dot(const Vec3& v) const {
        return x * v.x + y * v.y + z * v.z;
    }

    // Cross product
    Vec3 cross(const Vec3& v) const {
        return Vec3(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }

    // Length
    float length() const {
        return sqrt(x * x + y * y + z * z);
    }

    // Squared length (faster when only comparing)
    float lengthSquared() const {
        return x * x + y * y + z * z;
    }

    // Normalize
    Vec3 normalized() const {
        float len = length();
        if (len > 0.0001f) {
            return *this / len;
        }
        return Vec3(0, 0, 0);
    }

    void normalize() {
        float len = length();
        if (len > 0.0001f) {
            x /= len;
            y /= len;
            z /= len;
        }
    }

    // Distance between two points
    static float distance(const Vec3& a, const Vec3& b) {
        return (b - a).length();
    }

    // Linear interpolation
    static Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
        return a + (b - a) * t;
    }
};

#endif // VEC3_H