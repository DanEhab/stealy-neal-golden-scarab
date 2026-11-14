/*
 * GameManager.h - Enhanced game state and logic management
 */

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <ctime>
#include <cstdio>
#include <set>
#include "Vec3.h"
#include "Camera.h"
#include "Player.h"
#include "GameObject.h"
#include "SecurityDevices.h"
#include "Level.h"
#include "Lighting.h"
#include "Renderer.h"
#include "SoundPlayer.h"
enum GameState {
    GAME_ACTIVE,
    GAME_MENU,
    GAME_OVER,
    GAME_WIN,
    LEVEL_TRANSITION
};

class GameManager {
private:
    Camera* camera;
    Player* player;
    Level* currentLevel;
    LightingManager* lighting;
    Renderer* renderer;
    SoundPlayer* soundPlayer;
    GameState state;
    int currentLevelIndex;

    // Timing
    clock_t lastTime;
    float deltaTime;

    // Input state
    bool keys[256];
    bool specialKeys[256];

    // Inventory
    std::set<int> collectedKeys;  // Track collected keys

    // Flashlight state (Level 2)
    bool flashlightOn;
    
    // NEW: Track if Room 2 coins should be removed (Level 1 punishment)
    bool room2CoinsRemoved;
    
    // NEW: Music toggle state
    bool musicOn;

    void playSound(const char* soundName) {
        soundPlayer->playFile(soundName);
    }

public:


    GameManager() {
        camera = new Camera();
        player = new Player(camera);
        lighting = new LightingManager();
        renderer = new Renderer();
        soundPlayer = new SoundPlayer();
        currentLevelIndex = 0;
        state = GAME_ACTIVE;
        flashlightOn = false;
        room2CoinsRemoved = false;  // NEW: Initialize coin removal flag
        musicOn = true;  // NEW: Music starts ON

        currentLevel = new MuseumLevel();
        player->setPosition(currentLevel->getPlayerStartPosition());

        lastTime = clock();

        for (int i = 0; i < 256; i++) {
            keys[i] = false;
            specialKeys[i] = false;
        }
       /* soundPlayer->playBackgroundMusic();*/
    }

    ~GameManager() {
        delete camera;
        delete player;
        delete currentLevel;
        delete lighting;
        delete renderer;
    }
    
    void update() {
        clock_t currentTime = clock();
        deltaTime = (float)(currentTime - lastTime) / CLOCKS_PER_SEC;
        lastTime = currentTime;

        if (deltaTime > 0.1f) deltaTime = 0.1f;

        if (state == GAME_ACTIVE) {
            player->update(deltaTime);
            currentLevel->update(deltaTime);
            lighting->update(deltaTime);
            camera->updateShake(deltaTime);  // Update camera shake
            checkCollisions();

            // Check if EITHER target is collected to advance level
            bool targetCollected = false;

            if (
                currentLevel->getTargetObject() &&
                currentLevel->getTargetObject()->isCollected()) {
                targetCollected = true;
            }

            if ( currentLevel->getModelTarget() &&
                currentLevel->getModelTarget()->isCollected()) {
                targetCollected = true;
            }

            if (currentLevelIndex!=1 && targetCollected) {
                globalSoundPlayer->playFile("victory.wav");
                nextLevel();
            }
        }
    }

    void checkCollisions() {
        Vec3 playerPos = player->getPosition();
        float playerRadius = player->getRadius();

        // Wall collisions
        for (auto wall : currentLevel->getWalls()) {
            Vec3 wallMin = wall->getMin();
            Vec3 wallMax = wall->getMax();

            float wallHeight = wallMax.y - wallMin.y;
            if (wallHeight < 2.0f) continue;

            if (player->checkBoxCollision(wallMin, wallMax)) {
                player->resolveCollision(wallMin, wallMax);
            }
        }

        // Door collisions
        for (auto door : currentLevel->getDoors()) {
            if (door->blocksMovement()) {
                Vec3 doorMin = door->getMin();
                Vec3 doorMax = door->getMax();

                if (player->checkBoxCollision(doorMin, doorMax)) {
                    player->resolveCollision(doorMin, doorMax);
                }
            }
        }



        if (currentLevelIndex == 1) {
            Ladder* ladder = currentLevel->getLadder();
            if (ladder && !ladder->isReached() && ladder->checkReached(playerPos, playerRadius)) {
                ladder->markReached();

                // CELEBRATION SEQUENCE
                if (globalSoundPlayer) {
                    globalSoundPlayer->stopBackgroundMusic();  // Stop tense music
                    globalSoundPlayer->playFile("victory.wav", true);  // Victory fanfare
                }

                if (currentLevel->isTargetCollected()) {
                    player->addScore(50);
                    printf(" ESCAPED WITH THE CROWN! Bonus +50 points!\n");
                }
                else {
                    printf(" Escaped successfully!\n");
                }

                // Small delay before level transition for effect
                Sleep(500);  // Windows sleep for half second

                nextLevel();
                return;
            }
        }
        // Key collection


        // Collectible collection
        if (currentLevel) {
            auto& collectibles = currentLevel->getCollectibles();
            for (auto c : collectibles) {
                if (c->isActive() && !c->isBeingCollected() &&
                    c->checkCollection(player->getPosition(), player->getRadius())) {
                    // Start animation instead of instant removal
                    c->startCollection();
                    player->addScore(c->getPointValue());
                    printf("Collected! Score: %d\n", player->getScore());
                }
            }

            // DON'T erase collected items - keep them in vector so they can be restored on reset
            // They won't render when !isActive() anyway
        }

        if (currentLevel) {
            auto& modelCollectibles = currentLevel->getModelCollectibles();
            for (auto mc : modelCollectibles) {
                if (mc->isActive() && !mc->isCollected() &&
                    mc->checkCollection(player->getPosition(), player->getRadius())) {
                    mc->collect();  // This starts the animation
                    player->addScore(mc->getPointValue());
                    printf("Model collectible collected! +%d points. Total: %d\n",
                        mc->getPointValue(), player->getScore());
                }
            }

            // DON'T erase collected items - keep them in vector so they can be restored on reset
            // They won't render when isCollected() anyway
        }

        if (currentLevel->getTargetObject()) {
            if (currentLevel->getTargetObject()->checkCollection(playerPos, playerRadius)) {
                currentLevel->getTargetObject()->collect();  // Starts animation
                // Don't add score immediately - wait for animation
                currentLevel->setTargetCollected(true);
                player->addScore(100);
                printf("Crown collected! Bonus points! Find the ladder to escape.\n");
            }
        }

        if (currentLevel->getModelTarget()) {
            if (currentLevel->getModelTarget()->checkCollection(playerPos, playerRadius)) {
                currentLevel->getModelTarget()->collect();  // Starts animation
                currentLevel->setTargetCollected(true);
                player->addScore(100);
                printf("Crown collected! Bonus points! Find the ladder to escape.\n");
            }
        }

        for (auto key : currentLevel->getKeys()) {
            if (!key->isCollected() && key->checkCollection(playerPos, playerRadius)) {
                key->collect();  // Starts animation
                collectedKeys.insert(key->getKeyID());
                player->addScore(50);

                // Unlock matching doors
                for (auto door : currentLevel->getDoors()) {
                    if (collectedKeys.count(door->getRequiredKeyID()) > 0) {
                        door->unlock(door->getRequiredKeyID());
                        // Sound is played automatically by Door class
                    }
                }
            }
        }

        // Enemy detection - ENHANCED WITH STUMBLE, SHAKE, AND SCORE DEDUCTION
        for (auto enemy : currentLevel->getEnemies()) {
            if (enemy->detectPlayer(playerPos)) {
                if (enemy->shouldBark(playerPos)) {
                    enemy->bark();
                }
                else {
                    globalSoundPlayer->playFile("gaurd_grunt.wav");
                }
                if (currentLevelIndex == 0 && currentLevel->isInRoom2()) {
                    resetToCheckpoint();
                    lighting->setLevel2Detected(true);
                    return;
                }
                else if (currentLevelIndex == 1 && currentLevel->isInRoom2()) {
                    resetToRoom2Checkpoint();
                    lighting->setLevel2Detected(true);
                    return;
                }
                else {
                    gameOver();
                    return;
                }
            }
        }
        
        // NEW: Physical body collision with guards (ONLY in Level 1 - not Room 2)
        if (currentLevelIndex == 0 && !currentLevel->isInRoom2()) {
            for (auto enemy : currentLevel->getEnemies()) {
                // Check physical distance - much smaller than vision cone
                float enemyRadius = enemy->getRadius();
                float collisionDist = enemyRadius + playerRadius + 0.2f;  // Small buffer
                float actualDist = Vec3::distance(playerPos, enemy->getPosition());
                
                if (actualDist < collisionDist) {
                    // Physical collision detected!
                    printf("? GUARD COLLISION! Player bumped into guard!\n");
                    
                    // Play grunt sound
                    if (globalSoundPlayer) {
                        globalSoundPlayer->playFile("gaurd_grunt.wav");
                    }
                    
                    // Trigger camera shake
                    camera->shake(0.25f, 0.5f);
                    
                    // Apply knockback
                    player->applyKnockback(enemy->getPosition(), 1.2f);
                    
                    // Deduct score
                    player->deductScore(15);
                    
                    // Brief pause to see effect
                    Sleep(150);
                    
                    // Don't trigger game over - just the effects!
                    break;  // Only process one collision per frame
                }
            }
        }

        // Camera detection
        for (auto cam : currentLevel->getCameras()) {
            if (cam->detectPlayer(playerPos)) {
                if (currentLevelIndex == 0 && currentLevel->isInRoom2()) {
                    resetToCheckpoint();
                    lighting->setLevel2Detected(true);  // Turn red lights on
                    globalSoundPlayer->playFile("alarm.wav");
                    return;
                }
                else if (currentLevelIndex == 1 && currentLevel->isInRoom2()) {
                    // Level 2 Room 2 - reset to Room 2 checkpoint
                    resetToRoom2Checkpoint();
                    lighting->setLevel2Detected(true);  // Turn red lights on
                    globalSoundPlayer->playFile("alarm.wav");
                    return;
                }
                else {
                    gameOver();
                    globalSoundPlayer->playFile("alarm.wav");
                    return;
                }
            }
        }

        // Laser detection
        for (auto laser : currentLevel->getLasers()) {
            if (laser->detectPlayer(playerPos, playerRadius)) {
                if (currentLevelIndex == 0 && currentLevel->isInRoom2()) {
                    globalSoundPlayer->playFile("laser_hit.wav");
                    resetToCheckpoint();
                    lighting->setLevel2Detected(true);  // Turn red lights on
                    return;
                }
                else if (currentLevelIndex == 1 && currentLevel->isInRoom2()) {
                    globalSoundPlayer->playFile("laser_hit.wav");
                    resetToRoom2Checkpoint();
                    lighting->setLevel2Detected(true);  // Turn red lights on
                    return;
                }
                else {
                    globalSoundPlayer->playFile("laser_hit.wav");
                    gameOver();
                    return;
                }
            }
        }

        // Motion sensor detection
        for (auto sensor : currentLevel->getSensors()) {
            if (sensor->detectPlayer(playerPos)) {
                if (currentLevelIndex == 1 && currentLevel->isInRoom2()) {
                    resetToRoom2Checkpoint();
                    lighting->setLevel2Detected(true);  // Turn red lights on
                    globalSoundPlayer->playFile("alarm.wav");
                    return;
                }
                else {
                    gameOver();
                    globalSoundPlayer->playFile("alarm.wav");
                    return;
                }
            }
        }

        // Pit detection
        for (auto pit : currentLevel->getPits()) {
            if (pit->checkFall(playerPos, playerRadius)) {
                if (currentLevelIndex == 1 && currentLevel->isInRoom2()) {
                    resetToRoom2Checkpoint();
                    lighting->setLevel2Detected(true);  // Turn red lights on
                    globalSoundPlayer->playFile("fall_scream.wav");
                    return;
                }
                else {
                    gameOver();
                    globalSoundPlayer->playFile("fall_scream.wav");
                    return;
                }
            }
        }

        // PRESSURE PLATE DETECTION - MODIFIED FOR PUZZLE LOGIC
        // For Level 2, we need to check if player is on safe path
        if (currentLevelIndex == 1) {
            for (size_t i = 0; i < currentLevel->getPressurePlates().size(); i++) {
                auto plate = currentLevel->getPressurePlates()[i];
                if (plate->checkTrigger(playerPos, playerRadius)) {
                    // Calculate which plate this is in the 5x5 grid
                    int plateIndex = i;

                    // Safe path indices (0-24 for 5x5 grid)
                    // Based on the path defined in VaultLevel
                    std::vector<int> safePlateIndices = {
    0,  // (0,0)
    5,  // (1,0)
    10, // (2,0)
    11, // (2,1)
    12, // (2,2)
    13, // (2,3)
    14, // (2,4)
    19, // (3,4)
    24  // (4,4)
                    };


                    bool isSafe = false;
                    for (int safeIdx : safePlateIndices) {
                        if (plateIndex == safeIdx) {
                            isSafe = true;
                            break;
                        }
                    }

                    if (!isSafe) {
                        // Wrong plate - player dies
                        gameOver();
                        globalSoundPlayer->playFile("explosion.wav");
                        return;
                    }
                    // If safe plate, nothing happens - player can continue
                }
            }
        }

        // Check if player entered Room 2 (Level 1)
        if (currentLevelIndex == 0 && playerPos.z > -10 && !currentLevel->isInRoom2()) {
            currentLevel->setInRoom2(true);
        }

        // Check if player entered Room 2 (Level 2)
        if (currentLevelIndex == 1 && playerPos.z > 0 && !currentLevel->isInRoom2()) {
            currentLevel->setInRoom2(true);
            printf("Entered Room 2 - Checkpoint saved!\n");
        }
    }

    // Add new method for Level 2 Room 2 checkpoint
    void resetToRoom2Checkpoint() {
        // Reset to Room 2 start position
        player->reset(currentLevel->getRoom2StartPosition());

        // Reset only Room 2 objects
        for (auto plate : currentLevel->getPressurePlates()) {
            // Don't reset Room 1 plates if player is in Room 2
            if (plate->getPosition().z > 0) {
                plate->reset();
            }
        }

        for (auto enemy : currentLevel->getEnemies()) {
            // Only reset Room 2 enemies
            if (enemy->getPosition().z > 0) {
                enemy->reset();
            }
        }

        for (auto sensor : currentLevel->getSensors()) {
            // Only reset Room 2 sensors
            if (sensor->getPosition().z > 0) {
                sensor->reset();  // MAKE SURE THIS IS CALLED
            }
        }
        
        // FIXED: Reset Room 2 collectibles so they reappear
        for (auto collectible : currentLevel->getCollectibles()) {
            if (collectible->getPosition().z > 0) {
                collectible->reset();
            }
        }
        
        for (auto modelCollectible : currentLevel->getModelCollectibles()) {
            if (modelCollectible->getPosition().z > 0) {
                modelCollectible->reset();
            }
        }

        printf("Reset to Room 2 checkpoint!\n");
    }

    void resetToCheckpoint() {
        // Reset to Room 2 start position
        player->reset(currentLevel->getRoom2StartPosition());

        // NEW: PERMANENTLY REMOVE ROOM 2 COINS AS PUNISHMENT (Level 1 only)
        if (currentLevelIndex == 0 && !room2CoinsRemoved) {  // Level 1 only, first time
            room2CoinsRemoved = true;  // Set flag
            
            // Deactivate all coins in Room 2 (z > -10)
            for (auto collectible : currentLevel->getCollectibles()) {
                if (collectible->getPosition().z > -10) {
                    collectible->setActive(false);  // Permanently remove
                }
            }
            
            for (auto modelCollectible : currentLevel->getModelCollectibles()) {
                if (modelCollectible->getPosition().z > -10) {
                    modelCollectible->setActive(false);  // Permanently remove
                }
            }
            
            printf("\n?? DETECTED! Room 2 coins removed as punishment!\n");
        }

        // Reset pressure plates
        for (auto plate : currentLevel->getPressurePlates()) {
            plate->reset();
        }

        // Reset guard states
        for (auto enemy : currentLevel->getEnemies()) {
            enemy->reset();
        }

        // Reset sensor states
        for (auto sensor : currentLevel->getSensors()) {
            sensor->reset();
        }
        
        // DON'T reset Room 2 collectibles if they've been removed as punishment
        if (currentLevelIndex != 0 || !room2CoinsRemoved) {
            for (auto collectible : currentLevel->getCollectibles()) {
                collectible->reset();
            }
            
            for (auto modelCollectible : currentLevel->getModelCollectibles()) {
                modelCollectible->reset();
            }
        }
    }

    void render() {
        if (state == GAME_ACTIVE) {
            if (currentLevelIndex == 0) {
                lighting->setupMuseumLights();
            }
            else {
                Vec3 playerPos = player->getPosition();
                Vec3 playerForward = camera->getForward();

                // Get the actual Crown position from the level's model target
                Vec3 crownPosition = Vec3(0, 2.5f, 52);  // Default fallback
                if (currentLevel->getModelTarget()) {
                    crownPosition = currentLevel->getModelTarget()->getPosition();
                    
                    // DEBUG: Print Crown position to console EVERY FRAME so you can see it
                    static int frameCount = 0;
                    frameCount++;
                    if (frameCount % 60 == 0) {  // Print every 60 frames (once per second)
                        printf("=== CROWN POSITION: X=%.2f, Y=%.2f, Z=%.2f ===\n", 
                               crownPosition.x, crownPosition.y, crownPosition.z);
                    }
                }

                // Pass the detection state AND crown position to lighting
                lighting->setupVaultLights(playerPos, playerForward, flashlightOn, lighting->areRedLightsOn(), crownPosition);

                // Pass flashlight state to level for glow tiles
                currentLevel->setFlashlightState(flashlightOn);

                // Render flashlight in player's hand (Level 2 only)
                player->renderFlashlight(flashlightOn);
            }
            // In render() method after rendering HUD
            if (currentLevelIndex == 1) {
                // Show crown collection status
                std::string crownStatus = currentLevel->isTargetCollected() ?
                    "Crown: COLLECTED" : "Crown: NOT FOUND";

                // Show escape instruction
                std::string escapeText = "Find the ladder to escape!";

                // Show alert status if red lights are on
                if (lighting->areRedLightsOn()) {
                    std::string alertText = "ALERT: SECURITY TRIPPED!";
                    glDisable(GL_LIGHTING);
                    glMatrixMode(GL_PROJECTION);
                    glPushMatrix();
                    glLoadIdentity();
                    gluOrtho2D(0, 800, 0, 600);
                    glMatrixMode(GL_MODELVIEW);
                    glPushMatrix();
                    glLoadIdentity();

                    glColor3f(1.0f, 0.0f, 0.0f);
                    glRasterPos2f(10, 70);
                    for (char c : alertText) {
                        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
                    }

                    glPopMatrix();
                    glMatrixMode(GL_PROJECTION);
                    glPopMatrix();
                    glMatrixMode(GL_MODELVIEW);
                    glEnable(GL_LIGHTING);
                }

                // Render crown status and escape text
                glDisable(GL_LIGHTING);
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                gluOrtho2D(0, 800, 0, 600);
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                glLoadIdentity();

                glColor3f(1.0f, 1.0f, 0.0f);
                glRasterPos2f(10, 50);
                for (char c : crownStatus) {
                    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
                }

                glColor3f(0.0f, 1.0f, 1.0f);
                glRasterPos2f(10, 30);
                for (char c : escapeText) {
                    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
                }

                glPopMatrix();
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
                glEnable(GL_LIGHTING);
            }
            camera->apply(player->getPosition());
            currentLevel->render();
            player->render();
            renderer->renderHUD(player->getScore(), currentLevelIndex + 1, flashlightOn, musicOn);  // Pass musicOn state
        }
        else if (state == GAME_OVER) {
            renderer->renderGameOver();
        }
        else if (state == GAME_WIN) {
            renderer->renderGameWin(player->getScore());
        }
    }

    void nextLevel() {
        currentLevelIndex++;
        room2CoinsRemoved = false;  // NEW: Reset for next level

        if (currentLevelIndex >= 2) {
            state = GAME_WIN;
            globalSoundPlayer->playFile("victory.wav");
        }
        else {
            delete currentLevel;

            if (currentLevelIndex == 1) {
                currentLevel = new VaultLevel();
                flashlightOn = true;  // Auto-enable flashlight in dark level
                lighting->resetForNewLevel();  // Reset lights for new level
            }

            collectedKeys.clear();
            player->reset(currentLevel->getPlayerStartPosition());
        }
    }

    void gameOver() {
        state = GAME_OVER;

        if (globalSoundPlayer) {
            globalSoundPlayer->playFile("gaurd_grunt.wav", true);
        }
    }

    void restartLevel() {
        collectedKeys.clear();
        player->reset(currentLevel->getPlayerStartPosition());
        state = GAME_ACTIVE;
        
        room2CoinsRemoved = false;  // NEW: Reset coin removal flag

        // Reset lighting state
        lighting->resetForNewLevel();

        delete currentLevel;
        if (currentLevelIndex == 0) {
            currentLevel = new MuseumLevel();
            flashlightOn = false;
        }
        else {
            currentLevel = new VaultLevel();
            flashlightOn = true;
        }

        // Reset all guard states
        for (auto enemy : currentLevel->getEnemies()) {
            enemy->reset();
        }
    }


    void handleKeyboard(unsigned char key, int x, int y) {
        keys[key] = true;

        if (state == GAME_ACTIVE) {
            if (key == 'w' || key == 'W') player->setMovingForward(true);
            if (key == 's' || key == 'S') player->setMovingBackward(true);
            if (key == 'a' || key == 'A') player->setMovingLeft(true);
            if (key == 'd' || key == 'D') player->setMovingRight(true);
            if (key == 'h' || key == 'H') {
                player->toggleHitbox();
            }
            if (key == ' ') player->jump();

            if (key == 'c' || key == 'C') {
                camera->toggleMode();
            }

            // Toggle flashlight (Level 2 only)
            if (key == 'f' || key == 'F') {
                if (currentLevelIndex == 1) {
                    flashlightOn = !flashlightOn;
                    globalSoundPlayer->playFile("flashlight-toggle.wav");
                }
            }
            
            // NEW: Toggle background music with M key
            if (key == 'm' || key == 'M') {
                musicOn = !musicOn;
                
                if (musicOn) {
                    // Unmute - restart the music
                    globalSoundPlayer->playBackgroundMusic("The Pink Panther Theme Music.mp3");
                    printf("Music UNMUTED - Pink Panther theme playing\n");
                } else {
                    // Mute - stop the music
                    globalSoundPlayer->stopBackgroundMusic();
                    printf("Music MUTED\n");
                }
            }
        }
        else if (state == GAME_OVER) {
            if (key == 'r' || key == 'R') {
                restartLevel();
            }
        }
        else if (state == GAME_WIN) {
            if (key == 'r' || key == 'R') {
                currentLevelIndex = 0;
                collectedKeys.clear();
                room2CoinsRemoved = false;  // NEW: Reset coin removal flag
                delete currentLevel;
                currentLevel = new MuseumLevel();
                flashlightOn = false;
                player->resetScore();
                player->reset(currentLevel->getPlayerStartPosition());
                state = GAME_ACTIVE;
            }
        }
    }

    void handleKeyboardUp(unsigned char key, int x, int y) {
        keys[key] = false;

        if (key == 'w' || key == 'W') player->setMovingForward(false);
        if (key == 's' || key == 'S') player->setMovingBackward(false);
        if (key == 'a' || key == 'A') player->setMovingLeft(false);
        if (key == 'd' || key == 'D') player->setMovingRight(false);
    }

    void handleSpecialKeys(int key, int x, int y) {
        specialKeys[key] = true;

        if (state == GAME_ACTIVE) {
            if (key == GLUT_KEY_UP) player->setMovingForward(true);
            if (key == GLUT_KEY_DOWN) player->setMovingBackward(true);
            if (key == GLUT_KEY_LEFT) player->setMovingLeft(true);
            if (key == GLUT_KEY_RIGHT) player->setMovingRight(true);
        }
    }

    void handleSpecialKeysUp(int key, int x, int y) {
        specialKeys[key] = false;

        if (key == GLUT_KEY_UP) player->setMovingForward(false);
        if (key == GLUT_KEY_DOWN) player->setMovingBackward(false);
        if (key == GLUT_KEY_LEFT) player->setMovingLeft(false);
        if (key == GLUT_KEY_RIGHT) player->setMovingRight(false);
    }

    void handleMouseMotion(int deltaX, int deltaY) {
        if (state == GAME_ACTIVE) {
            camera->handleMouseMove((float)deltaX, (float)deltaY);
        }
    }

    int getCollectedKeyCount() const { return collectedKeys.size(); }
};

#endif // GAMEMANAGER_H