/*
 * GameObject.h - Base classes for game objects with enhanced animations
 * UPDATED: Added collection animations, door swing, guard animations
 */

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <windows.h>
#include <glut.h>
#include <stdlib.h>
#include "Vec3.h"
#include <vector>
#include "ModelLoader.h"
#include "SoundPlayer.h"
#include "ModelResourceManager.h"

// Forward declare SoundPlayer for use in GameObjects
extern SoundPlayer* globalSoundPlayer;

// Forward declare Wall class for use in GameObject
class Wall;
class TrashBin;  // Forward declare TrashBin

// Base game object class
class GameObject {
protected:
    Vec3 position;
    Vec3 scale;
    Vec3 rotation;
    bool active;

public:
    GameObject() :
        position(0, 0, 0),
        scale(1, 1, 1),
        rotation(0, 0, 0),
        active(true)
    {}

    virtual ~GameObject() {}

    virtual void update(float deltaTime) {}
    
    // NEW: Overload for objects that need wall collision (like PatrolEnemy)
    virtual void update(float deltaTime, const std::vector<Wall*>& walls) {
        update(deltaTime); // Call non-wall version by default
    }
    
    // NEW: Overload for objects that need both wall AND trash bin collision
    virtual void update(float deltaTime, const std::vector<Wall*>& walls, const std::vector<TrashBin*>& trashBins) {
        update(deltaTime, walls); // Call wall version by default
    }
    
    virtual void render() = 0;

    Vec3 getPosition() const { return position; }
    void setPosition(const Vec3& pos) { position = pos; }
    bool isActive() const { return active; }
    void setActive(bool a) { active = a; }
};

// Wall/Obstacle (unchanged)
class Wall {
private:
    Vec3 position;
    Vec3 size;
    float r, g, b;
    bool useTexture;
    GLuint textureID;
    float textureRepeatX;
    float textureRepeatY;

public:
    Wall(const Vec3& pos, const Vec3& siz, float red, float green, float blue)
        : position(pos), size(siz), r(red), g(green), b(blue),
        useTexture(false), textureID(0), textureRepeatX(1.0f), textureRepeatY(1.0f) {}

    Wall(const Vec3& pos, const Vec3& siz, GLuint texID, float repeatX = 1.0f, float repeatY = 1.0f)
        : position(pos), size(siz), r(1.0f), g(1.0f), b(1.0f),
        useTexture(true), textureID(texID), textureRepeatX(repeatX), textureRepeatY(repeatY) {}

    void render() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glColor3f(r, g, b);

        if (useTexture && textureID != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }

        float halfWidth = size.x / 2.0f;
        float halfHeight = size.y / 2.0f;
        float halfDepth = size.z / 2.0f;

        glBegin(GL_QUADS);
        // Front face
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, -halfHeight, halfDepth);
        glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(halfWidth, -halfHeight, halfDepth);
        glTexCoord2f(textureRepeatX, textureRepeatY); glVertex3f(halfWidth, halfHeight, halfDepth);
        glTexCoord2f(0.0f, textureRepeatY); glVertex3f(-halfWidth, halfHeight, halfDepth);
        // Back face
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, -halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(-halfWidth, halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, textureRepeatY); glVertex3f(halfWidth, halfHeight, -halfDepth);
        glTexCoord2f(0.0f, textureRepeatY); glVertex3f(halfWidth, -halfHeight, -halfDepth);
        // Left face
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, -halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(-halfWidth, -halfHeight, halfDepth);
        glTexCoord2f(textureRepeatX, textureRepeatY); glVertex3f(-halfWidth, halfHeight, halfDepth);
        glTexCoord2f(0.0f, textureRepeatY); glVertex3f(-halfWidth, halfHeight, -halfDepth);
        // Right face
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(halfWidth, -halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(halfWidth, halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, textureRepeatY); glVertex3f(halfWidth, halfHeight, halfDepth);
        glTexCoord2f(0.0f, textureRepeatY); glVertex3f(halfWidth, -halfHeight, halfDepth);
        // Top face
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(-halfWidth, halfHeight, halfDepth);
        glTexCoord2f(textureRepeatX, textureRepeatY); glVertex3f(halfWidth, halfHeight, halfDepth);
        glTexCoord2f(0.0f, textureRepeatY); glVertex3f(halfWidth, halfHeight, -halfDepth);
        // Bottom face
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-halfWidth, -halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, 0.0f); glVertex3f(halfWidth, -halfHeight, -halfDepth);
        glTexCoord2f(textureRepeatX, textureRepeatY); glVertex3f(halfWidth, -halfHeight, halfDepth);
        glTexCoord2f(0.0f, textureRepeatY); glVertex3f(-halfWidth, -halfHeight, halfDepth);
        glEnd();

        if (useTexture && textureID != 0) {
            glDisable(GL_TEXTURE_2D);
        }

        glPopMatrix();
    }

    Vec3 getPosition() const { return position; }
    Vec3 getSize() const { return size; }
    Vec3 getMin() const {
        return Vec3(position.x - size.x / 2, position.y - size.y / 2, position.z - size.z / 2);
    }
    Vec3 getMax() const {
        return Vec3(position.x + size.x / 2, position.y + size.y / 2, position.z + size.z / 2);
    }
};

// ============== LADDER (unchanged) ==============
class Ladder : public GameObject {
private:
    Model* ladderModel;
    bool reached;
    float radius;
    float width, height, depth;
    Vec3 modelBoundsMin, modelBoundsMax;

public:
    Ladder(const Vec3& pos, const char* modelFile = "models/Ladder.obj") {
        position = pos;
        reached = false;
        radius = 0.8f;
        width = 1.0f;
        height = 3.0f;
        depth = 1.0f;

        ladderModel = ModelResourceManager::getInstance().getModel("models/Ladder.obj");
        if (ladderModel) {
            ladderModel->autoScaleToHeight(0.2f);
            ladderModel->getScaledBounds(modelBoundsMin, modelBoundsMax);
            width = modelBoundsMax.x - modelBoundsMin.x;
            height = modelBoundsMax.y - modelBoundsMin.y;
            depth = modelBoundsMax.z - modelBoundsMin.z;
            radius = ((width > height) ? (width > depth ? width : depth) : (height > depth ? height : depth)) * 0.5f;
            printf("Ladder model loaded\n");
        }
    }

    ~Ladder() {
    }

    void update(float deltaTime) override {}

    void render() override {
        if (reached) return;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(90, 1, 0, 0);
        glDisable(GL_COLOR_MATERIAL);

        if (ladderModel && ladderModel->hasMaterials()) {
            ladderModel->render();
        }
        else {
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
            glColor3f(0.6f, 0.4f, 0.2f);
            glBegin(GL_QUADS);
            glVertex3f(-0.1f, 0, -0.4f);
            glVertex3f(0.1f, 0, -0.4f);
            glVertex3f(0.1f, 3.0f, -0.4f);
            glVertex3f(-0.1f, 3.0f, -0.4f);
            glEnd();
        }

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glPopMatrix();

        if (!reached) {
            renderGlowEffect();
        }
    }

    void renderGlowEffect() {
        glPushMatrix();
        glTranslatef(position.x, position.y + 1.5f, position.z);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING);
        glDisable(GL_TEXTURE_2D);
        glColor4f(1.0f, 1.0f, 0.0f, 0.3f);
        GLUquadricObj* quad = gluNewQuadric();
        gluSphere(quad, 0.5f, 16, 16);
        gluDeleteQuadric(quad);
        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glPopMatrix();
    }

    bool checkReached(const Vec3& playerPos, float playerRadius) {
        if (reached) return false;
        float dist = Vec3::distance(position, playerPos);
        return dist < (radius + playerRadius);
    }

    void markReached() {
        reached = true;
        printf("Ladder reached!\n");
    }

    bool isReached() const { return reached; }
};

// ============== COLLECTIBLE WITH ANIMATION ==============
class Collectible : public GameObject {
private:
    float rotationSpeed;
    float bobHeight;
    float bobSpeed;
    float bobTime;
    int pointValue;
    float radius;
    float r, g, b;

    // ? ANIMATION VARIABLES
    bool isCollecting;
    float collectionTime;
    float collectionDuration;
    float initialScale;

public:
    Collectible(const Vec3& pos, int points, float red = 1.0f, float green = 0.8f, float blue = 0.0f) {
        position = pos;
        pointValue = points;
        rotationSpeed = 90.0f;
        bobHeight = 0.3f;
        bobSpeed = 2.0f;
        bobTime = 0.0f;
        radius = 0.3f;
        r = red; g = green; b = blue;

        // Initialize animation
        isCollecting = false;
        collectionTime = 0.0f;
        collectionDuration = 0.5f;  // 0.5 second animation
        initialScale = 1.0f;
    }

    void update(float deltaTime) override {
        if (!active) return;

        // ? COLLECTION ANIMATION
        if (isCollecting) {
            collectionTime += deltaTime;

            // Rotate faster during collection
            rotation.y += rotationSpeed * 3.0f * deltaTime;

            // Scale down to zero
            float t = collectionTime / collectionDuration;
            if (t >= 1.0f) {
                active = false;  // Disappear
                return;
            }

            // Smooth scale down with easing
            float scaleProgress = 1.0f - (t * t);  // Quadratic easing
            scale.x = scale.y = scale.z = initialScale * scaleProgress;

            return;
        }

        // Normal idle animation
        rotation.y += rotationSpeed * deltaTime;
        bobTime += deltaTime * bobSpeed;
    }

    void render() override {
        if (!active) return;

        float bob = sin(bobTime) * bobHeight;

        glPushMatrix();
        glTranslatef(position.x, position.y + bob, position.z);
        glRotatef(rotation.y, 0, 1, 0);

        // ? Apply scale during collection animation
        glScalef(scale.x, scale.y, scale.z);

        glColor3f(r, g, b);
        GLUquadricObj* quad = gluNewQuadric();
        gluSphere(quad, radius, 16, 16);
        gluDeleteQuadric(quad);

        glPopMatrix();
    }

    int getPointValue() const { return pointValue; }
    float getRadius() const { return radius; }

    bool checkCollection(const Vec3& playerPos, float playerRadius) {
        if (isCollecting) return false;  // Already collecting
        float dist = Vec3::distance(position, playerPos);
        return dist < (radius + playerRadius);
    }

    // ? START COLLECTION ANIMATION
    void startCollection() {
        if (!isCollecting && active) {
            isCollecting = true;
            collectionTime = 0.0f;

            // ? PLAY SOUND
            if (globalSoundPlayer) {
                globalSoundPlayer->playFile("collect.wav", true);
            }
        }
    }

    bool isBeingCollected() const { return isCollecting; }
    
    // Reset method for checkpoint system
    void reset() {
        active = true;
        isCollecting = false;
        collectionTime = 0.0f;
        scale = Vec3(1.0f, 1.0f, 1.0f);  // Reset per-instance scale
        // Note: We no longer need to restore model scale since we're using per-instance scaling
    }
};

// ============== MODEL COLLECTIBLE WITH SPARKLE ANIMATION ==============
class ModelCollectible : public GameObject {
private:
    Model* collectibleModel;
    int pointValue;
    float bobHeight;
    float bobSpeed;
    float bobTime;
    float rotationSpeed;
    float radius;
    bool collected;
    float width, height, depth;
    Vec3 modelBoundsMin, modelBoundsMax;

    // ? ANIMATION VARIABLES
    bool isCollecting;
    float collectionTime;
    float collectionDuration;
    float initialScale;
    float sparkleTime;

public:
    ModelCollectible(const Vec3& pos, int points, const char* modelFile) {
        position = pos;
        pointValue = points;
        bobHeight = 0.3f;
        bobSpeed = 2.0f;
        bobTime = 0.0f;
        rotationSpeed = 60.0f;
        radius = 0.5f;
        width = 0.6f;
        height = 0.6f;
        depth = 0.6f;
        collected = false;

        // Animation init
        isCollecting = false;
        collectionTime = 0.0f;
        collectionDuration = 0.7f;  // Longer for sparkle effect
        initialScale = 1.0f;
        sparkleTime = 0.0f;

        collectibleModel = ModelResourceManager::getInstance().getModel(modelFile);

        if (collectibleModel) {
            collectibleModel->autoScaleToFit(0.6f, 0.6f, 0.6f);
            collectibleModel->getScaledBounds(modelBoundsMin, modelBoundsMax);
            width = modelBoundsMax.x - modelBoundsMin.x;
            height = modelBoundsMax.y - modelBoundsMin.y;
            depth = modelBoundsMax.z - modelBoundsMin.z;
            radius = ((width > height) ? (width > depth ? width : depth) : (height > depth ? height : depth)) * 0.5f;
        }
    }

    ~ModelCollectible() {
    }

    void update(float deltaTime) override {
        if (!active) return;  // NEW: Check active state first
        if (collected) return;

        // ? COLLECTION ANIMATION
        if (isCollecting) {
            collectionTime += deltaTime;
            sparkleTime += deltaTime * 10.0f;  // Fast sparkle

            // Rotate faster
            rotation.y += rotationSpeed * 4.0f * deltaTime;

            // Scale down to zero
            float t = collectionTime / collectionDuration;
            if (t >= 1.0f) {
                collected = true;
                active = false;
                return;
            }

            // FIXED: Use per-instance scale instead of modifying shared model
            float scaleProgress = 1.0f - (t * t * t);  // Cubic easing
            scale.x = scale.y = scale.z = initialScale * scaleProgress;

            return;
        }

        // Normal animation
        bobTime += deltaTime * bobSpeed;
        rotation.y += rotationSpeed * deltaTime;
    }

    void render() override {
        if (!active) return;
        if (collected) return;

        float bob = sin(bobTime) * bobHeight;

        glPushMatrix();
        glTranslatef(position.x, position.y + bob, position.z);
        glRotatef(90, 1, 0, 0);
        glRotatef(rotation.y, 0, 1, 0);
        
        // FIXED: Apply per-instance scale using OpenGL instead of model scale
        if (isCollecting) {
            glScalef(scale.x, scale.y, scale.z);
        }

        glDisable(GL_COLOR_MATERIAL);

        if (collectibleModel) {
            collectibleModel->render();
        }

        // ? SPARKLE EFFECT during collection
        if (isCollecting) {
            renderSparkles();
        }

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        glPopMatrix();
    }

    // ? RENDER SPARKLES
    void renderSparkles() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);

        // Draw multiple sparkles
        for (int i = 0; i < 8; i++) {
            float angle = (sparkleTime + i * 45.0f) * M_PI / 180.0f;
            float distance = 0.5f + sin(sparkleTime + i) * 0.3f;

            glPushMatrix();
            glTranslatef(cos(angle) * distance, sin(angle) * distance, 0);

            float alpha = 0.8f * sin(sparkleTime * 2.0f);
            glColor4f(1.0f, 1.0f, 0.5f, alpha);

            GLUquadricObj* quad = gluNewQuadric();
            gluSphere(quad, 0.1f, 8, 8);
            gluDeleteQuadric(quad);

            glPopMatrix();
        }

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
    }

    bool checkCollection(const Vec3& playerPos, float playerRadius) {
        if (collected || isCollecting) return false;

        Vec3 currentPos = getCurrentPosition();
        float dist = Vec3::distance(currentPos, playerPos);
        float requiredDist = radius + playerRadius + 0.5f;

        return dist < requiredDist;
    }

    // ? START COLLECTION
    void collect() {
        if (!isCollecting && !collected) {
            isCollecting = true;
            collectionTime = 0.0f;

            // ? PLAY CHIME SOUND
            if (globalSoundPlayer) {
                globalSoundPlayer->playFile("collect.wav", true);
            }
        }
    }

    Vec3 getCurrentPosition() const {
        float bob = sin(bobTime) * bobHeight;
        return Vec3(position.x, position.y + bob, position.z);
    }

    bool isCollected() const { return collected; }
    float getRadius() const { return radius; }
    
    // Add getter for point value
    int getPointValue() const { return pointValue; }
    
    // Reset method for checkpoint system
    void reset() {
        collected = false;
        isCollecting = false;
        collectionTime = 0.0f;
        sparkleTime = 0.0f;
        active = true;
        scale = Vec3(1.0f, 1.0f, 1.0f);  // Reset per-instance scale
        // Note: We no longer need to restore model scale since we're using per-instance scaling
    }
};

// ============== KEY (with collection animation) ==============
class Key : public GameObject {
private:
    int keyID;
    float rotationSpeed;
    float bobHeight;
    float bobSpeed;
    float bobTime;
    float radius;
    bool collected;
    Model* keyModel;

    // ? ANIMATION
    bool isCollecting;
    float collectionTime;
    float collectionDuration;

public:
    Key(const Vec3& pos, int id) {
        position = pos;
        keyID = id;
        rotationSpeed = 120.0f;
        bobHeight = 0.2f;
        bobSpeed = 3.0f;
        bobTime = 0.0f;
        radius = 0.25f;
        collected = false;

        isCollecting = false;
        collectionTime = 0.0f;
        collectionDuration = 0.5f;

        keyModel = ModelResourceManager::getInstance().getModel("models/KEY_-_3D_MODEL.obj");
        if (keyModel) {
            keyModel->autoScaleToHeight(1.0f);
        }
    }

    void update(float deltaTime) override {
        if (collected) return;

        if (isCollecting) {
            collectionTime += deltaTime;
            rotation.y += rotationSpeed * 3.0f * deltaTime;

            float t = collectionTime / collectionDuration;
            if (t >= 1.0f) {
                collected = true;
                return;
            }

            // Move up and fade
            position.y += deltaTime * 2.0f;
            float scaleProgress = 1.0f - t;
            scale.x = scale.y = scale.z = scaleProgress;
            return;
        }

        rotation.y += rotationSpeed * deltaTime;
        bobTime += deltaTime * bobSpeed;
    }

    void render() override {
        if (collected) return;

        float bob = sin(bobTime) * bobHeight;

        glPushMatrix();
        glTranslatef(position.x, position.y + bob, position.z);
        glRotatef(rotation.y, 0, 1, 0);
        glScalef(scale.x, scale.y, scale.z);
		glDisable(GL_COLOR_MATERIAL);
        glColor3f(0.8f, 0.6f, 0.1f);    // Darker, more bronze-like gold
        if (keyModel && keyModel->hasMaterials()) {
            keyModel->render();
        }
        else {
            glColor3f(1.0f, 0.84f, 0.0f);
            GLUquadricObj* quad = gluNewQuadric();
            gluSphere(quad, 0.15f, 16, 16);
            glPushMatrix();
            glRotatef(90, 0, 1, 0);
            gluCylinder(quad, 0.05f, 0.05f, 0.3f, 8, 1);
            glPopMatrix();
            gluDeleteQuadric(quad);
        }
        glEnable(GL_COLOR_MATERIAL);
        glPopMatrix();
    }

    bool checkCollection(const Vec3& playerPos, float playerRadius) {
        if (collected || isCollecting) return false;
        float dist = Vec3::distance(position, playerPos);
        return dist < (radius + playerRadius);
    }

    void collect() {
        if (!isCollecting && !collected) {
            isCollecting = true;

            // ? PLAY KEY SOUND
            if (globalSoundPlayer) {
                globalSoundPlayer->playFile("collect.wav", true);
            }
        }
    }

    bool isCollected() const { return collected; }
    int getKeyID() const { return keyID; }
};

// ============== DOOR WITH SWING ANIMATION ==============
class Door : public GameObject {
private:
    Vec3 size;
    int requiredKeyID;
    bool isOpen;
    float openProgress;
    float openSpeed;
    Model* doorModel;

    // ? ANIMATION
    float swingAngle;  // Current swing angle
    float targetSwingAngle;  // Target angle (90 degrees when open)
    bool playedUnlockSound;

public:
    Door(const Vec3& pos, const Vec3& sz, int keyID,bool isVault = false) {
        position = pos;
        size = sz;
        requiredKeyID = keyID;
        isOpen = false;
        openProgress = 0.0f;
        openSpeed = 2.0f;
        swingAngle = 0.0f;
        targetSwingAngle = 0.0f;
        playedUnlockSound = false;

        doorModel = new Model();
        float modelScale;
        if (isVault) {
            doorModel = ModelResourceManager::getInstance().getModel("models/Vault_Door.obj");if (doorModel) {
                doorModel->autoScaleToHeight(1.0f);
                modelScale = doorModel->getScale().x;
                Vec3 minBounds, maxBounds;
                doorModel->getScaledBounds(minBounds, maxBounds);
            }
        }
        else {
            doorModel = ModelResourceManager::getInstance().getModel("models/door_door_metal.obj");
            
            if (doorModel) {
                doorModel->autoScaleToHeight(0.25f);
                modelScale = doorModel->getScale().x;
                Vec3 minBounds, maxBounds;
                doorModel->getScaledBounds(minBounds, maxBounds);
            }
        }
        
        // Set initial swing angle based on open/close state
        swingAngle = isOpen ? 90.0f : 0.0f;
        targetSwingAngle = swingAngle;
    }

    void update(float deltaTime) override {
        if (isOpen && openProgress < 1.0f) {
            openProgress += openSpeed * deltaTime;
            if (openProgress > 1.0f) openProgress = 1.0f;

            // ? SWING ANIMATION (smooth rotation)
            targetSwingAngle = 90.0f * openProgress;
            swingAngle += (targetSwingAngle - swingAngle) * deltaTime * 5.0f;
        }
    }

    void render() override {
        if (openProgress >= 0.99f) return;  // Fully open, don't render

        glPushMatrix();
        glTranslatef(position.x - 1.9f, position.y, position.z);

        // ? PIVOT ROTATION for swing effect
        glTranslatef(size.x * 0.5f, 0, 0);  // Move to hinge
        glRotatef(swingAngle, 0, 1, 0);     // Rotate
        glTranslatef(-size.x * 0.5f, 0, 0); // Move back

        glRotatef(90, 1, 0, 0);
        glRotatef(90, 0, 1, 0);
        glScalef(size.x, size.y * (1.0f - openProgress * 0.3f), size.z);
        glDisable(GL_COLOR_MATERIAL);

            glColor3f(0.7f, 0.7f, 0.7f);
        
        if (doorModel && doorModel->hasMaterials()) {
            doorModel->render();
        }
        else {
            glColor3f(0.5f, 0.3f, 0.1f);
            glBegin(GL_QUADS);
            glNormal3f(0, 0, 1);
            glVertex3f(-0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, -0.5f, 0.5f);
            glVertex3f(0.5f, 0.5f, 0.5f);
            glVertex3f(-0.5f, 0.5f, 0.5f);
            glNormal3f(0, 0, -1);
            glVertex3f(-0.5f, -0.5f, -0.5f);
            glVertex3f(-0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, 0.5f, -0.5f);
            glVertex3f(0.5f, -0.5f, -0.5f);
            glEnd();
        }

        glPopMatrix();
    }

    void unlock(int keyID) {
        if (keyID == requiredKeyID && !isOpen) {
            isOpen = true;

            // ? PLAY UNLOCK SOUND (only once)
            if (!playedUnlockSound && globalSoundPlayer) {
                globalSoundPlayer->playFile("door_unlock.wav", true);
                playedUnlockSound = true;
            }
        }
    }

    bool getIsOpen() const { return isOpen; }
    int getRequiredKeyID() const { return requiredKeyID; }
    Vec3 getMin() const { return position - size * 0.5f; }
    Vec3 getMax() const { return position + size * 0.5f; }
    bool blocksMovement() const { return !isOpen && openProgress < 0.5f; }
};

// ============== PIT (unchanged) ==============
class Pit : public GameObject {
private:
    Vec3 size;

public:
    Pit(const Vec3& pos, const Vec3& sz) {
        position = pos;
        size = sz;
    }

    void render() override {
        if (!active) return;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glScalef(size.x, size.y, size.z);

        glDisable(GL_LIGHTING);
        glColor3f(0.05f, 0.05f, 0.05f);
        glBegin(GL_QUADS);
        glNormal3f(0, 1, 0);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glEnd();
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    bool checkFall(const Vec3& playerPos, float playerRadius) {
        Vec3 pitMin = position - size * 0.5f;
        Vec3 pitMax = position + size * 0.5f;
        Vec3 playerMin = playerPos - Vec3(playerRadius, 0, playerRadius);
        Vec3 playerMax = playerPos + Vec3(playerRadius, 0, playerRadius);
        return (playerMin.x <= pitMax.x && playerMax.x >= pitMin.x) &&
            (playerMin.z <= pitMax.z && playerMax.z >= pitMin.z);
    }
};

// ============== PRESSURE PLATE (unchanged) ==============


// ============== TARGET OBJECT (simple version) ==============
class TargetObject : public GameObject {
private:
    float rotationSpeed;
    float radius;
    bool collected;
    float r, g, b;

public:
    TargetObject(const Vec3& pos, float red = 1.0f, float green = 0.8f, float blue = 0.0f) {
        position = pos;
        rotationSpeed = 50.0f;
        radius = 0.5f;
        collected = false;
        r = red; g = green; b = blue;
    }

    void update(float deltaTime) override {
        if (collected) return;
        rotation.y += rotationSpeed * deltaTime;
    }

    void render() override {
        if (collected) return;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation.y, 0, 1, 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glColor4f(r, g, b, 0.3f);

        GLUquadricObj* quad = gluNewQuadric();
        gluSphere(quad, radius * 1.5f, 16, 16);

        glDisable(GL_BLEND);
        glColor3f(r, g, b);
        gluSphere(quad, radius, 20, 20);

        gluDeleteQuadric(quad);
        glPopMatrix();
    }

    bool checkCollection(const Vec3& playerPos, float playerRadius) {
        if (collected) return false;
        float dist = Vec3::distance(position, playerPos);
        return dist < (radius + playerRadius + 0.5f);
    }

    void collect() { collected = true; }
    bool isCollected() const { return collected; }
};

// ============== MODEL TARGET WITH LEVITATION ==============
class ModelTarget : public GameObject {
private:
    Model* targetModel;
    float bobHeight;
    float bobSpeed;
    float bobTime;
    float glowIntensity;
    float glowTime;
    float radius;
    float width, height, depth;
    bool collected;
    Vec3 modelBoundsMin, modelBoundsMax;
    bool isScarab=false;
    // ? LEVITATION ANIMATION
    bool isCollecting;
    float collectionTime;
    float collectionDuration;
    float levitationHeight;

public:
    ModelTarget(const Vec3& pos, const char* modelFile,bool issScarab) {
        position = pos;
        bobHeight = 0.3f;
        bobSpeed = 1.0f;
        bobTime = 0.0f;
        glowIntensity = 0.3f;
        glowTime = 0.0f;
        radius = 1.0f;
        width = 1.2f;
        height = 1.2f;
        depth = 1.2f;
        collected = false;
        isScarab = issScarab;
        isCollecting = false;
        collectionTime = 0.0f;
        collectionDuration = 2.5f;  // Longer triumphant animation
        levitationHeight = 0.0f;

        targetModel = ModelResourceManager::getInstance().getModel(modelFile);

        if (targetModel) {
            targetModel->autoScaleToFit(1.2f, 1.2f, 1.2f);
            targetModel->getScaledBounds(modelBoundsMin, modelBoundsMax);
            width = modelBoundsMax.x - modelBoundsMin.x;
            height = modelBoundsMax.y - modelBoundsMin.y;
            depth = modelBoundsMax.z - modelBoundsMin.z;
            radius = ((width > height) ? (width > depth ? width : depth) : (height > depth ? height : depth)) * 0.5f;
            printf("Target model loaded\n");
        }
    }

    ~ModelTarget() {
    }

    void update(float deltaTime) override {
        if (collected) return;

        // ? LEVITATION ANIMATION
        if (isCollecting) {
            collectionTime += deltaTime;
            glowTime += deltaTime * 3.0f;

            // Levitate upward
            levitationHeight += deltaTime * 3.0f;

            // Spin faster
            rotation.y += 180.0f * deltaTime;

            // Pulsing glow
            glowIntensity = 0.8f + 0.2f * sin(glowTime * 3.0f);

            // Scale up then fade
            float t = collectionTime / collectionDuration;
            if (t >= 1.0f) {
                collected = true;
                active = false;
                return;
            }

            // Scale effect: grow then shrink
            float scalePhase;
            if (t < 0.4f) {
                scalePhase = 1.0f + (t / 0.4f) * 0.5f;  // Grow to 1.5x
            }
            else {
                scalePhase = 1.5f - ((t - 0.4f) / 0.6f) * 1.5f;  // Shrink to 0
            }

            // FIXED: Use per-instance scale instead of modifying shared model
            targetModel->setScale(targetModel->getScale().x * scalePhase / scale.x);
            scale.x = scale.y = scale.z = scalePhase;

            return;
        }

        bobTime += deltaTime * bobSpeed;
        glowTime += deltaTime * 2.0f;
        bobHeight = 0.1f + 0.05f * sin(glowTime * 0.5f);
        glowIntensity = 0.4f + 0.3f * sin(glowTime);
    }

    void render() override {
        if (collected) return;

        float bob = sin(bobTime) * bobHeight;

        glDisable(GL_COLOR_MATERIAL);

        glPushMatrix();
        glTranslatef(position.x, position.y + bob + levitationHeight, position.z);
        if (!isScarab) {
            glRotatef(180, 0, 0, 1);
            glRotatef(90, 1, 0, 0);
        }

        if (targetModel) {
            targetModel->render();
        }

        glPopMatrix();

        // ? EXPANDING GLOW during collection
        if (isCollecting) {
            renderTriumphantGlow();
        }

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    }

    // ? TRIUMPHANT GLOW EFFECT
    void renderTriumphantGlow() {
        glPushMatrix();
        glTranslatef(position.x, position.y + levitationHeight, position.z);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDisable(GL_LIGHTING);

        // Expanding golden rings
        float expansionRadius = collectionTime * 2.0f;
        for (int i = 0; i < 3; i++) {
            float ringRadius = expansionRadius + i * 0.5f;
            float alpha = (1.0f - collectionTime / collectionDuration) * 0.6f;

            glColor4f(1.0f, 0.8f, 0.0f, alpha);

            glPushMatrix();
            glRotatef(90, 1, 0, 0);
            GLUquadricObj* quad = gluNewQuadric();
            gluDisk(quad, ringRadius, ringRadius + 0.2f, 32, 1);
            gluDeleteQuadric(quad);
            glPopMatrix();
        }

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
        glPopMatrix();
    }

    bool checkCollection(const Vec3& playerPos, float playerRadius) {
        if (collected || isCollecting) return false;

        Vec3 currentPos = getCurrentPosition();
        float dist = Vec3::distance(currentPos, playerPos);
        float requiredDist = radius + playerRadius + 0.5f;

        return dist < requiredDist;
    }

    void collect() {
        if (!isCollecting && !collected) {
            isCollecting = true;
            collectionTime = 0.0f;

            // ? PLAY TRIUMPHANT SOUND
            if (globalSoundPlayer) {
                globalSoundPlayer->playFile("victory.wav", true);
            }
        }
    }

    Vec3 getCurrentPosition() const {
        float bob = sin(bobTime) * bobHeight;
        return Vec3(position.x, position.y + bob, position.z);
    }

    bool isCollected() const { return collected; }
    float getRadius() const { return radius; }
};

// ============== PATROL ENEMY WITH ANIMATIONS ==============
class PatrolEnemy : public GameObject {
private:
    std::vector<Vec3> waypoints;
    int currentWaypoint;
    float speed;
    float detectionRadius;
    float visionAngle;
    float radius;
    bool isDog;
    bool detectedPlayer;
    float detectionTime;
    float alertDuration;
    bool playerSpotted;
    float spottedTime;
    float catchDelay;
    Model* enemyModel;
    float modelScale;

    // ? ANIMATION STATE
    float walkCycleTime;
    float barkTime;
    bool isBarking;
    float barkCooldown;
    float barkDistance;  // Distance at which dog starts barking
    bool playedBarkSound;

    // NEW: Smooth wandering state
    float targetRotation;  // Target rotation angle for smooth turning
    float rotationSpeed;   // Speed of rotation in degrees per second
    float lookAheadDistance; // Distance to check for walls ahead
    bool isAvoidingCollision; // True when actively avoiding an obstacle
    float avoidanceTimer;  // Time spent in avoidance mode
    float avoidanceDuration; // How long to stay in avoidance mode before checking waypoint again
    Vec3 lastKnownClearPosition; // Last position where movement was clear

    // NEW: Stuck detection (simple anti-stuck system)
    Vec3 lastPositionCheck;  // Position from 2 seconds ago
    float stuckCheckTimer;   // Timer to check if stuck
    float stuckCheckInterval; // Check every 2 seconds (faster response)
    int unstuckAttempts;     // Track how many times we've tried to unstuck

    // NEW: Check if a position would collide with walls (with lookahead)
    bool wouldCollideWithWalls(const Vec3& newPos, const std::vector<Wall*>& walls, float extraLookAhead = 0.0f) {
        // Guard's AABB bounds (cylinder approximation as box)
        Vec3 guardMin = Vec3(newPos.x - radius, newPos.y, newPos.z - radius);
        Vec3 guardMax = Vec3(newPos.x + radius, newPos.y + 2.0f, newPos.z + radius);
        
        // Add extra lookahead if specified
        if (extraLookAhead > 0.0f) {
            float yawRad = rotation.y * M_PI / 180.0f;
            Vec3 forward(sin(yawRad), 0, cos(yawRad));
            Vec3 lookAheadPos = newPos + forward * extraLookAhead;
            guardMin = Vec3(lookAheadPos.x - radius, lookAheadPos.y, lookAheadPos.z - radius);
            guardMax = Vec3(lookAheadPos.x + radius, lookAheadPos.y + 2.0f, lookAheadPos.z + radius);
        }
        
        for (auto wall : walls) {
            Vec3 wallMin = wall->getMin();
            Vec3 wallMax = wall->getMax();
            
            // Skip floor and ceiling (very thin horizontal walls)
            float wallHeight = wallMax.y - wallMin.y;
            if (wallHeight < 2.0f) continue;
            
            // AABB collision check
            bool collisionX = guardMax.x >= wallMin.x && guardMin.x <= wallMax.x;
            bool collisionY = guardMax.y >= wallMin.y && guardMin.y <= wallMax.y;
            bool collisionZ = guardMax.z >= wallMin.z && guardMin.z <= wallMax.z;
            
            if (collisionX && collisionY && collisionZ) {
                return true; // Collision detected
            }
        }
        
        return false; // No collision
    }

    // NEW: Pick a random direction that's clear of walls
    void pickRandomAvoidanceDirection(const std::vector<Wall*>& walls) {
        // Try up to 12 random directions to find one that's clear
        for (int attempt = 0; attempt < 12; attempt++) {
            // Bias toward perpendicular directions (left/right) rather than backward
            float baseAngle = rotation.y;
            float randomOffset = ((rand() % 2) == 0 ? 70.0f : -70.0f) + ((rand() % 40) - 20); // 70° ± 20° left or right
            float randomAngle = baseAngle + randomOffset;
            
            // Normalize to [0, 360]
            while (randomAngle < 0) randomAngle += 360.0f;
            while (randomAngle >= 360.0f) randomAngle -= 360.0f;
            
            // Test this direction
            float testYaw = randomAngle * M_PI / 180.0f;
            Vec3 testForward(sin(testYaw), 0, cos(testYaw));
            Vec3 testPos = position + testForward * lookAheadDistance * 2.0f;
            
            if (!wouldCollideWithWalls(testPos, walls)) {
                // Found a clear direction!
                targetRotation = randomAngle;
                isAvoidingCollision = true;
                avoidanceTimer = 0.0;
                return;
            }
        }
        
        // If all attempts failed, turn 180 degrees
        targetRotation = fmod(rotation.y + 180.0f, 360.0f);
        isAvoidingCollision = true;
        avoidanceTimer = 0.0;
    }

    // NEW: Check if we can see and move toward our waypoint
    bool canMoveTowardWaypoint(const Vec3& waypointPos, const std::vector<Wall*>& walls) {
        Vec3 toWaypoint = waypointPos - position;
        float distToWaypoint = toWaypoint.length();
        
        if (distToWaypoint < 0.1f) return true; // Already at waypoint
        
        toWaypoint.normalize();
        
        // Check several points along the path to the waypoint
        float checkDistance = (distToWaypoint < lookAheadDistance * 2.0f) ? distToWaypoint : lookAheadDistance * 2.0f;
        
        for (float t = 0.5f; t <= checkDistance; t += 0.5f) {
            Vec3 checkPos = position + toWaypoint * t;
            if (wouldCollideWithWalls(checkPos, walls)) {
                return false;
            }
        }
        
        return true;
    }
    
    // NEW: Simple check for trash bin collision (inline, no forward declaration needed)
    bool checkTrashBinCollision(const Vec3& newPos, const std::vector<TrashBin*>& bins);  // Declared here, defined after TrashBin class

public:
    PatrolEnemy(const Vec3& startPos, bool dog = false) {
        position = startPos;
        currentWaypoint = 0;
        speed = 2.0f;
        detectionRadius = 8.0f;
        visionAngle = 60.0f;
        radius = 0.5f;
        isDog = dog;
        detectedPlayer = false;
        detectionTime = 0.0f;
        alertDuration = 2.0f;
        playerSpotted = false;
        spottedTime = 0.0f;
        catchDelay = 2.0f;

        // Animation init
        walkCycleTime = 0.0f;
        barkTime = 0.0f;
        isBarking = false;
        barkCooldown = 0.0f;
        barkDistance = 5.0f;
        playedBarkSound = false;

        // NEW: Initialize smooth movement variables
        targetRotation = 0.0f;
        rotationSpeed = 120.0f;  // Increased to 120 degrees/sec for more responsive turning
        lookAheadDistance = 1.8f;  // Increased lookahead
        isAvoidingCollision = false;
        avoidanceTimer = 0.0f;
        avoidanceDuration = 1.5f;  // Stay in avoidance mode for 1.5 seconds
        lastKnownClearPosition = startPos;

        // NEW: Initialize stuck detection
        lastPositionCheck = startPos;
        stuckCheckTimer = 0.0f;
        stuckCheckInterval = 2.0f;  // Check every 2 seconds (faster)
        unstuckAttempts = 0;

        enemyModel = new Model();
        modelScale = 1.0f;
        if (isDog) {
            enemyModel = ModelResourceManager::getInstance().getModel("models/dog.usdz.obj");
        }
        else {
            enemyModel = ModelResourceManager::getInstance().getModel("models/Low_poly_security_guard.obj");
        }
        if (isDog) {
            
                enemyModel->autoScaleToHeight(1.0f);
                modelScale = enemyModel->getScale().x;
                Vec3 minBounds, maxBounds;
                enemyModel->getScaledBounds(minBounds, maxBounds);
                float modelWidth = maxBounds.x - minBounds.x;
                float modelDepth = maxBounds.z - minBounds.z;
                radius = (((modelWidth) > (modelDepth)) ? (modelWidth) : (modelDepth)) * 0.5f;
                printf("Dog model loaded\n");
            
        }
        else {
                enemyModel->autoScaleToHeight(1.0f);
                modelScale = enemyModel->getScale().x;
                Vec3 minBounds, maxBounds;
                enemyModel->getScaledBounds(minBounds, maxBounds);
                float modelWidth = maxBounds.x - minBounds.x;
                float modelDepth = maxBounds.z - minBounds.z;
                radius = (((modelWidth) > (modelDepth)) ? (modelWidth) : (modelDepth)) * 0.5f;
                printf("Guard model loaded\n");
            
        }
        
        // Initialize target rotation to current rotation
        targetRotation = rotation.y;
    }

    ~PatrolEnemy() {
    }

    void addWaypoint(const Vec3& wp) {
        waypoints.push_back(wp);
    }

    // FIXED: Smooth movement without seizures, avoids trash bins, has stuck detection
    void update(float deltaTime, const std::vector<Wall*>& walls, const std::vector<TrashBin*>& trashBins) override {
        if (!active || waypoints.empty()) return;

        // Update bark cooldown
        if (barkCooldown > 0.0f) {
            barkCooldown -= deltaTime;
        }

        if (playerSpotted) {
            spottedTime += deltaTime;
            if (spottedTime >= catchDelay) {
                detectedPlayer = true;
            }
            return;
        }

        if (detectedPlayer) {
            detectionTime += deltaTime;
            if (detectionTime >= alertDuration) {
                detectedPlayer = false;
                detectionTime = 0.0f;
            }
            return;
        }

        // === STUCK DETECTION (Enhanced Anti-Stuck System) ===
        stuckCheckTimer += deltaTime;
        if (stuckCheckTimer >= stuckCheckInterval) {
            // Check if we've moved at least 0.3 units in 2 seconds
            float distMoved = Vec3::distance(position, lastPositionCheck);
            
            if (distMoved < 0.3f) {
                // Guard is stuck!
                unstuckAttempts++;
                
                if (unstuckAttempts < 3) {
                    // First few attempts: try small random offset to escape wall
                    float randomAngle = (rand() % 360) * M_PI / 180.0f;
                    float offsetDist = 1.0f + (rand() % 10) * 0.1f;  // 1.0 to 2.0 units
                    Vec3 randomOffset(
                        cos(randomAngle) * offsetDist,
                        0,
                        sin(randomAngle) * offsetDist
                    );
                    
                    Vec3 testPos = position + randomOffset;
                    
                    // Only move if new position doesn't collide
                    if (!wouldCollideWithWalls(testPos, walls) && 
                        !checkTrashBinCollision(testPos, trashBins)) {
                        position = testPos;
                        isAvoidingCollision = false;  // Reset avoidance state
                    }
                }
                else {
                    // After 3 failed attempts, teleport to FURTHEST waypoint
                    // (furthest is better than nearest - gets guard away from problem area)
                    Vec3 furthestWaypoint = waypoints[0];
                    float furthestDist = Vec3::distance(position, furthestWaypoint);
                    int furthestIndex = 0;
                    
                    for (size_t i = 1; i < waypoints.size(); i++) {
                        float dist = Vec3::distance(position, waypoints[i]);
                        if (dist > furthestDist) {  // Changed from < to >
                            furthestDist = dist;
                            furthestWaypoint = waypoints[i];
                            furthestIndex = i;
                        }
                    }
                    
                    // Teleport to furthest waypoint
                    position = furthestWaypoint;
                    currentWaypoint = furthestIndex;
                    isAvoidingCollision = false;
                    unstuckAttempts = 0;  // Reset counter
                }
            }
            else {
                // Guard moved successfully - reset attempt counter
                unstuckAttempts = 0;
            }
            
            // Reset timer and update check position
            lastPositionCheck = position;
            stuckCheckTimer = 0.0f;
        }

        // ? WALKING ANIMATION
        walkCycleTime += deltaTime * speed;

        // === SMOOTH ROTATION LOGIC ===
        float rotationDiff = targetRotation - rotation.y;
        
        // Normalize rotation difference to [-180, 180]
        while (rotationDiff > 180.0f) rotationDiff -= 360.0f;
        while (rotationDiff < -180.0f) rotationDiff += 360.0f;
        
        // Smoothly rotate towards target
        if (fabs(rotationDiff) > 0.5f) {  // Small threshold to prevent micro-adjustments
            float rotationStep = rotationSpeed * deltaTime;
            if (fabs(rotationDiff) < rotationStep) {
                rotation.y = targetRotation;
            } else {
                rotation.y += (rotationDiff > 0 ? rotationStep : -rotationStep);
            }
            
            // Normalize rotation to [0, 360]
            while (rotation.y < 0) rotation.y += 360.0f;
            while (rotation.y >= 360.0f) rotation.y -= 360.0f;
        }

        // === AVOIDANCE STATE MACHINE ===
        Vec3 currentTarget = waypoints[currentWaypoint];
        
        // Calculate movement direction based on current rotation
        float yawRad = rotation.y * M_PI / 180.0f;
        Vec3 moveDirection(sin(yawRad), 0, cos(yawRad));
        Vec3 newPosition = position + moveDirection * speed * deltaTime;
        Vec3 lookAheadPos = position + moveDirection * lookAheadDistance;
        
        // Check for imminent collision (walls AND trash bins)
        bool collisionAhead = wouldCollideWithWalls(lookAheadPos, walls) || 
                             checkTrashBinCollision(lookAheadPos, trashBins);
        
        if (isAvoidingCollision) {
            // In avoidance mode - stay in this mode for a minimum duration
            avoidanceTimer += deltaTime;
            
            // Check if we've avoided long enough AND can now see waypoint
            if (avoidanceTimer >= avoidanceDuration && !collisionAhead) {
                if (canMoveTowardWaypoint(currentTarget, walls)) {
                    // Exit avoidance mode and resume waypoint following
                    isAvoidingCollision = false;
                    
                    // Set rotation toward waypoint
                    Vec3 toTarget = (currentTarget - position).normalized();
                    targetRotation = atan2(toTarget.x, toTarget.z) * 180.0f / M_PI;
                    if (targetRotation < 0) targetRotation += 360.0f;
                }
            }
            
            // If still blocked after avoidance, pick a new direction
            if (avoidanceTimer >= avoidanceDuration && collisionAhead) {
                pickRandomAvoidanceDirection(walls);
            }
        }
        else {
            // Normal waypoint following mode
            if (collisionAhead) {
                // Collision detected - enter avoidance mode
                pickRandomAvoidanceDirection(walls);
            }
            else {
                // Path is clear - smoothly move toward waypoint
                Vec3 toTarget = (currentTarget - position).normalized();
                float desiredRotation = atan2(toTarget.x, toTarget.z) * 180.0f / M_PI;
                if (desiredRotation < 0) desiredRotation += 360.0f;
                
                // Only update target rotation if significantly different (prevents jitter)
                float rotDiff = desiredRotation - targetRotation;
                while (rotDiff > 180.0f) rotDiff -= 360.0f;
                while (rotDiff < -180.0f) rotDiff += 360.0f;
                
                if (fabs(rotDiff) > 5.0f) {  // 5 degree threshold
                    targetRotation = desiredRotation;
                }
            }
        }
        
        // === MOVEMENT ===
        // Only move if we're facing roughly the right direction (prevents sliding)
        float facingDiff = fabs(rotationDiff);
        if (facingDiff < 45.0f) {  // Only move if facing within 45 degrees of target
            // Check both walls AND trash bins before moving
            if (!wouldCollideWithWalls(newPosition, walls) && 
                !checkTrashBinCollision(newPosition, trashBins)) {
                lastKnownClearPosition = position;
                position = newPosition;
            }
        }
        
        // === WAYPOINT CHECKING ===
        // Only check waypoint distance when not in avoidance mode
        if (!isAvoidingCollision) {
            float distToWaypoint = Vec3::distance(position, currentTarget);
            if (distToWaypoint < 1.0f) {  // Slightly larger threshold
                // Reached waypoint - move to next
                currentWaypoint = (currentWaypoint + 1) % waypoints.size();
                Vec3 nextTarget = waypoints[currentWaypoint];
                Vec3 toNext = (nextTarget - position).normalized();
                targetRotation = atan2(toNext.x, toNext.z) * 180.0f / M_PI;
                if (targetRotation < 0) targetRotation += 360.0f;
            }
        }
    }

    // ? CHECK IF SHOULD BARK (for dogs)
    bool shouldBark(const Vec3& playerPos) {
        if (!isDog) return false;
        


        float dist = Vec3::distance(position, playerPos);
        if (dist < barkDistance && dist > 2.0f) {  // Close but not too close
            return true;
        }
        return false;
    }

    // ? PLAY BARK SOUND
    void bark() {
        if (globalSoundPlayer && !playedBarkSound) {
            globalSoundPlayer->playFile("dog_bark.wav", true);
            playedBarkSound = true;
            isBarking = true;
            barkTime = 0.0f;
        }
    }

    void render() override {
        if (!active) return;

        glDisable(GL_COLOR_MATERIAL);

        glPushMatrix();

        // ? WALKING BOB ANIMATION
        float walkBob = 0.0f;
        if (!playerSpotted && !detectedPlayer) {
            walkBob = sin(walkCycleTime * 4.0f) * 0.05f;  // Subtle bob
        }

        glTranslatef(position.x, position.y + (isDog ? 0.8f : 1.8f) + walkBob, position.z);

        if (isDog) {
            glRotatef(rotation.y + 90, 0, 1, 0);
        }
        else {
            glRotatef(rotation.y, 0, 1, 0);
            glRotatef(-90, 1, 0, 0);
        }

        if (enemyModel && enemyModel->hasMaterials()) {
            if (isDog) {
                // Dog - brown color
                glColor3f(0.6f, 0.4f, 0.2f);
                glDisable(GL_LIGHTING);
                enemyModel->render();
                glEnable(GL_LIGHTING);
            }
            else {
                // Guard - dark blue/black uniform
                enemyModel->render();
            }


        }
        else {
            // Fallback rendering
            if (isDog) {
                glColor3f(0.6f, 0.4f, 0.2f);
                GLUquadricObj* quad = gluNewQuadric();
                glPushMatrix();
                glRotatef(90, 0, 0, 1);
                gluCylinder(quad, 0.3f, 0.3f, 0.8f, 16, 1);
                glPopMatrix();
                glPushMatrix();
                glTranslatef(0.5f, 0, 0);
                gluSphere(quad, 0.25f, 16, 16);
                glPopMatrix();
                gluDeleteQuadric(quad);
            }
            else {
                glColor3f(0.8f, 0.2f, 0.2f);
                GLUquadricObj* quad = gluNewQuadric();
                gluCylinder(quad, 0.4f, 0.4f, 1.5f, 16, 1);
                glTranslatef(0, 0, 1.5f);
                gluSphere(quad, 0.3f, 16, 16);
                gluDeleteQuadric(quad);
            }
        }

        glPopMatrix();

        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

        renderVisionCone();

        if (playerSpotted && !detectedPlayer) {
            renderAlertSymbol();
        }
    }

    void renderVisionCone() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LIGHTING);

        if (playerSpotted) {
            glColor4f(1.0f, 0.0f, 0.0f, 0.4f);
        }
        else {
            glColor4f(1.0f, 1.0f, 0.8f, 0.25f);
        }

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(rotation.y, 0, 1, 0);

        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0, 0, 0);

        int segments = 20;
        for (int i = 0; i <= segments; i++) {
            float angle = (-visionAngle + (2.0f * visionAngle * i / segments)) * M_PI / 180.0f;
            float x = sin(angle) * detectionRadius;
            float z = cos(angle) * detectionRadius;
            glVertex3f(x, -position.y + 0.1f, z);
        }

        glEnd();

        if (playerSpotted) {
            glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
        }
        else {
            glColor4f(1.0f, 1.0f, 0.5f, 0.6f);
        }
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex3f(0, 0, 0);
        for (int i = 0; i <= segments; i++) {
            float angle = (-visionAngle + (2.0f * visionAngle * i / segments)) * M_PI / 180.0f;
            float x = sin(angle) * detectionRadius;
            float z = cos(angle) * detectionRadius;
            glVertex3f(x, -position.y + 0.1f, z);
        }
        glEnd();
        glLineWidth(1.0f);

        glPopMatrix();

        glEnable(GL_LIGHTING);
        glDisable(GL_BLEND);
    }

    void renderAlertSymbol() {
        glPushMatrix();
        glTranslatef(position.x, position.y + 2.5f, position.z);

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // Yellow circle
        glColor3f(1.0f, 1.0f, 0.0f);
        GLUquadricObj* quad = gluNewQuadric();
        gluSphere(quad, 0.4f, 16, 16);
        gluDeleteQuadric(quad);

        // Draw "?" (simplified)
        glColor3f(0.0f, 0.0f, 0.0f);
        glPushMatrix();
        glScalef(0.01f, 0.01f, 0.01f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 10; i++) {
            float angle = M_PI * i / 10.0f;
            glVertex3f(10 * cos(angle), 15 + 10 * sin(angle), 0);
        }
        glEnd();
        glBegin(GL_LINES);
        glVertex3f(0, 5, 0);
        glVertex3f(0, -5, 0);
        glEnd();
        glPopMatrix();

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    bool detectPlayer(const Vec3& playerPos) {
        Vec3 toPlayer = playerPos - position;
        float distance = toPlayer.length();

        // ? CHECK IF SHOULD BARK (dogs only)
        /*if (shouldBark(playerPos)) {
            bark();
        }
        else {
            globalSoundPlayer->playFile("gaurd_grunt.wav");
        }*/

        if (distance > detectionRadius) {
            if (playerSpotted && !detectedPlayer) {
                playerSpotted = false;
                spottedTime = 0.0f;
                playedBarkSound = false;
            }
            return false;
        }

        float yawRad = rotation.y * M_PI / 180.0f;
        Vec3 forward(sin(yawRad), 0, cos(yawRad));

        toPlayer.y = 0;
        toPlayer.normalize();

        float dotProduct = forward.dot(toPlayer);
        float angleToPlayer = acos(dotProduct) * 180.0f / M_PI;

        if (angleToPlayer < visionAngle) {
            if (!playerSpotted && !detectedPlayer) {
                playerSpotted = true;
                spottedTime = 0.0f;

                // ? PLAY ALERT SOUND
                if (isDog) {
                    bark();
                }
                else {
                    globalSoundPlayer->playFile("gaurd_grunt.wav");
                }
            }

            return detectedPlayer;
        }
        else {
            if (playerSpotted && !detectedPlayer) {
                playerSpotted = false;
                spottedTime = 0.0f;
                playedBarkSound = false;
            }
        }

        return false;
    }

    Vec3 getPosition() const { return position; }
    float getRadius() const { return radius; }
    bool isDetecting() const { return detectedPlayer; }
    bool isSpotting() const { return playerSpotted; }

    void reset() {
        playerSpotted = false;
        detectedPlayer = false;
        spottedTime = 0.0f;
        detectionTime = 0.0f;
        playedBarkSound = false;
        isBarking = false;
        barkCooldown = 0.0f;
    }

    void scaleModel(float scaleFactor) {
        if (enemyModel) {
            Vec3 currentScale = enemyModel->getScale();
            enemyModel->setScale(currentScale.x * scaleFactor);
            modelScale *= scaleFactor;

            Vec3 minBounds, maxBounds;
            enemyModel->getScaledBounds(minBounds, maxBounds);
            float modelWidth = maxBounds.x - minBounds.x;
            float modelDepth = maxBounds.z - minBounds.z;
            radius = (((modelWidth) > (modelDepth)) ? (modelWidth) : (modelDepth)) * 0.5f;
        }
    }
};

// ============== TRASH BIN (Simple Obstacle) ==============
class TrashBin : public GameObject {
private:
    Model* binModel;
    float radius;
    float width, height, depth;
    Vec3 modelBoundsMin, modelBoundsMax;
    GLuint textureID;  // Optional texture support
    bool useTexture;

public:
    TrashBin(const Vec3& pos, const char* modelFile = "models/trashbin.obj") {
        position = pos;
        radius = 0.5f;  // Default radius for collision
        width = 1.0f;
        height = 1.0f;
        depth = 1.0f;
        useTexture = false;
        textureID = 0;

        // Load the model

        binModel = ModelResourceManager::getInstance().getModel("models/Modern_Trash_Bin__Low_Poly.obj");
        if (binModel) {
            // Auto-scale to reasonable size (adjust as needed)
            binModel->autoScaleToHeight(1.5f);

            // Get the actual bounds for collision
            binModel->getScaledBounds(modelBoundsMin, modelBoundsMax);
            width = modelBoundsMax.x - modelBoundsMin.x;
            height = modelBoundsMax.y - modelBoundsMin.y;
            depth = modelBoundsMax.z - modelBoundsMin.z;

            // Set collision radius based on model size
            radius = ((width > height) ? (width > depth ? width : depth) : (height > depth ? height : depth)) * 0.5f;
            printf("Trash bin model loaded successfully\n");
        }
        else {
            // Fallback to default size if model fails to load
            printf("Failed to load trash bin model, using default\n");
            width = 0.8f;
            height = 1.5f;
            depth = 0.8f;
            radius = 0.6f;
        }
    }

    ~TrashBin() {
    }

    void update(float deltaTime) override {
        // Trash bin is static, no updates needed
    }

    void render() override {
        if (!active) return;

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glRotatef(-90, 1, 0, 0);
        // Optional: Apply texture if available



        if (useTexture && textureID != 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, textureID);
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }

        glDisable(GL_COLOR_MATERIAL);  // Turn off material tracking
        glColor3f(0.3f, 0.3f, 0.3f);   // Set simple gray color


        if (binModel && binModel->hasMaterials()) {
            // Use the model's materials
            glDisable(GL_LIGHTING);
            binModel->render();
            glEnable(GL_LIGHTING);
        }
        else {
            // Fallback rendering (simple cylinder)
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

            // Draw a simple cylinder (trash bin shape)
            glColor3f(0.3f, 0.3f, 0.3f);  // Gray color

            GLUquadricObj* quad = gluNewQuadric();

            // Main cylinder
            gluQuadricNormals(quad, GLU_SMOOTH);
            glPushMatrix();
            glRotatef(-90, 1, 0, 0);  // Rotate to stand upright
            gluCylinder(quad, radius * 0.8f, radius * 0.9f, height, 16, 1);

            // Top rim
            glTranslatef(0, 0, height);
            gluDisk(quad, 0, radius * 0.9f, 16, 1);

            // Bottom base
            glTranslatef(0, 0, -height);
            gluDisk(quad, 0, radius * 0.7f, 16, 1);

            glPopMatrix();
            gluDeleteQuadric(quad);

            glDisable(GL_COLOR_MATERIAL);
        }

        glEnable(GL_COLOR_MATERIAL);
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();

        // Optional: Render collision bounds for debugging
        // renderCollisionBounds();
    }

    // Check collision with player
    bool checkCollision(const Vec3& playerPos, float playerRadius) {
        Vec3 binPos2D = Vec3(position.x, 0, position.z);
        Vec3 playerPos2D = Vec3(playerPos.x, 0, playerPos.z);
        float horizontalDist = Vec3::distance(binPos2D, playerPos2D);

        // Also check vertical overlap (player should be near the ground level)
        float verticalOverlap = fabs(playerPos.y - position.y) < (height * 0.5f + playerRadius);

        return horizontalDist < (radius + playerRadius) && verticalOverlap;
    }

    // Get bounding box for broad-phase collision
    Vec3 getMinBounds() const {
        return Vec3(position.x - radius, position.y, position.z - radius);
    }

    Vec3 getMaxBounds() const {
        return Vec3(position.x + radius, position.y + height, position.z + radius);
    }

    // For debugging: render collision bounds
    void renderCollisionBounds() {
        glPushMatrix();
        glTranslatef(position.x, position.y + height * 0.5f, position.z);

        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Render collision cylinder
        glColor4f(1.0f, 0.0f, 0.0f, 0.2f);

        GLUquadricObj* quad = gluNewQuadric();
        glPushMatrix();
        glRotatef(-90, 1, 0, 0);
        gluCylinder(quad, radius, radius, height, 16, 1);
        gluDisk(quad, 0, radius, 16, 1);
        glTranslatef(0, 0, height);
        gluDisk(quad, 0, radius, 16, 1);
        glPopMatrix();
        gluDeleteQuadric(quad);

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glPopMatrix();
    }

    // Optional: Set texture
    void setTexture(GLuint texID) {
        textureID = texID;
        useTexture = true;
    }

    // Getter for collision properties
    float getRadius() const { return radius; }
    float getHeight() const { return height; }
    Vec3 getPosition() const { return position; }
};

// ============== IMPLEMENTATION OF PatrolEnemy::checkTrashBinCollision ==============
// (Must be defined AFTER TrashBin class)
inline bool PatrolEnemy::checkTrashBinCollision(const Vec3& newPos, const std::vector<TrashBin*>& bins) {
    for (size_t i = 0; i < bins.size(); i++) {
        auto bin = bins[i];
        if (!bin) continue;
        
        Vec3 binPos = bin->getPosition();
        float binRadius = bin->getRadius();
        
        float dx = newPos.x - binPos.x;
        float dz = newPos.z - binPos.z;
        float dist = sqrt(dx * dx + dz * dz);
        
        if (dist < (radius + binRadius + 0.3f)) {  // Small buffer
            return true;
        }
    }
    return false;
}

#endif // GAMEOBJECT_H