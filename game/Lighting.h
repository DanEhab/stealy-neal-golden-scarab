/*
 * Lighting.h - Enhanced dynamic lighting with animations
 * Implements comprehensive lighting requirements
 */

#ifndef LIGHTING_H
#define LIGHTING_H
#include <windows.h>
#include <glut.h>
#include <stdlib.h>
#include <cmath>
#include "Vec3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

    class LightingManager {
    private:
        // Time of day progression (for Level 1)
        float timeOfDay;  // 0.0 = bright day, 1.0 = evening
        float timeProgression;

        // Rotating/animated lights
        float lightRotation;
        float lightRotationSpeed;

        // Pulsing effects
        float pulseTime;
        float pulseSpeed;

        // Color cycling for special lights
        float colorCycleTime;

        // Level 2 detection state
        bool level2Detected;
        bool redLightsPermanentlyOn;  // Red lights stay on after first detection
        float detectionTime;

    public:
        LightingManager() :
            timeOfDay(0.0f),
            timeProgression(0.08f),  // Speed of day-to-evening transition
            lightRotation(0.0f),
            lightRotationSpeed(45.0f),
            pulseTime(0.0f),
            pulseSpeed(2.0f),
            colorCycleTime(0.0f),
            level2Detected(false),
            detectionTime(0.0f)
        {}

        /*
         * LEVEL 1: Museum Lighting (Daytime to Evening)
         * - Bright white/yellow ceiling lights that gradually dim
         * - Color intensity changes from bright white to orange/warm tones
         * - Rotating spotlight animation on display cases
         */
        void setupMuseumLights() {
            // Progressive dimming factor (1.0 = day, 0.4 = evening)
            float dimFactor = 1.0f - (timeOfDay * 0.6f);

            // Color temperature shift
            float warmth = timeOfDay * 0.3f;

            // REDUCE GLOBAL AMBIENT SIGNIFICANTLY
            GLfloat ambient[] = {
                0.1f * dimFactor,  // Much lower ambient
                0.1f * dimFactor,
                0.1f * dimFactor,
                1.0f
            };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

            // Enable two-sided lighting for better model viewing
            glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

            // LIGHT 0: Main ceiling light - REDUCE AMBIENT COMPONENT
            GLfloat l0Diffuse[] = {
                (0.9f + warmth * 0.1f) * dimFactor,
                (0.9f - warmth * 0.1f) * dimFactor,
                (0.8f - warmth * 0.3f) * dimFactor,
                1.0f
            };
            GLfloat l0Ambient[] = {  // REDUCED
                0.1f * dimFactor,
                0.1f * dimFactor,
                0.1f * dimFactor,
                1.0f
            };
            GLfloat l0Position[] = { 0.0f, 8.0f, 0.0f, 1.0f };
            glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
            glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
            glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
            glEnable(GL_LIGHT0);

            // LIGHT 1: Secondary - REDUCE AMBIENT
            GLfloat l1Diffuse[] = {
                1.0f * dimFactor,
                (0.85f - warmth * 0.2f) * dimFactor,
                (0.6f - warmth * 0.4f) * dimFactor,
                1.0f
            };
            GLfloat l1Ambient[] = {  // REDUCED
                0.08f * dimFactor,
                0.08f * dimFactor,
                0.05f * dimFactor,
                1.0f
            };
            GLfloat l1Position[] = { 15.0f, 8.0f, 15.0f, 1.0f };
            glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
            glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
            glLightfv(GL_LIGHT1, GL_POSITION, l1Position);
            glEnable(GL_LIGHT1);

            // Keep other lights as they are but reduce their ambient if they have it
            // LIGHT 2: Spot light - REDUCE AMBIENT
            float rotRad = lightRotation * M_PI / 180.0f;
            GLfloat l2Diffuse[] = { 1.0f, 1.0f, 0.95f, 1.0f };
            GLfloat l2Ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };  // REDUCED
            GLfloat l2Position[] = {
                5.0f + 3.0f * cos(rotRad),
                7.0f,
                5.0f + 3.0f * sin(rotRad),
                1.0f
            };
            GLfloat l2Direction[] = { 0.0f, -1.0f, 0.0f };
            glLightfv(GL_LIGHT2, GL_DIFFUSE, l2Diffuse);
            glLightfv(GL_LIGHT2, GL_AMBIENT, l2Ambient);
            glLightfv(GL_LIGHT2, GL_POSITION, l2Position);
            glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, l2Direction);
            glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 30.0f);
            glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 25.0f);
            glEnable(GL_LIGHT2);

            // LIGHT 3: Pulsing light - no ambient needed
            float pulse = 0.5f + 0.5f * sin(pulseTime);
            GLfloat l3Diffuse[] = {
                0.8f * pulse * dimFactor,
                0.7f * pulse * dimFactor,
                0.5f * pulse * dimFactor,
                1.0f
            };
            GLfloat l3Position[] = { -15.0f, 7.0f, -10.0f, 1.0f };
            glLightfv(GL_LIGHT3, GL_DIFFUSE, l3Diffuse);
            glLightfv(GL_LIGHT3, GL_POSITION, l3Position);
            glEnable(GL_LIGHT3);
        }

        /*
         * LEVEL 2: Vault Lighting (Dark with Flashlight)
         * - Very dark blue ambient lighting ONLY at start
         * - Red emergency lights appear ONLY when player gets detected
         * - Player's flashlight (spotlight that follows player)
         * - REFACTORED: Now accepts dynamic crownPosition instead of hardcoded coordinates
         */
        void setupVaultLights(const Vec3& playerPos, const Vec3& playerForward, bool flashlightOn, bool playerDetected, const Vec3& crownPosition) {
            // MUCH BRIGHTER BLUE AMBIENT (more noticeable)
            GLfloat ambient[] = { 0.15f, 0.15f, 0.4f, 1.0f };
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

            // Set detection state - once detected, red lights stay on permanently
            if (playerDetected && !redLightsPermanentlyOn) {
                redLightsPermanentlyOn = true;
                detectionTime = 0.0f;
            }

            // LIGHT 0: Player's FLASHLIGHT (follows player direction)
            // Only active if flashlight is on
            if (flashlightOn) {
                // Position the light source at the flashlight position
                Vec3 flashlightPos = playerPos + playerForward * 0.3f;
                flashlightPos.y += 1.2f;

                // VERY BRIGHT FLASHLIGHT
                GLfloat l0Diffuse[] = { 3.5f, 3.5f, 3.0f, 1.0f };  // Even brighter
                GLfloat l0Specular[] = { 4.0f, 4.0f, 3.5f, 1.0f }; // Stronger specular highlights
                GLfloat l0Ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };  // More ambient boost

                GLfloat l0Position[] = { flashlightPos.x, flashlightPos.y, flashlightPos.z, 1.0f };
                GLfloat l0Direction[] = { playerForward.x, playerForward.y, playerForward.z };

                glLightfv(GL_LIGHT0, GL_DIFFUSE, l0Diffuse);
                glLightfv(GL_LIGHT0, GL_SPECULAR, l0Specular);
                glLightfv(GL_LIGHT0, GL_AMBIENT, l0Ambient);
                glLightfv(GL_LIGHT0, GL_POSITION, l0Position);
                glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, l0Direction);

                // WIDE BEAM for better visibility
                glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 60.0f);      // Even wider cone
                glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 15.0f);    // Softer edges

                // MINIMAL ATTENUATION - light travels very far
                glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 1.0f);
                glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.002f);    // Even smaller
                glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0002f); // Even smaller

                glEnable(GL_LIGHT0);
            }
            else {
                // Still enable a dim version of LIGHT0 to provide some ambient light
                GLfloat l0DimDiffuse[] = { 0.1f, 0.1f, 0.15f, 1.0f };
                GLfloat l0DimAmbient[] = { 0.05f, 0.05f, 0.1f, 1.0f };
                GLfloat l0DimPosition[] = { playerPos.x, playerPos.y + 1.5f, playerPos.z, 1.0f };

                glLightfv(GL_LIGHT0, GL_DIFFUSE, l0DimDiffuse);
                glLightfv(GL_LIGHT0, GL_AMBIENT, l0DimAmbient);
                glLightfv(GL_LIGHT0, GL_POSITION, l0DimPosition);
                glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 180.0f);  // Omnidirectional
                glEnable(GL_LIGHT0);
            }

            // EMERGENCY RED LIGHTS - ONCE DETECTED, STAY ON PERMANENTLY
            if (redLightsPermanentlyOn) {
                // Update detection time for pulsing effect
                detectionTime += 0.016f; // Assuming 60 FPS, adjust in update()

                // LIGHT 1: PULSING RED EMERGENCY LIGHT (permanently on)
                float redPulse = 0.6f + 0.4f * sin(pulseTime * 2.0f); // Slower, more noticeable pulsing
                GLfloat l1Diffuse[] = { 1.2f * redPulse, 0.0f, 0.0f, 1.0f };  // Brighter red
                GLfloat l1Ambient[] = { 0.4f * redPulse, 0.0f, 0.0f, 1.0f };  // Stronger ambient
                GLfloat l1Position[] = { -18.0f, 7.0f, -15.0f, 1.0f };
                glLightfv(GL_LIGHT1, GL_DIFFUSE, l1Diffuse);
                glLightfv(GL_LIGHT1, GL_AMBIENT, l1Ambient);
                glLightfv(GL_LIGHT1, GL_POSITION, l1Position);

                // Add attenuation so red light fades with distance
                glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 1.0f);
                glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION, 0.05f);
                glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.01f);
                glEnable(GL_LIGHT1);

                // LIGHT 3: Secondary pulsing emergency light (different phase)
                float redPulse2 = 0.6f + 0.4f * sin(pulseTime * 2.0f + M_PI * 0.7f); // Different phase
                GLfloat l3Diffuse[] = { 1.2f * redPulse2, 0.0f, 0.0f, 1.0f };
                GLfloat l3Ambient[] = { 0.4f * redPulse2, 0.0f, 0.0f, 1.0f };
                GLfloat l3Position[] = { 18.0f, 7.0f, 10.0f, 1.0f };
                glLightfv(GL_LIGHT3, GL_DIFFUSE, l3Diffuse);
                glLightfv(GL_LIGHT3, GL_AMBIENT, l3Ambient);
                glLightfv(GL_LIGHT3, GL_POSITION, l3Position);

                // Add attenuation
                glLightf(GL_LIGHT3, GL_CONSTANT_ATTENUATION, 1.0f);
                glLightf(GL_LIGHT3, GL_LINEAR_ATTENUATION, 0.05f);
                glLightf(GL_LIGHT3, GL_QUADRATIC_ATTENUATION, 0.01f);
                glEnable(GL_LIGHT3);
            }
            else {
                // Disable emergency lights when not detected yet
                glDisable(GL_LIGHT1);
                glDisable(GL_LIGHT3);
            }

            // LIGHT 2: ROTATING + COLOR-CYCLING spotlight on Crown Jewels (REFACTORED WITH DYNAMIC POSITION)
            float rotRad = lightRotation * M_PI / 180.0f;
            float colorPhase = colorCycleTime * 2.0f * M_PI / 3.0f; // 3-second color cycle

            // Smooth RGB color cycling (Red -> Yellow -> Green -> Cyan -> Blue -> Magenta -> Red)
            GLfloat l2Diffuse[] = {
                0.6f + 0.4f * sin(colorPhase),              // Red component
                0.6f + 0.4f * sin(colorPhase + 2.094f),     // Green component (120° phase shift)
                0.6f + 0.4f * sin(colorPhase + 4.189f),     // Blue component (240° phase shift)
                1.0f
            };
            
            // Position spotlight to rotate around the DYNAMIC Crown position (passed as parameter)
            GLfloat l2Position[] = {
                crownPosition.x + 3.0f * cos(rotRad),  // Rotates in a 3-meter radius circle
                crownPosition.y + 5.5f,                 // 5.5 meters above the crown
                crownPosition.z + 3.0f * sin(rotRad),  // Centered on Crown z-position
                1.0f
            };
            
            // Calculate direction vector to point spotlight AT the crown
            Vec3 lightPos(l2Position[0], l2Position[1], l2Position[2]);
            Vec3 spotDirection = (crownPosition - lightPos).normalized();
            
            GLfloat l2Direction[] = { spotDirection.x, spotDirection.y, spotDirection.z };
            
            glLightfv(GL_LIGHT2, GL_DIFFUSE, l2Diffuse);
            glLightfv(GL_LIGHT2, GL_POSITION, l2Position);
            glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, l2Direction);
            glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 25.0f);      // Focused beam (theatrical effect)
            glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 30.0f);    // Sharp edges for dramatic spotlight

            // Minimal attenuation for strong illumination
            glLightf(GL_LIGHT2, GL_CONSTANT_ATTENUATION, 1.0f);
            glLightf(GL_LIGHT2, GL_LINEAR_ATTENUATION, 0.01f);
            glLightf(GL_LIGHT2, GL_QUADRATIC_ATTENUATION, 0.001f);

            glEnable(GL_LIGHT2);



            // DEBUG: Render a tiny wire sphere at light source position to visualize it
            glDisable(GL_LIGHTING);
            glColor3f(1.0f, 1.0f, 0.0f);  // Yellow for visibility
            glPushMatrix();
            glTranslatef(l2Position[0], l2Position[1], l2Position[2]);
            glutWireSphere(0.5, 10, 10);  // Small yellow sphere
            glPopMatrix();
            glEnable(GL_LIGHTING);
        }

        void renderFlashlightCone(const Vec3& position, const Vec3& direction) {
            glDisable(GL_LIGHTING);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            glPushMatrix();
            glTranslatef(position.x, position.y, position.z);

            // Calculate rotation to align with direction
            Vec3 forward = direction.normalized();
            Vec3 up(0, 1, 0);
            if (fabs(forward.y) > 0.9f) {
                up = Vec3(0, 0, 1);
            }
            Vec3 right = forward.cross(up).normalized();
            up = right.cross(forward).normalized();

            // Create rotation matrix
            float rotation[16] = {
                right.x, right.y, right.z, 0,
                up.x, up.y, up.z, 0,
                -forward.x, -forward.y, -forward.z, 0,
                0, 0, 0, 1
            };
            glMultMatrixf(rotation);

            // Draw translucent cone
            glColor4f(1.0f, 1.0f, 0.8f, 0.15f);

            float coneLength = 20.0f;  // Longer reach
            float coneAngle = 30.0f;   // Wider cone
            float baseRadius = coneLength * tan(coneAngle * M_PI / 180.0f);

            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, 0, 0);  // Flashlight position

            int segments = 24;
            for (int i = 0; i <= segments; i++) {
                float angle = (2.0f * M_PI * i) / segments;
                float x = sin(angle) * baseRadius;
                float y = cos(angle) * baseRadius;
                glVertex3f(x, y, -coneLength);
            }
            glEnd();

            // Draw cone outline
            glColor4f(1.0f, 1.0f, 0.5f, 0.4f);
            glLineWidth(2.0f);
            glBegin(GL_LINES);
            glVertex3f(0, 0, 0);
            for (int i = 0; i <= segments; i++) {
                float angle = (2.0f * M_PI * i) / segments;
                float x = sin(angle) * baseRadius;
                float y = cos(angle) * baseRadius;
                glVertex3f(0, 0, 0);
                glVertex3f(x, y, -coneLength);
            }
            glEnd();
            glLineWidth(1.0f);

            glPopMatrix();

            glDisable(GL_BLEND);
            glEnable(GL_LIGHTING);
        }

        /*
         * Update all light animations
         */
        void update(float deltaTime) {
            // Rotate lights (translation animation)
            lightRotation += lightRotationSpeed * deltaTime;
            if (lightRotation >= 360.0f) {
                lightRotation -= 360.0f;
            }

            // Pulse animation (intensity changes)
            pulseTime += deltaTime * pulseSpeed;
            if (pulseTime >= 2.0f * M_PI) {
                pulseTime -= 2.0f * M_PI;
            }

            // Color cycling animation
            colorCycleTime += deltaTime * 0.5f;
            if (colorCycleTime >= 3.0f) {
                colorCycleTime -= 3.0f;
            }

            // Update detection time for emergency lights
            if (redLightsPermanentlyOn) {
                detectionTime += deltaTime;
            }
            else if (level2Detected) {
                detectionTime += deltaTime;
            }

            // Time of day progression (Level 1: day -> evening)
            // This creates gradual color intensity change
            timeOfDay += timeProgression * deltaTime;
            if (timeOfDay > 1.0f) {
                timeOfDay = 1.0f;
            }
        }

        void resetTimeOfDay() {
            timeOfDay = 0.0f;
        }

        // Set detection state for level 2
        void setLevel2Detected(bool detected) {
            level2Detected = detected;
            if (detected) {
                redLightsPermanentlyOn = true;  // Once detected, red lights stay on
                detectionTime = 0.0f;
            }
        }

        // Reset all states for new level
        void resetForNewLevel() {
            level2Detected = false;
            redLightsPermanentlyOn = false;
            detectionTime = 0.0f;
        }

        bool isLevel2Detected() const { return level2Detected; }
        bool areRedLightsOn() const { return redLightsPermanentlyOn; }

        void disableAll() {
            glDisable(GL_LIGHT0);
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            glDisable(GL_LIGHT3);
        }

        float getTimeOfDay() const { return timeOfDay; }
        float getLightRotation() const { return lightRotation; }
};

#endif // LIGHTING_H