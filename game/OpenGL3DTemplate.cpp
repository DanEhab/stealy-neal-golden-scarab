/*
 * Stealy Neal: The Golden Scarab
 * Main entry point - UPDATED WITH GLOBAL SOUND PLAYER
 *
 * DMET 502 Computer Graphics - Final Project
 */

#include <windows.h>
#include <glut.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <vector>
#include <cstdio>
#include "Vec3.h"
#include "Camera.h"
#include "Player.h"
#include "GameObject.h"
#include "SecurityDevices.h"
#include "Level.h"
#include "Lighting.h"
#include "Renderer.h"

#include "SoundPlayer.h"
#include "GameManager.h"
//#include "LoadingScreen.h"
#include "ModelResourceManager.h"
 // ? GLOBAL SOUND PLAYER (accessible by all game objects)
SoundPlayer* globalSoundPlayer = nullptr;

// Global game manager
GameManager* gameManager = nullptr;

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

// Mouse state
ModelResourceManager* ModelResourceManager::instance = nullptr;
//float LoadingScreen::currentProgress = 0.0f;
//std::string LoadingScreen::currentStatus = "Initializing...";
//float LoadingScreen::spinnerAngle = 0.0f;
int lastMouseX = WINDOW_WIDTH / 2;
int lastMouseY = WINDOW_HEIGHT / 2;
bool mouseCaptured = true;

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameManager) {
        gameManager->render();
    }

    glutSwapBuffers();
}

void reshape(int width, int height) {
    if (height == 0) height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)width / (float)height, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    if (gameManager) {
        gameManager->handleKeyboard(key, x, y);
    }

    if (key == 27) {
        exit(0);
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    if (gameManager) {
        gameManager->handleKeyboardUp(key, x, y);
    }
}

void specialKeys(int key, int x, int y) {
    if (gameManager) {
        gameManager->handleSpecialKeys(key, x, y);
    }
}

void specialKeysUp(int key, int x, int y) {
    if (gameManager) {
        gameManager->handleSpecialKeysUp(key, x, y);
    }
}

void mouseMotion(int x, int y) {
    if (!mouseCaptured) return;

    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    int deltaX = x - centerX;
    int deltaY = y - centerY;

    if (abs(deltaX) > 2 || abs(deltaY) > 2) {
        if (gameManager) {
            gameManager->handleMouseMotion(deltaX, deltaY);
        }

        glutWarpPointer(centerX, centerY);
        lastMouseX = centerX;
        lastMouseY = centerY;
    }
}

void passiveMouseMotion(int x, int y) {
    mouseMotion(x, y);
}



void idle() {
    if (gameManager) {
        gameManager->update();
    }
    glutPostRedisplay();
}

void initGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_SMOOTH);
    TextureLoader::loadTextures();
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

   

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void loadGameResources() {
    printf("\n ===== OPTIMIZED LOADING STARTED =====\n\n");

    // Step 1: Initialize OpenGL (10%)
    //LoadingScreen::show("Initializing graphics...", 0.1f);
    initGL();
    Sleep(100);  // Brief pause so user sees the loading screen

    // Step 2: Load textures (20%)
    //LoadingScreen::show("Loading textures...", 0.2f);
    TextureLoader::loadTextures();

    // Step 3: Create sound player (30%)
    //LoadingScreen::show("Initializing audio...", 0.3f);
    globalSoundPlayer = new SoundPlayer();

    // Register sound files (no actual loading yet - Windows does lazy loading)
    globalSoundPlayer->registerSound("collect", "collect.wav");
    globalSoundPlayer->registerSound("door_unlock", "door_unlock.wav");
    globalSoundPlayer->registerSound("guard_alert", "guard_alert.wav");
    globalSoundPlayer->registerSound("guard_grunt", "guard_grunt.wav");
    globalSoundPlayer->registerSound("alarm", "alarm.wav");
    globalSoundPlayer->registerSound("victory", "victory.wav");
    globalSoundPlayer->registerSound("dog_bark", "dog_bark.wav");
    globalSoundPlayer->registerSound("camera_beep", "camera_beep.wav");
    globalSoundPlayer->registerSound("level_complete", "level_complete.wav");
    globalSoundPlayer->registerSound("laser_hit", "laser_hit.wav");
    globalSoundPlayer->registerSound("fall_scream", "fall_scream.wav");

    // Step 4: Preload all models (40% - 80%)
    //  THIS IS THE BIG OPTIMIZATION - Load each model ONCE
    //LoadingScreen::show("Loading 3D models...", 0.4f);

    auto& modelManager = ModelResourceManager::getInstance();

    // Load models with progress updates
    modelManager.getModel("models/dog.usdz.obj");
    //LoadingScreen::update(0.45f);

    modelManager.getModel("models/Low_poly_security_guard.obj");
    //LoadingScreen::update(0.5f);

    modelManager.getModel("models/surveillance_camera.obj");
    //LoadingScreen::update(0.55f);

    modelManager.getModel("models/Motion_sensor.obj");
    //LoadingScreen::update(0.6f);

    modelManager.getModel("models/KEY_-_3D_MODEL.obj");
    //LoadingScreen::update(0.65f);

    modelManager.getModel("models/door_door_metal.obj");
    //LoadingScreen::update(0.7f);

    modelManager.getModel("models/original_Coin.obj");
    //LoadingScreen::update(0.73f);

    modelManager.getModel("models/Diamond.obj");
    //LoadingScreen::update(0.76f);


    //LoadingScreen::update(0.79f);

    modelManager.getModel("models/Crown.obj");
    //LoadingScreen::update(0.82f);

    modelManager.getModel("models/Ladder.obj");
    //LoadingScreen::update(0.85f);

    modelManager.getModel("models/elite.obj");
    //LoadingScreen::update(0.9f);
    modelManager.getModel("models/Modern_Trash_Bin__Low_Poly.obj");
    modelManager.getModel("models/golden_scarab.obj");
    modelManager.getModel("models/Vault_Door.obj");
    // Step 5: Create game manager (90%)
    //LoadingScreen::show("Building game world...", 0.9f);
    gameManager = new GameManager();

    // Step 6: Start Pink Panther theme music! (95%)
    //LoadingScreen::show("Starting background music...", 0.95f);
    printf("\n Starting The Pink Panther Theme Music...\n");
    globalSoundPlayer->playBackgroundMusic("The Pink Panther Theme Music.mp3");

    // Step 7: Complete! (100%)
    //LoadingScreen::show("Ready!", 1.0f);
    Sleep(300);  // Show "Ready!" briefly

    // Print stats
    modelManager.printStats();
    printf("\n ===== LOADING COMPLETE =====\n\n");
}
int main(int argc, char** argv) {
    printf(" Stealy Neal: The Golden Scarab\n");
    printf(" Optimized Build - Fast Loading Edition\n\n");

    // Initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Stealy Neal: The Golden Scarab - Loading...");

    //  LOAD EVERYTHING WITH PROGRESS BAR
    loadGameResources();

    // Update window title
    glutSetWindowTitle("Stealy Neal: The Golden Scarab");

    // Register callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialKeys);
    glutSpecialUpFunc(specialKeysUp);
    glutPassiveMotionFunc(passiveMouseMotion);
    glutMotionFunc(mouseMotion);
    glutIdleFunc(idle);

    // Setup window
    glutSetCursor(GLUT_CURSOR_NONE);
    glutFullScreen();

    // Center mouse
    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;
    glutWarpPointer(centerX, centerY);
    lastMouseX = centerX;
    lastMouseY = centerY;

    printf(" Game started! Enjoy!\n\n");

    // Start main loop
    glutMainLoop();

    // Cleanup
    printf("\n Cleaning up...\n");
    delete gameManager;
    delete globalSoundPlayer;

    // Model cleanup handled by ModelResourceManager destructor
    printf(" Cleanup complete. Goodbye!\n");

    return 0;
}