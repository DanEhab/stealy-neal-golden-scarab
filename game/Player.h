/*
 * Player.h - Player movement and state management
 */

#ifndef PLAYER_H
#define PLAYER_H
#include "ModelLoader.h"
#include "Vec3.h"
#include "Camera.h"
#include <set>
#include "ModelResourceManager.h"
class Player {
private:
    Vec3 position;
    Camera* camera;
    Model* playerModel;
    Model* flashlightModel;  // NEW: 3D flashlight model
    
    // Movement state
    bool movingForward;
    bool movingBackward;
    bool movingLeft;
    bool movingRight;
    bool jumping;

    // Player physics
    float speed;
    float jumpVelocity;
    float verticalVelocity;
    float gravity;
    bool onGround;

    // Player dimensions (for collision)
    float radius;
    float height;
    float width;  // Added for box collision
    float depth;  // Added for box collision

    // Score
    int score;

    // Debug visualization
    bool showHitbox;

public:
    Player(Camera* cam) :
        position(0, 0, 0),
        camera(cam),
        movingForward(false),
        movingBackward(false),
        movingLeft(false),
        movingRight(false),
        jumping(false),
        speed(5.0f),
        jumpVelocity(10.0f),
        verticalVelocity(0.0f),
        gravity(-15.0f),
        onGround(true),
        radius(0.4f),
        height(1.9f), // Standard human height
        width(0.8f),  // Default width
        depth(0.4f),  // Default depth (thickness)
        score(0),
        showHitbox(false)  // Debug off by default
    {
        // Initialize player model
        playerModel = ModelResourceManager::getInstance().getModel("models/elite.obj");

        // Load the player model
        if (playerModel) {
            printf("Player model loaded successfully\n");

            // OPTION 1: Auto-scale to target height (recommended)
            playerModel->autoScaleToHeight(height); // Scale to 1.8m tall

            // Update collision dimensions based on scaled model
            Vec3 minBounds, maxBounds;
            playerModel->getScaledBounds(minBounds, maxBounds);

            // Calculate dimensions from model bounds
            width = maxBounds.x - minBounds.x;
            depth = maxBounds.z - minBounds.z;
            height = maxBounds.y - minBounds.y;

            // Use the larger of width/depth for radius (for backward compatibility)
            radius = (((width) > (depth)) ? (width) : (depth)) * 0.5f;

            printf("Player collision dimensions:\n");
            printf("  Width: %.2f, Depth: %.2f, Height: %.2f\n", width, depth, height);
            printf("  Radius (circular approximation): %.2f\n", radius);
        }
        else {
            printf("ERROR: Failed to load player model\n");
        }
        
        // NEW: Load flashlight model
        flashlightModel = ModelResourceManager::getInstance().getModel("models/Flashlight.obj");
        if (flashlightModel) {
            // Scale flashlight to much smaller size (was 0.4f, now 0.15f)
            flashlightModel->autoScaleToHeight(0.45f);  // 15cm tall flashlight
            printf("Flashlight model loaded successfully\n");
        }
        else {
            printf("ERROR: Failed to load flashlight model\n");
        }
    }

    ~Player() {
        if (playerModel) {
        }
    }

    /*
     * Update player position based on input
     */
    void update(float deltaTime) {
        Vec3 movement(0, 0, 0);

        // Calculate movement direction
        if (movingForward) {
            Vec3 forward = camera->getForward();
            forward.y = 0;  // Don't move up/down when walking
            forward.normalize();
            movement += forward;
        }
        if (movingBackward) {
            Vec3 forward = camera->getForward();
            forward.y = 0;
            forward.normalize();
            movement -= forward;
        }
        if (movingLeft) {
            movement -= camera->getRight();
        }
        if (movingRight) {
            movement += camera->getRight();
        }

        // Normalize movement vector if moving diagonally
        if (movement.lengthSquared() > 0) {
            movement.normalize();
            position += movement * speed * deltaTime;
        }

        // Handle jumping and gravity
        if (jumping && onGround) {
            verticalVelocity = jumpVelocity;
            onGround = false;
            jumping = false;
        }

        verticalVelocity += gravity * deltaTime;
        position.y += verticalVelocity * deltaTime;

        // Simple ground collision (floor at y=0)
        if (position.y <= 0) {
            position.y = 0;
            verticalVelocity = 0;
            onGround = true;
        }

        // Update camera position
        Vec3 cameraPos = position;
        cameraPos.y += height;  // Eye height
        camera->setPosition(cameraPos);
    }

    /*
     * Check collision with a box (AABB) using player's box hitbox
     */
    bool checkBoxCollision(const Vec3& boxMin, const Vec3& boxMax) {
        Vec3 playerMin = getHitboxMin();
        Vec3 playerMax = getHitboxMax();

        return (playerMin.x <= boxMax.x && playerMax.x >= boxMin.x) &&
            (playerMin.y <= boxMax.y && playerMax.y >= boxMin.y) &&
            (playerMin.z <= boxMax.z && playerMax.z >= boxMin.z);
    }

    /*
     * Resolve collision by pushing player back (improved for box collision)
     */
    void resolveCollision(const Vec3& boxMin, const Vec3& boxMax) {
        Vec3 playerMin = getHitboxMin();
        Vec3 playerMax = getHitboxMax();

        // Calculate overlap on each axis
        float overlapX = 0, overlapY = 0, overlapZ = 0;

        // X-axis overlap
        if (playerMax.x > boxMin.x && playerMin.x < boxMax.x) {
            float leftOverlap = playerMax.x - boxMin.x;
            float rightOverlap = boxMax.x - playerMin.x;
            overlapX = (leftOverlap < rightOverlap) ? -leftOverlap : rightOverlap;
        }

        // Z-axis overlap
        if (playerMax.z > boxMin.z && playerMin.z < boxMax.z) {
            float frontOverlap = playerMax.z - boxMin.z;
            float backOverlap = boxMax.z - playerMin.z;
            overlapZ = (frontOverlap < backOverlap) ? -frontOverlap : backOverlap;
        }

        // Y-axis overlap (for jumping/falling)
        if (playerMax.y > boxMin.y && playerMin.y < boxMax.y) {
            float topOverlap = playerMax.y - boxMin.y;
            float bottomOverlap = boxMax.y - playerMin.y;
            overlapY = (topOverlap < bottomOverlap) ? -topOverlap : bottomOverlap;
        }

        // Resolve collision based on smallest overlap
        if (abs(overlapX) < abs(overlapZ)) {
            // Push along X axis
            position.x += overlapX;
        }
        else {
            // Push along Z axis
            position.z += overlapZ;
        }

        // Handle vertical collisions (landing on top of boxes)
        if (abs(overlapY) < abs(overlapX) && abs(overlapY) < abs(overlapZ)) {
            if (overlapY < 0) {
                // Landing on top of something
                position.y += overlapY;
                if (verticalVelocity < 0) {
                    verticalVelocity = 0;
                    onGround = true;
                }
            }
            else {
                // Hitting head on something
                position.y += overlapY;
                if (verticalVelocity > 0) {
                    verticalVelocity = 0;
                }
            }
        }
    }

    /*
     * Get the minimum point of the player's hitbox
     */
    Vec3 getHitboxMin() const {
        return Vec3(position.x - width / 2, position.y, position.z - depth / 2);
    }

    /*
     * Get the maximum point of the player's hitbox
     */
    Vec3 getHitboxMax() const {
        return Vec3(position.x + width / 2, position.y + height, position.z + depth / 2);
    }

    /*
     * Render player model (for third person view)
     */
    void render() {
        if (camera->getMode() == THIRD_PERSON) {
            glDisable(GL_COLOR_MATERIAL);
            glPushMatrix();
            glTranslatef(position.x, position.y, position.z);

            // Rotate model to face movement direction
            if (movingForward || movingBackward || movingLeft || movingRight) {
                Vec3 forward = camera->getForward();
                float yaw = atan2f(forward.x, forward.z) * 180.0f / M_PI;
                glRotatef(yaw, 0, 1, 0);
            }

            // IMPORTANT: Don't use glScalef here anymore!
            // The scaling is already handled inside the Model class
            // This preserves colors and materials

            // Render the OBJ model - it will use its internal scale
            if (playerModel) {
                playerModel->render();
            }

            glPopMatrix();
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        }

        // Render debug hitbox if enabled
        if (showHitbox) {
            renderDebugHitbox();
        }
    }

    /*
     * Render debug visualization of player hitbox
     */
    void renderDebugHitbox() {
        glPushMatrix();

        // Disable lighting for clear visualization
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);

        // Draw wireframe box
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);

        // Green for hitbox
        glColor3f(0.0f, 1.0f, 0.0f);

        Vec3 min = getHitboxMin();
        Vec3 max = getHitboxMax();

        // Draw the box
        glBegin(GL_QUADS);

        // Front face
        glVertex3f(min.x, min.y, max.z);
        glVertex3f(max.x, min.y, max.z);
        glVertex3f(max.x, max.y, max.z);
        glVertex3f(min.x, max.y, max.z);

        // Back face
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(min.x, max.y, min.z);
        glVertex3f(max.x, max.y, min.z);
        glVertex3f(max.x, min.y, min.z);

        // Left face
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(min.x, min.y, max.z);
        glVertex3f(min.x, max.y, max.z);
        glVertex3f(min.x, max.y, min.z);

        // Right face
        glVertex3f(max.x, min.y, min.z);
        glVertex3f(max.x, max.y, min.z);
        glVertex3f(max.x, max.y, max.z);
        glVertex3f(max.x, min.y, max.z);

        // Top face
        glVertex3f(min.x, max.y, min.z);
        glVertex3f(min.x, max.y, max.z);
        glVertex3f(max.x, max.y, max.z);
        glVertex3f(max.x, max.y, min.z);

        // Bottom face
        glVertex3f(min.x, min.y, min.z);
        glVertex3f(max.x, min.y, min.z);
        glVertex3f(max.x, min.y, max.z);
        glVertex3f(min.x, min.y, max.z);

        glEnd();

        // Draw center point
        glPointSize(5.0f);
        glBegin(GL_POINTS);
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for center
        glVertex3f(position.x, position.y + height / 2, position.z);
        glEnd();

        // Restore settings
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);

        glPopMatrix();

        // Draw hitbox dimensions text (would need GLUT bitmap text functions)
        // For now, just print to console
        static int frameCount = 0;
        if (frameCount++ % 60 == 0) {  // Print every second at 60fps
            printf("Hitbox: (%.2f, %.2f, %.2f) to (%.2f, %.2f, %.2f)\n",
                min.x, min.y, min.z, max.x, max.y, max.z);
        }
    }

    /*
     * Scale methods to adjust model size at runtime
     */
    void scaleModel(float scaleFactor) {
        if (playerModel) {
            Vec3 currentScale = playerModel->getScale();
            playerModel->setScale(currentScale.x * scaleFactor,
                currentScale.y * scaleFactor,
                currentScale.z * scaleFactor);

            // Update collision dimensions
            Vec3 minBounds, maxBounds;
            playerModel->getScaledBounds(minBounds, maxBounds);
            width = maxBounds.x - minBounds.x;
            depth = maxBounds.z - minBounds.z;
            height = maxBounds.y - minBounds.y;
            radius = (((width) > (depth)) ? (width) : (depth)) * 0.5f;

            printf("Player scaled by %.2f\n", scaleFactor);
            printf("New dimensions: W=%.2f, D=%.2f, H=%.2f, R=%.2f\n",
                width, depth, height, radius);
        }
    }

    void setModelScale(float uniformScale) {
        if (playerModel) {
            playerModel->setScale(uniformScale);

            // Update collision dimensions
            Vec3 minBounds, maxBounds;
            playerModel->getScaledBounds(minBounds, maxBounds);
            width = maxBounds.x - minBounds.x;
            depth = maxBounds.z - minBounds.z;
            height = maxBounds.y - minBounds.y;
            radius = (((width) > (depth)) ? (width) : (depth)) * 0.5f;

            printf("Player scale set to %.2f\n", uniformScale);
            printf("New dimensions: W=%.2f, D=%.2f, H=%.2f, R=%.2f\n",
                width, depth, height, radius);
        }
    }

    void setModelScale(float x, float y, float z) {
        if (playerModel) {
            playerModel->setScale(x, y, z);

            // Update collision dimensions
            Vec3 minBounds, maxBounds;
            playerModel->getScaledBounds(minBounds, maxBounds);
            width = maxBounds.x - minBounds.x;
            depth = maxBounds.z - minBounds.z;
            height = maxBounds.y - minBounds.y;
            radius = (((width) > (depth)) ? (width) : (depth)) * 0.5f;

            printf("Player scale set to (%.2f,%.2f,%.2f)\n", x, y, z);
            printf("New dimensions: W=%.2f, D=%.2f, H=%.2f, R=%.2f\n",
                width, depth, height, radius);
        }
    }

    float getModelScale() const {
        if (playerModel) {
            Vec3 scale = playerModel->getScale();
            return (scale.x + scale.y + scale.z) / 3.0f; // Average scale
        }
        return 1.0f;
    }

    /*
     * Render flashlight in player's hand (first person view) - NOW WITH 3D MODEL - FIXED VISIBILITY
     */
    void renderFlashlight(bool flashlightOn) {
        // Only render flashlight in first person view
        if (camera->getMode() != FIRST_PERSON) return;

        glPushMatrix();

        // Get camera vectors
        Vec3 camPos = camera->getPosition();
        Vec3 forward = camera->getForward();
        Vec3 right = camera->getRight();
        Vec3 up = camera->getUp();

        // Position adjustments
        float horizontalOffset = 0.5f;    // Right side
        float verticalOffset = -0.3f;     // Lower position
        float depthOffset = 1.0f;         // Distance in front

        // Calculate position
        Vec3 flashlightPos = camPos +
            forward * depthOffset +
            right * horizontalOffset +
            up * verticalOffset;

        // Don't let flashlight go below ground
        float groundLevel = 0.0f;
        if (flashlightPos.y < groundLevel + 0.1f) {
            flashlightPos.y = groundLevel + 0.1f;
        }

        glTranslatef(flashlightPos.x, flashlightPos.y, flashlightPos.z);

        // Scale it down
        glScalef(0.3f, 0.3f, 0.3f);

        // Rotate to face camera direction
        float yaw = atan2f(forward.x, forward.z) * 180.0f / M_PI;
        float pitch = -asinf(forward.y) * 180.0f / M_PI;

        glRotatef(yaw, 0, 1, 0);
        glRotatef(pitch, 1, 0, 0);

        // Fix: Rotate flashlight barrel to point forward
        // (Many flashlight models point along the Z-axis, not X-axis)
        //glRotatef(90, 0, 1, 0);  // Rotate 90 degrees around Y to point forward

        // Enable lighting and materials
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        // Disable textures so we can use solid color
        glDisable(GL_TEXTURE_2D);

        // HARDCODED DARK GREY COLOR
        float darkGreyR = 0.2f;   // 20% red
        float darkGreyG = 0.2f;   // 20% green
        float darkGreyB = 0.25f;  // 25% blue (slightly blue tint)

        if (flashlightModel) {
            // Apply dark grey color
            glColor3f(darkGreyR, darkGreyG, darkGreyB);

            // Set material properties for dark grey metal
            GLfloat darkAmbient[] = { darkGreyR * 0.5f, darkGreyG * 0.5f, darkGreyB * 0.5f, 1.0f };
            GLfloat darkDiffuse[] = { darkGreyR, darkGreyG, darkGreyB, 1.0f };
            GLfloat darkSpecular[] = { 0.3f, 0.3f, 0.3f, 1.0f }; // Low specular for dark metal

            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, darkAmbient);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, darkDiffuse);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, darkSpecular);
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 10.0f); // Low shininess

            // Apply emission for the lens when flashlight is on
            if (flashlightOn) {
                // Bright yellow/orange emission for the lens
                //GLfloat emissiveMaterial[] = { 0.8f, 0.7f, 0.2f, 1.0f };
                //glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emissiveMaterial);
            }

            // Render the model
            flashlightModel->render();

            // Reset emission
            if (flashlightOn) {
                GLfloat noEmission[] = { 0.0f, 0.0f, 0.0f, 1.0f };
                glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, noEmission);
            }
        }
        else {
            // Fallback primitive rendering
            GLUquadricObj* quad = gluNewQuadric();

            if (flashlightOn) {
                glColor3f(0.8f, 0.8f, 0.9f);
                gluCylinder(quad, 0.05f, 0.05f, 0.3f, 8, 1);

                glPushMatrix();
                glTranslatef(0, 0, 0.3f);
                gluCylinder(quad, 0.08f, 0.08f, 0.1f, 8, 1);

                glDisable(GL_LIGHTING);
                glColor3f(1.0f, 1.0f, 0.7f);
                glTranslatef(0, 0, 0.1f);
                gluDisk(quad, 0, 0.08f, 8, 1);
                glEnable(GL_LIGHTING);
                glPopMatrix();
            }
            else {
                glColor3f(0.4f, 0.4f, 0.5f);
                gluCylinder(quad, 0.05f, 0.05f, 0.3f, 8, 1);

                glPushMatrix();
                glTranslatef(0, 0, 0.3f);
                gluCylinder(quad, 0.08f, 0.08f, 0.1f, 8, 1);

                glColor3f(0.2f, 0.2f, 0.3f);
                glTranslatef(0, 0, 0.1f);
                gluDisk(quad, 0, 0.08f, 8, 1);
                glPopMatrix();
            }

            gluDeleteQuadric(quad);
        }
        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    // Input handlers
    void setMovingForward(bool m) { movingForward = m; }
    void setMovingBackward(bool m) { movingBackward = m; }
    void setMovingLeft(bool m) { movingLeft = m; }
    void setMovingRight(bool m) { movingRight = m; }
    void jump() { jumping = true; }

    // Score management
    void addScore(int points) { score += points; }
    void resetScore() { score = 0; }
    int getScore() const { return score; }

    // Getters
    Vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }
    float getHeight() const { return height; }
    float getWidth() const { return width; }
    float getDepth() const { return depth; }
    bool isHitboxVisible() const { return showHitbox; }

    // Setters
    void setPosition(const Vec3& pos) { position = pos; }
    void setHitboxVisible(bool visible) { showHitbox = visible; }
    void toggleHitbox() { showHitbox = !showHitbox; }

    // Manually set hitbox dimensions (for fine-tuning)
    void setHitboxDimensions(float w, float d, float h) {
        width = w;
        depth = d;
        height = h;
        printf("Hitbox set to: W=%.2f, D=%.2f, H=%.2f\n", width, depth, height);
    }

    void reset(const Vec3& startPos) {
        position = startPos;
        verticalVelocity = 0;
        onGround = true;
        movingForward = movingBackward = movingLeft = movingRight = false;
    }
    
    // NEW: Apply knockback effect when hit by guard
    void applyKnockback(const Vec3& guardPosition, float knockbackForce = 2.0f) {
        // Calculate direction away from guard
        Vec3 knockbackDir = position - guardPosition;
        knockbackDir.y = 0; // Only horizontal knockback
        
        if (knockbackDir.lengthSquared() > 0.001f) {
            knockbackDir.normalize();
            
            // Apply knockback to position
            position += knockbackDir * knockbackForce;
            
            // Add slight upward velocity for stumble effect
            verticalVelocity = 2.0f;
            onGround = false;
        }
        
        printf("Player knocked back!\n");
    }
    
    // NEW: Deduct score with negative feedback
    void deductScore(int points) {
        score -= points;
        if (score < 0) score = 0; // Don't go negative
        printf("Score deducted! -%d points. Current score: %d\n", points, score);
    }
};

#endif // PLAYER_H