/*
 * SecurityDevices.h - Security cameras and laser grids with SOUNDS
 * UPDATED: Added beeping sounds for cameras
 */

#ifndef SECURITYDEVICES_H
#define SECURITYDEVICES_H

#include <windows.h>
#include <glut.h>
#include <stdlib.h>
#include "Vec3.h"
#include "GameObject.h"
#include <cmath>
#include "SoundPlayer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Model* enemyModel;
float modelScale;

// Forward declare
extern SoundPlayer* globalSoundPlayer;

// ? Security Camera WITH BEEPING
class SecurityCamera : public GameObject {
private:
    float rotationRange;
    float rotationSpeed;
    float currentAngle;
    float baseYaw;
    bool rotatingRight;
    float detectionDistance;
    float detectionAngle;
    Model* cameraModel;

    // ? SOUND VARIABLES
    float beepTimer;
    float beepInterval;
    bool playedAlarmSound;

public:
    SecurityCamera(const Vec3& pos, float yaw = 0.0f) {
        position = pos;
        baseYaw = yaw;
        currentAngle = 0.0f;
        rotationRange = 60.0f;
        rotationSpeed = 30.0f;
        rotatingRight = true;
        detectionDistance = 8.0f;
        detectionAngle = 30.0f;

        // Sound init
        beepTimer = 0.0f;
        beepInterval = 2.0f;  // Beep every 2 seconds
        playedAlarmSound = false;

        cameraModel = ModelResourceManager::getInstance().getModel("models/surveillance_camera.obj");

        if (!cameraModel) {
            printf("ERROR: Failed to load camera model\n");
        }
        else {
            cameraModel->autoScaleToHeight(1.0f);
            modelScale = cameraModel->getScale().x;
            Vec3 minBounds, maxBounds;
            cameraModel->getScaledBounds(minBounds, maxBounds);
            printf("Camera model loaded\n");
        }
    }

    void update(float deltaTime) override {
        if (!active) return;

        // ? BEEPING SOUND (idle patrol beep)
        beepTimer += deltaTime;
        if (beepTimer >= beepInterval) {
            beepTimer = 0.0f;

            // Play beep sound
            if (globalSoundPlayer) {
                /*globalSoundPlayer->playFile("camera_beep.wav", true);*/
            }
        }

        // Sweep back and forth
        if (rotatingRight) {
            currentAngle += rotationSpeed * deltaTime;
            if (currentAngle > rotationRange) {
                currentAngle = rotationRange;
                rotatingRight = false;
            }
        }
        else {
            currentAngle -= rotationSpeed * deltaTime;
            if (currentAngle < -rotationRange) {
                currentAngle = -rotationRange;
                rotatingRight = true;
            }
        }
    }

    void render() override {
        if (!active) return;

        float totalYaw = baseYaw + currentAngle;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(totalYaw, 0, 1, 0);

        renderDetectionCone();

        if (cameraModel && cameraModel->hasMaterials()) {
            cameraModel->render();
        }
        else {
            glColor3f(0.2f, 0.2f, 0.2f);
            GLUquadricObj* quad = gluNewQuadric();

            glPushMatrix();
            glRotatef(-90, 1, 0, 0);
            gluCylinder(quad, 0.15f, 0.15f, 0.3f, 16, 1);
            glPopMatrix();

            glPushMatrix();
            glTranslatef(0, 0, 0.15f);
            glColor3f(0.1f, 0.1f, 0.3f);
            gluSphere(quad, 0.2f, 16, 16);
            glPopMatrix();

            gluDeleteQuadric(quad);
        }

        glPopMatrix();
    }

    void renderDetectionCone() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING);

        glColor4f(1.0f, 1.0f, 0.5f, 0.3f);

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);

        int segments = 20;
        for (int i = 0; i <= segments; i++) {
            float angle = (-detectionAngle + (2.0f * detectionAngle * i / segments)) * M_PI / 180.0f;
            float x = sin(angle) * detectionDistance;
            float z = cos(angle) * detectionDistance;
            glVertex3f(x, -position.y, z);
        }

        glEnd();

        glColor4f(1.0f, 1.0f, 0.3f, 0.8f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(0, 0, 0);
        for (int i = 0; i <= segments; i++) {
            float angle = (-detectionAngle + (2.0f * detectionAngle * i / segments)) * M_PI / 180.0f;
            float x = sin(angle) * detectionDistance;
            float z = cos(angle) * detectionDistance;
            glVertex3f(x, -position.y, z);
        }
        glEnd();
        glLineWidth(1.0f);

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
    }

    bool detectPlayer(const Vec3& playerPos) {
        if (!active) return false;

        Vec3 toPlayer = playerPos - position;
        float distance = toPlayer.length();

        // TIGHTENED: Reduced detection distance slightly for more precise detection
        if (distance > detectionDistance * 0.95f) {  // 5% tighter
            playedAlarmSound = false;
            return false;
        }

        float totalYaw = (baseYaw + currentAngle) * M_PI / 180.0f;
        Vec3 forward(sin(totalYaw), 0, cos(totalYaw));

        Vec3 toPlayer2D = toPlayer;
        toPlayer2D.y = 0;
        toPlayer2D.normalize();

        float dotProduct = forward.dot(toPlayer2D);
        
        // Clamp to avoid floating point errors with acos
        if (dotProduct > 1.0f) dotProduct = 1.0f;
        if (dotProduct < -1.0f) dotProduct = -1.0f;
        
        float angleToPlayer = acos(dotProduct) * 180.0f / M_PI;

        // TIGHTENED: Reduced detection angle by 5 degrees (30° -> 25°)
        bool detected = angleToPlayer < (detectionAngle - 5.0f);

        // ? PLAY ALARM SOUND WHEN DETECTING
        if (detected && !playedAlarmSound) {
            if (globalSoundPlayer) {
                globalSoundPlayer->playFile("alarm.wav", true);
            }
            playedAlarmSound = true;
        }
        else if (!detected) {
            playedAlarmSound = false;
        }

        return detected;
    }
};

// Laser Grid (unchanged functionality)
class LaserGrid : public GameObject {
private:
    Vec3 endPosition;
    float moveDistance;
    float moveSpeed;
    float currentOffset;
    bool movingPositive;
    int axis;

public:
    LaserGrid(const Vec3& start, const Vec3& end, int moveAxis = 2) {
        position = start;
        endPosition = end;
        axis = moveAxis;
        moveDistance = 3.0f;
        moveSpeed = 1.5f;
        currentOffset = 0.0f;
        movingPositive = true;
    }

    void update(float deltaTime) override {
        if (!active) return;

        if (movingPositive) {
            currentOffset += moveSpeed * deltaTime;
            if (currentOffset > moveDistance) {
                currentOffset = moveDistance;
                movingPositive = false;
            }
        }
        else {
            currentOffset -= moveSpeed * deltaTime;
            if (currentOffset < 0) {
                currentOffset = 0;
                movingPositive = true;
            }
        }
    }

    void render() override {
        if (!active) return;

        Vec3 start = position;
        Vec3 end = endPosition;

        if (axis == 0) {
            start.x += currentOffset;
            end.x += currentOffset;
        }
        else if (axis == 1) {
            start.y += currentOffset;
            end.y += currentOffset;
        }
        else {
            start.z += currentOffset;
            end.z += currentOffset;
        }

        glDisable(GL_LIGHTING);

        glColor3f(1.0f, 0.0f, 0.0f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex3f(start.x, start.y, start.z);
        glVertex3f(end.x, end.y, end.z);
        glEnd();

        glColor3f(0.8f, 0.1f, 0.1f);
        glPushMatrix();
        glTranslatef(start.x, start.y, start.z);
        GLUquadricObj* quad = gluNewQuadric();
        gluSphere(quad, 0.15f, 8, 8);
        gluDeleteQuadric(quad);
        glPopMatrix();

        glPushMatrix();
        glTranslatef(end.x, end.y, end.z);
        quad = gluNewQuadric();
        gluSphere(quad, 0.15f, 8, 8);
        gluDeleteQuadric(quad);
        glPopMatrix();

        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    bool detectPlayer(const Vec3& playerPos, float playerRadius) {
        if (!active) return false;

        Vec3 start = position;
        Vec3 end = endPosition;

        if (axis == 0) {
            start.x += currentOffset;
            end.x += currentOffset;
        }
        else if (axis == 1) {
            start.y += currentOffset;
            end.y += currentOffset;
        }
        else {
            start.z += currentOffset;
            end.z += currentOffset;
        }

        Vec3 line = end - start;
        float lineLength = line.length();
        line.normalize();

        Vec3 toPlayer = playerPos - start;
        float projection = toPlayer.dot(line);

        if (projection < 0) projection = 0;
        if (projection > lineLength) projection = lineLength;

        Vec3 closestPoint = start + line * projection;
        float distance = Vec3::distance(playerPos, closestPoint);

        // UPDATED: More generous than previous attempt (0.15f -> 0.25f)
        // Player radius multiplier: 0.8 (was 0.7, now closer to original 1.0)
        // This makes lasers clearly triggerable but still ~20% tighter than original
        return distance < (playerRadius * 0.8f + 0.25f);
    }
};

// Motion Sensor
class MotionSensor : public GameObject {
private:
    float detectionRadius;
    float pulseTime;
    float pulseSpeed;
    float playerInZoneTime;  // How long player has been in detection zone
    float timeToDeath;       // Time needed in zone to trigger death
    bool playerInZone;
    Vec3 lastPlayerPos;
    float beepTimer;
    float beepInterval;
    float currentBeepSpeed;

public:
    MotionSensor(const Vec3& pos) {
        position = pos;
        detectionRadius = 4.0f;  // Increased radius for better visualization
        pulseTime = 0.0f;
        pulseSpeed = 3.0f;
        playerInZoneTime = 0.0f;
        timeToDeath = 3.0f;  // 3 seconds in zone = death
        playerInZone = false;
        beepTimer = 0.0f;
        beepInterval = 2.0f;  // Start with slow beeps
        currentBeepSpeed = 1.0f;

        enemyModel = ModelResourceManager::getInstance().getModel("models/Motion_sensor.obj");
        modelScale = 1.0f;
        if (!enemyModel) {
            printf("ERROR: Failed to load motion sensor model\n");
        }
        else {
            enemyModel->autoScaleToHeight(1.0f);
            modelScale = enemyModel->getScale().x;
            Vec3 minBounds, maxBounds;
            enemyModel->getScaledBounds(minBounds, maxBounds);
            printf("Motion sensor loaded\n");
        }
    }

    void update(float deltaTime) override {
        if (!active) return;

        pulseTime += deltaTime * pulseSpeed;

        // Update beep speed based on player proximity time
        if (playerInZone) {
            playerInZoneTime += deltaTime;

            // Increase beep speed as player stays longer in zone
            float dangerLevel = playerInZoneTime / timeToDeath;
            currentBeepSpeed = 1.0f + (dangerLevel * 4.0f); // Speed increases from 1x to 5x

            // Update beep timer with current speed
            beepTimer += deltaTime * currentBeepSpeed;

            // Play beep sound
            if (beepTimer >= beepInterval) {
                beepTimer = 0.0f;
                if (globalSoundPlayer) {
                    globalSoundPlayer->playFile("camera_beep.wav", true);
                }
            }
        }
        else {
            playerInZoneTime = 0.0f;
            beepTimer = 0.0f;
            currentBeepSpeed = 1.0f;
        }
    }

    void render() override {
        if (!active) return;

        float pulse = 0.5f + 0.5f * sin(pulseTime);

        // ALWAYS render the detection radius (visible at all times)
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // Always render detection radius (semi-transparent yellow)
        renderDetectionRadius();

        glDisable(GL_COLOR_MATERIAL);

        glColor3f(0.7f, 0.7f, 0.7f);
        // Render the sensor model
        if (enemyModel && enemyModel->hasMaterials()) {
            enemyModel->render();
        }
        else {
            glColor3f(0.3f, 0.3f, 0.3f);
            GLUquadricObj* quad = gluNewQuadric();
            gluCylinder(quad, 0.2f, 0.2f, 0.3f, 16, 1);

            // Light behavior based on player presence
            if (playerInZone) {
                // Blinking red light when player is in zone
                float blinkSpeed = 5.0f + (playerInZoneTime / timeToDeath * 10.0f);
                float lightPulse = 0.5f + 0.5f * sin(pulseTime * blinkSpeed);
                glColor3f(1.0f * lightPulse, 0.0f, 0.0f);
            }
            else {
                // Steady green light when no player
                glColor3f(0.0f, 0.8f, 0.0f);
            }

            glTranslatef(0, 0, 0.3f);
            gluSphere(quad, 0.15f, 16, 16);

            gluDeleteQuadric(quad);
        }
        glEnable(GL_COLOR_MATERIAL);
        glPopMatrix();
    }

    void renderDetectionRadius() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        // Different color based on player presence
        if (playerInZone) {
            // Red pulsing circle when player is in zone
            float dangerPulse = 0.5f + 0.5f * sin(pulseTime * 5.0f);
            glColor4f(1.0f, 0.0f, 0.0f, 0.15f + dangerPulse * 0.1f);
        }
        else {
            // Yellow semi-transparent circle when no player
            glColor4f(1.0f, 1.0f, 0.5f, 0.1f);
        }

        // Draw filled circle (ALWAYS VISIBLE)
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0.11f, 0); // Slightly above ground to avoid z-fighting

        int segments = 36;
        for (int i = 0; i <= segments; i++) {
            float angle = (i * 2.0f * M_PI) / segments;
            float x = sin(angle) * detectionRadius;
            float z = cos(angle) * detectionRadius;
            glVertex3f(x, 0.11f, z);
        }
        glEnd();

        // Draw outline
        glLineWidth(1.0f);
        if (playerInZone) {
            // Red outline when player is in zone
            float outlinePulse = 0.5f + 0.5f * sin(pulseTime * 8.0f);
            glColor4f(1.0f, outlinePulse * 0.3f, outlinePulse * 0.3f, 0.8f);
        }
        else {
            // Yellow outline when no player
            glColor4f(1.0f, 1.0f, 0.3f, 0.6f);
        }

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float angle = (i * 2.0f * M_PI) / segments;
            float x = sin(angle) * detectionRadius;
            float z = cos(angle) * detectionRadius;
            glVertex3f(x, 0.21f, z);
        }
        glEnd();
        glLineWidth(1.0f);

        // Draw warning indicator if player is close to death
        if (playerInZone && (playerInZoneTime / timeToDeath) > 0.7f) {
            renderWarningIndicator();
        }

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
    }

    void renderWarningIndicator() {
        // Pulsing warning circle when death is imminent
        float warningPulse = 0.5f + 0.5f * sin(pulseTime * 10.0f);
        float warningRadius = detectionRadius * 0.8f;

        glLineWidth(3.0f);
        glColor4f(1.0f, 0.0f, 0.0f, warningPulse);

        glBegin(GL_LINE_LOOP);
        int segments = 24;
        for (int i = 0; i < segments; i++) {
            float angle = (i * 2.0f * M_PI) / segments;
            float x = sin(angle) * warningRadius;
            float z = cos(angle) * warningRadius;
            glVertex3f(x, 0.22f, z);
        }
        glEnd();
        glLineWidth(1.0f);
    }

    bool detectPlayer(const Vec3& playerPos) {
        if (!active) return false;

        lastPlayerPos = playerPos;
        float distance = Vec3::distance(position, playerPos);
        bool nowInZone = (distance < detectionRadius);

        // CRITICAL FIX: Reset timer when leaving zone
        if (!nowInZone && playerInZone) {
            playerInZone = false;
            playerInZoneTime = 0.0f;  // Reset timer when leaving zone
            return false;
        }

        // Update player zone status
        playerInZone = nowInZone;

        if (!playerInZone) {
            return false;
        }

        // DEBUG: Show how much time has accumulated
        // printf("Motion sensor: Player in zone for %.2f seconds (need %.2f)\n", 
        //        playerInZoneTime, timeToDeath);

        // Only trigger death when player has been in zone long enough CONTINUOUSLY
        return playerInZoneTime >= timeToDeath;
    }

    // Helper function to get current danger level (for UI display)
    float getDangerLevel() const {
        if (!playerInZone) return 0.0f;
        return playerInZoneTime / timeToDeath;
    }

    // Reset the sensor (call when player dies or respawns)
    void reset() {
        playerInZoneTime = 0.0f;
        playerInZone = false;
        beepTimer = 0.0f;
        currentBeepSpeed = 1.0f;
    }

    // Check if player is in zone (without triggering death)
    bool isPlayerInZone() const {
        return playerInZone;
    }
};


class PressurePlate : public GameObject {
private:
    bool triggered;
    float triggerTime;
    float explosionDelay;
    float radius;
    float height;  // Added height
    GLuint textureID;  // Texture ID for OpenGL

public:
    PressurePlate(const Vec3& pos, GLuint texID = 0) {
        position = pos;
        triggered = false;
        triggerTime = 0.0f;
        explosionDelay = 0.5f;
        radius = 0.4f;
        height = 0.1f;  // Slightly raised from ground
        textureID = texID;
    }

    void update(float deltaTime) override {
        if (triggered) {
            triggerTime += deltaTime;
        }
    }

    void render() override {
        if (!active) return;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // Apply slight depression when triggered
        float currentHeight = triggered ? height * 0.3f : height;
        glTranslatef(0, currentHeight, 0);

        if (textureID != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureID);

            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }

        glEnable(GL_LIGHTING);

        // Material properties
        if (triggered) {
            float flash = (sin(triggerTime * 20.0f) + 1.0f) * 0.5f;
            GLfloat mat_emission[] = { 1.0f, flash * 0.3f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
            glColor3f(1.0f, flash * 0.3f, 0.0f);
        }
        else {
            GLfloat mat_emission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
            glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
            glColor3f(1.0f, 1.0f, 1.0f);
        }

        // Draw top face (plate surface)
        glBegin(GL_QUADS);
        if (textureID != 0) glTexCoord2f(0.0f, 0.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-radius, 0, -radius);

        if (textureID != 0) glTexCoord2f(1.0f, 0.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(radius, 0, -radius);

        if (textureID != 0) glTexCoord2f(1.0f, 1.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(radius, 0, radius);

        if (textureID != 0) glTexCoord2f(0.0f, 1.0f);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-radius, 0, radius);
        glEnd();

        // Draw side faces (edges of the plate)
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.3f, 0.3f, 0.3f);  // Dark gray for sides

        // Front face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-radius, -currentHeight, -radius);
        glVertex3f(radius, -currentHeight, -radius);
        glVertex3f(radius, 0, -radius);
        glVertex3f(-radius, 0, -radius);
        glEnd();

        // Back face
        glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-radius, -currentHeight, radius);
        glVertex3f(radius, -currentHeight, radius);
        glVertex3f(radius, 0, radius);
        glVertex3f(-radius, 0, radius);
        glEnd();

        // Left face
        glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-radius, -currentHeight, -radius);
        glVertex3f(-radius, -currentHeight, radius);
        glVertex3f(-radius, 0, radius);
        glVertex3f(-radius, 0, -radius);
        glEnd();

        // Right face
        glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(radius, -currentHeight, -radius);
        glVertex3f(radius, -currentHeight, radius);
        glVertex3f(radius, 0, radius);
        glVertex3f(radius, 0, -radius);
        glEnd();

        // Reset material emission
        GLfloat mat_emission_reset[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission_reset);

        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    bool checkTrigger(const Vec3& playerPos, float playerRadius) {
        if (triggered && triggerTime >= explosionDelay) {
            return true;
        }

        // Check horizontal distance only (ignore Y)
        Vec3 platePos2D = Vec3(position.x, 0, position.z);
        Vec3 playerPos2D = Vec3(playerPos.x, 0, playerPos.z);
        float horizontalDist = Vec3::distance(platePos2D, playerPos2D);

        // Optional: Check if player is above the plate (not below)
        bool isAbovePlate = playerPos.y > (position.y - 0.1f); // Small tolerance

        // Trigger if player is on the plate horizontally and above it
        if (horizontalDist < (radius + playerRadius) && isAbovePlate && !triggered) {
            triggered = true;
            triggerTime = 0.0f;
        }

        return false;
    }

    void reset() {
        triggered = false;
        triggerTime = 0.0f;
    }

    // Getter for height (useful for positioning)
    float getHeight() const {
        return height;
    }

    // Setter for texture
    void setTexture(GLuint texID) {
        textureID = texID;
    }
};

#endif // SECURITYDEVICES_H