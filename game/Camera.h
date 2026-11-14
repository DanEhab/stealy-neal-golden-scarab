/*
 * Camera.h - First-person and third-person camera system
 */

#ifndef CAMERA_H
#define CAMERA_H

#include <windows.h>
#include <glut.h>
#include <stdlib.h>
#include "Vec3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum CameraMode {
    FIRST_PERSON,
    THIRD_PERSON
};

class Camera {
private:
    Vec3 position;
    Vec3 forward;
    Vec3 up;
    Vec3 right;

    float yaw;      // Horizontal rotation
    float pitch;    // Vertical rotation

    CameraMode mode;

    // Camera sensitivity
    float mouseSensitivity;

    // Third person camera settings
    float thirdPersonDistance;
    float thirdPersonHeight;
    
    // Camera shake variables
    bool isShaking;
    float shakeIntensity;
    float shakeDuration;
    float shakeTimer;
    Vec3 shakeOffset;

    void updateVectors() {
        // Calculate new forward vector
        float yawRad = yaw * M_PI / 180.0f;
        float pitchRad = pitch * M_PI / 180.0f;

        forward.x = cos(yawRad) * cos(pitchRad);
        forward.y = sin(pitchRad);
        forward.z = sin(yawRad) * cos(pitchRad);
        forward.normalize();

        // Calculate right vector
        right = forward.cross(Vec3(0, 1, 0)).normalized();

        // Calculate up vector
        up = right.cross(forward).normalized();
    }

public:
    Camera() :
        position(0, 1.7f, 0),
        forward(0, 0, -1),
        up(0, 1, 0),
        right(1, 0, 0),
        yaw(-90.0f),
        pitch(0.0f),
        mode(FIRST_PERSON),
        mouseSensitivity(0.15f),
        thirdPersonDistance(5.0f),
        thirdPersonHeight(2.0f),
        isShaking(false),
        shakeIntensity(0.0f),
        shakeDuration(0.0f),
        shakeTimer(0.0f),
        shakeOffset(0, 0, 0)
    {
        updateVectors();
    }
    
    // Update camera shake effect
    void updateShake(float deltaTime) {
        if (isShaking) {
            shakeTimer += deltaTime;
            
            if (shakeTimer >= shakeDuration) {
                // Shake finished
                isShaking = false;
                shakeOffset = Vec3(0, 0, 0);
                shakeTimer = 0.0f;
            } else {
                // Calculate shake offset with decreasing intensity
                float progress = shakeTimer / shakeDuration;
                float currentIntensity = shakeIntensity * (1.0f - progress);
                
                // Random shake in all directions
                shakeOffset.x = ((rand() % 200 - 100) / 100.0f) * currentIntensity;
                shakeOffset.y = ((rand() % 200 - 100) / 100.0f) * currentIntensity;
                shakeOffset.z = ((rand() % 200 - 100) / 100.0f) * currentIntensity;
            }
        }
    }
    
    // Trigger camera shake
    void shake(float intensity = 0.15f, float duration = 0.3f) {
        isShaking = true;
        shakeIntensity = intensity;
        shakeDuration = duration;
        shakeTimer = 0.0f;
    }

    // Apply camera transformation to OpenGL
    void apply(const Vec3& playerPos) {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        if (mode == FIRST_PERSON) {
            // First person: camera at player position + shake offset
            Vec3 shakenPos = position + shakeOffset;
            Vec3 center = shakenPos + forward;
            gluLookAt(
                shakenPos.x, shakenPos.y, shakenPos.z,
                center.x, center.y, center.z,
                up.x, up.y, up.z
            );
        }
        else {
            // Third person: camera behind and above player + shake offset
            Vec3 offset = forward * (-thirdPersonDistance);
            offset.y += thirdPersonHeight;
            Vec3 camPos = playerPos + offset + shakeOffset;

            gluLookAt(
                camPos.x, camPos.y, camPos.z,
                playerPos.x, playerPos.y + 1.0f, playerPos.z,
                0, 1, 0
            );
        }
    }

    // Handle mouse movement
    void handleMouseMove(float deltaX, float deltaY) {
        yaw += deltaX * mouseSensitivity;
        pitch -= deltaY * mouseSensitivity;

        // Clamp pitch to avoid camera flipping
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateVectors();
    }

    // Toggle between first and third person
    void toggleMode() {
        mode = (mode == FIRST_PERSON) ? THIRD_PERSON : FIRST_PERSON;
    }

    // Getters
    Vec3 getPosition() const { return position; }
    Vec3 getForward() const { return forward; }
    Vec3 getRight() const { return right; }
    Vec3 getUp() const { return up; }
    CameraMode getMode() const { return mode; }
    float getYaw() const { return yaw; }

    // Setters
    void setPosition(const Vec3& pos) { position = pos; }
    void setYaw(float y) { yaw = y; updateVectors(); }
    void setPitch(float p) { pitch = p; updateVectors(); }
};

#endif // CAMERA_H