/*
 * Enhanced Level.h - Level 2 with Pressure Plate Puzzle Room
 *
 * Level 2 now has TWO rooms:
 * Room 1: Pressure plate puzzle - player must follow the safe path shown on ceiling
 * Room 2: Original vault room with pits, lasers, and target objective
 */

#ifndef LEVEL_H
#define LEVEL_H

#include "Vec3.h"
#include "GameObject.h"
#include "SecurityDevices.h"
#include <vector>
#include "TextureLoader.h"

 // Glowing ceiling tile for path visualization
class GlowTile {
private:
    Vec3 position;
    Vec3 size;
    float glowIntensity;
    float glowTime;

public:
    GlowTile(const Vec3& pos, const Vec3& sz) : position(pos), size(sz), glowIntensity(0.0f), glowTime(0.0f) {}

    void update(float deltaTime) {
        glowTime += deltaTime * 2.0f;
        glowIntensity = 0.6f + 0.4f * sin(glowTime);
    }

    void render() {
       
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // Disable lighting for glowing effect
        glDisable(GL_LIGHTING);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // Bright green glow
        glColor4f(0.0f, glowIntensity, 0.0f, 0.9f);

        glBegin(GL_QUADS);
        // Bottom face (visible from below)
        glVertex3f(-size.x / 2, -size.y / 2, -size.z / 2);
        glVertex3f(size.x / 2, -size.y / 2, -size.z / 2);
        glVertex3f(size.x / 2, -size.y / 2, size.z / 2);
        glVertex3f(-size.x / 2, -size.y / 2, size.z / 2);
        glEnd();

        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);

        glPopMatrix();
    }
};

// Helper for transparent glass rendering
class DisplayCase {
private:
    Vec3 position;
    Vec3 size;
public:
    DisplayCase(const Vec3& pos, const Vec3& sz) : position(pos), size(sz) {}

    void render() {
        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);

        // Safety: Disable texturing so we don't accidentally map the floor texture to the glass
        glDisable(GL_TEXTURE_2D);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Draw Frame (Wireframe)
        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);
        glColor3f(0.1f, 0.1f, 0.1f); // Black frame
        glScalef(size.x, size.y, size.z);
        glutWireCube(1.0);

        // Draw Glass (Transparent Box)
        // We render it slightly smaller (0.98) to prevent "z-fighting" with the wireframe
        glColor4f(0.8f, 0.9f, 1.0f, 0.15f); // Blueish tint, high transparency
        glutSolidCube(0.98);

        // Reset state to avoid messing up other textures (like pressure plates)
        glDisable(GL_BLEND);
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D); // Re-enable textures for subsequent objects
        glLineWidth(1.0f);

        glPopMatrix();
    }
};


class Level {
protected:
    std::vector<Wall*> walls;
    std::vector<Collectible*> collectibles;
    std::vector<ModelCollectible*> modelCollectibles;
    std::vector<PatrolEnemy*> enemies;
    std::vector<SecurityCamera*> cameras;
    std::vector<LaserGrid*> lasers;
    std::vector<MotionSensor*> sensors;
    std::vector<Key*> keys;
    std::vector<Door*> doors;
    std::vector<Pit*> pits;
    std::vector<PressurePlate*> pressurePlates;
    std::vector<TrashBin*> trashBins;
    std::vector<GlowTile*> glowTiles;  // Add glow tiles
    std::vector<DisplayCase*> displayCases;
    Ladder* ladder;
    TargetObject* targetObject;
    ModelTarget* modelTarget;
    Vec3 playerStartPos;
    Vec3 room2StartPos;  // Checkpoint for Room 2
    bool flashlightOn;
    bool levelComplete;
    bool inRoom2;
    bool targetCollected;

public:
    Level() : targetObject(nullptr), modelTarget(nullptr),
        ladder(nullptr), levelComplete(false),
        inRoom2(false), targetCollected(false) ,flashlightOn(false) {}
    

    virtual ~Level() {
        cleanup();
    }

    void cleanup() {
        for (auto w : walls) delete w;
        for (auto c : collectibles) delete c;
        for (auto mc : modelCollectibles) delete mc;
        for (auto e : enemies) delete e;
        for (auto cam : cameras) delete cam;
        for (auto l : lasers) delete l;
        for (auto s : sensors) delete s;
        for (auto k : keys) delete k;
        for (auto d : doors) delete d;
        for (auto p : pits) delete p;
        for (auto pp : pressurePlates) delete pp;
        for (auto gt : glowTiles) delete gt;

        walls.clear();
        collectibles.clear();
        modelCollectibles.clear();
        enemies.clear();
        cameras.clear();
        lasers.clear();
        sensors.clear();
        keys.clear();
        doors.clear();
        pits.clear();
        pressurePlates.clear();
        glowTiles.clear();


        if (targetObject) {
            delete targetObject;
            targetObject = nullptr;
        }
        if (modelTarget) {
            delete modelTarget;
            modelTarget = nullptr;
        }
        if (ladder) {
            delete ladder;
            ladder = nullptr;
        }
    }

    virtual void update(float deltaTime) {
        for (auto c : collectibles) c->update(deltaTime);
        for (auto mc : modelCollectibles) mc->update(deltaTime);
        
        // Pass both walls AND trash bins to enemies
        for (auto e : enemies) e->update(deltaTime, walls, trashBins);
        
        for (auto cam : cameras) cam->update(deltaTime);
        for (auto l : lasers) l->update(deltaTime);
        for (auto s : sensors) s->update(deltaTime);
        for (auto k : keys) k->update(deltaTime);
        for (auto d : doors) d->update(deltaTime);
        for (auto pp : pressurePlates) pp->update(deltaTime);
        for (auto gt : glowTiles) gt->update(deltaTime);

        if (targetObject) targetObject->update(deltaTime);
        if (modelTarget) modelTarget->update(deltaTime);
        if (ladder) ladder->update(deltaTime);
    }

    virtual void render() {
        for (auto w : walls) w->render();
        for (auto p : pits) p->render();
        for (auto c : collectibles) c->render();
        for (auto mc : modelCollectibles) mc->render();
        for (auto e : enemies) e->render();
        for (auto cam : cameras) cam->render();
        for (auto l : lasers) l->render();
        for (auto s : sensors) s->render();
        for (auto k : keys) k->render();
        for (auto d : doors) d->render();
        for (auto pp : pressurePlates) pp->render();
        if (flashlightOn) {
            for (auto gt : glowTiles) gt->render();
        }
        for (auto bin : trashBins) bin->render();
        if (targetObject) targetObject->render();
        if (modelTarget) modelTarget->render();
        if (ladder) ladder->render();
        for (auto dc : displayCases) dc->render();
    }
    void setFlashlightState(bool on) { flashlightOn = on; }
    Vec3 getPlayerStartPosition() const { return playerStartPos; }
    Vec3 getRoom2StartPosition() const { return room2StartPos; }
    Ladder* getLadder() { return ladder; }
    bool isTargetCollected() const { return targetCollected; }
    void setTargetCollected(bool collected) { targetCollected = collected; }
    bool isComplete() const { return levelComplete; }
    bool isInRoom2() const { return inRoom2; }
    void setInRoom2(bool val) { inRoom2 = val; }

    // Collision checking
    std::vector<Wall*>& getWalls() { return walls; }
    std::vector<Collectible*>& getCollectibles() { return collectibles; }
    std::vector<ModelCollectible*>& getModelCollectibles() { return modelCollectibles; }
    std::vector<PatrolEnemy*>& getEnemies() { return enemies; }
    std::vector<SecurityCamera*>& getCameras() { return cameras; }
    std::vector<LaserGrid*>& getLasers() { return lasers; }
    std::vector<MotionSensor*>& getSensors() { return sensors; }
    std::vector<Key*>& getKeys() { return keys; }
    std::vector<Door*>& getDoors() { return doors; }
    std::vector<Pit*>& getPits() { return pits; }
    std::vector<PressurePlate*>& getPressurePlates() { return pressurePlates; }
    std::vector<TrashBin*>& getTrashBins() { return trashBins; }
    TargetObject* getTargetObject() { return targetObject; }
    ModelTarget* getModelTarget() { return modelTarget; }
};

// Level 1: Museum (unchanged)
class MuseumLevel : public Level {
public:
    MuseumLevel() {
        playerStartPos = Vec3(0.0f, 1.0f, -38.0f);
        room2StartPos = Vec3(0.0f, 1.0f, 2.0f);
        buildLevel();
    }

private:
    void buildLevel() {
        cleanup();

        GLuint wallTex = TextureLoader::getWallTexture();
        GLuint floorTex = TextureLoader::getFloorTexture();



        // Floor and Ceiling
        walls.push_back(new Wall(Vec3(0, -0.5f, 0), Vec3(60, 1, 80), floorTex, 15.0f, 20.0f));
        walls.push_back(new Wall(Vec3(0, 6.5f, 0), Vec3(60, 1, 80), wallTex, 15.0f, 2.0f));

        // Room 1: Maze
        walls.push_back(new Wall(Vec3(0, 3, -40), Vec3(50, 6, 1), wallTex, 12.5f, 1.5f));
        walls.push_back(new Wall(Vec3(-13.5f, 3, -10), Vec3(23, 6, 1), wallTex, 5.75f, 1.5f));
        walls.push_back(new Wall(Vec3(13.5f, 3, -10), Vec3(23, 6, 1), wallTex, 5.75f, 1.5f));
        walls.push_back(new Wall(Vec3(-25, 3, -25), Vec3(1, 6, 30), wallTex, 0.75f, 1.5f));
        walls.push_back(new Wall(Vec3(25, 3, -25), Vec3(1, 6, 30), wallTex, 0.75f, 1.5f));
        walls.push_back(new Wall(Vec3(-15, 3, -35), Vec3(1, 6, 8), wallTex, 0.25f, 1.5f));
        walls.push_back(new Wall(Vec3(-10, 3, -30), Vec3(12, 6, 1), wallTex, 3.0f, 1.5f));
        walls.push_back(new Wall(Vec3(5, 3, -35), Vec3(1, 6, 12), wallTex, 0.3f, 1.5f));
        walls.push_back(new Wall(Vec3(15, 3, -28), Vec3(1, 6, 10), wallTex, 0.25f, 1.5f));
        walls.push_back(new Wall(Vec3(-5, 3, -23), Vec3(8, 6, 1), wallTex, 2.0f, 1.5f));
        walls.push_back(new Wall(Vec3(10, 3, -20), Vec3(12, 6, 1), wallTex, 3.0f, 1.5f));
        walls.push_back(new Wall(Vec3(-18, 3, -18), Vec3(1, 6, 8), wallTex, 0.2f, 1.5f));
        walls.push_back(new Wall(Vec3(0, 3, -15), Vec3(10, 6, 1), wallTex, 2.5f, 1.5f));
        // 1. Near the entrance
        trashBins.push_back(new TrashBin(Vec3(-8, 0, -35), "models/Modern_Trash_Bin__Low_Poly.obj"));

        // 2. In the left corridor
        trashBins.push_back(new TrashBin(Vec3(-22, 0, -20), "models/Modern_Trash_Bin__Low_Poly.obj"));

        // 3. In the right corridor
        trashBins.push_back(new TrashBin(Vec3(18, 0, -25), "models/Modern_Trash_Bin__Low_Poly.obj"));

        // 4. In the middle pathway
        trashBins.push_back(new TrashBin(Vec3(0, 0, -18), "models/Modern_Trash_Bin__Low_Poly.obj"));

        // 5. Near the key
        
        keys.push_back(new Key(Vec3(-20, 1.2f, -15), 1));

        PatrolEnemy* mazeGuard1 = new PatrolEnemy(Vec3(-20, 0, -28), false);
        mazeGuard1->addWaypoint(Vec3(-20, 0, -28));
        mazeGuard1->addWaypoint(Vec3(-20, 0, -22));
        mazeGuard1->addWaypoint(Vec3(-15, 0, -22));
        mazeGuard1->addWaypoint(Vec3(-15, 0, -28));
        enemies.push_back(mazeGuard1);

        PatrolEnemy* mazeGuard2 = new PatrolEnemy(Vec3(8, 0, -30), false);
        mazeGuard2->addWaypoint(Vec3(8, 0, -30));
        mazeGuard2->addWaypoint(Vec3(15, 0, -30));
        mazeGuard2->addWaypoint(Vec3(15, 0, -20));
        mazeGuard2->addWaypoint(Vec3(8, 0, -20));
        enemies.push_back(mazeGuard2);

        PatrolEnemy* mazeGuard3 = new PatrolEnemy(Vec3(-15, 0, -18), false);
        mazeGuard3->addWaypoint(Vec3(-15, 0, -18));
        mazeGuard3->addWaypoint(Vec3(-5, 0, -18));
        mazeGuard3->addWaypoint(Vec3(-15, 0, -18));
        enemies.push_back(mazeGuard3);

        doors.push_back(new Door(Vec3(0, 3, -10), Vec3(4, 6, 1), 1,false));

        // Room 2: Laser Room
        walls.push_back(new Wall(Vec3(0, 3, 30), Vec3(50, 6, 1), wallTex, 12.5f, 1.5f));
        walls.push_back(new Wall(Vec3(-25, 3, 10), Vec3(1, 6, 40), wallTex, 1.0f, 1.5f));
        walls.push_back(new Wall(Vec3(25, 3, 10), Vec3(1, 6, 40), wallTex, 1.0f, 1.5f));
        walls.push_back(new Wall(Vec3(-12, 2, 10), Vec3(1, 4, 30), wallTex, 0.75f, 1.0f));
        walls.push_back(new Wall(Vec3(12, 2, 10), Vec3(1, 4, 30), wallTex, 0.75f, 1.0f));
        //walls.push_back(new Wall(Vec3(0, 0.75f, 25), Vec3(1.5f, 1.5f, 1.5f), 0.6f, 0.6f, 0.65f));

        modelCollectibles.push_back(new ModelCollectible(Vec3(-8, 1.5f, 5), 10, "models/original_Coin.obj"));
        modelCollectibles.push_back(new ModelCollectible(Vec3(8, 1.5f, 5), 10, "models/original_Coin.obj"));
        modelCollectibles.push_back(new ModelCollectible(Vec3(-6, 1.5f, 15), 15, "models/original_Coin.obj"));
        modelCollectibles.push_back(new ModelCollectible(Vec3(6, 1.5f, 15), 15, "models/original_Coin.obj"));

        // ---------------------------------------------------------
        // SCARAB DISPLAY SETUP (Level 1, Room 2)
        // ---------------------------------------------------------

        // 1. The Pedestal (Black Box)
        // Floor is at -0.5, so a 2.0 height box centered at 0.5 sits perfectly on the floor
        walls.push_back(new Wall(Vec3(0, 0.5f, 23.5), Vec3(1.5f, 2.0f, 1.5f), 0.1f, 0.1f, 0.1f));

        // 2. The Golden Scarab (Model Target)
        // Adjusted height to 2.2f so it hovers nicely just above the pedestal
        modelTarget = new ModelTarget(Vec3(0, 2.2f, 23.5), "models/golden_scarab.obj", true);

        // 3. The Glass Case
        // Sits on top of the pedestal, enclosing the scarab
        displayCases.push_back(new DisplayCase(Vec3(0, 2.25f, 23.5), Vec3(1.3f, 1.5f, 1.3f)));

        // ---------------------------------------------------------

        cameras.push_back(new SecurityCamera(Vec3(-15, 5, 5), 45));
        cameras.push_back(new SecurityCamera(Vec3(15, 5, 10), 135));
        cameras.push_back(new SecurityCamera(Vec3(0, 5, 20), 90));
        cameras.push_back(new SecurityCamera(Vec3(-10, 5, 22), 0));
        cameras.push_back(new SecurityCamera(Vec3(10, 5, 22), 180));

        lasers.push_back(new LaserGrid(Vec3(-10, 1, 8), Vec3(10, 1, 8), 2));
        lasers.push_back(new LaserGrid(Vec3(-10, 1.0f, 15), Vec3(10, 1.0f, 15), 2));
        lasers.push_back(new LaserGrid(Vec3(-10, 2.0f, 22), Vec3(10, 2.0f, 22), 2));
    }
};

// Level 2: Enhanced Vault with Pressure Plate Puzzle Room
class VaultLevel : public Level {
private:
    // Safe path through pressure plates (5x5 grid)
    // Path visualization will be shown on ceiling as glowing tiles
    std::vector<std::pair<int, int>> safePath;

public:
    VaultLevel() {
        playerStartPos = Vec3(0.0f, 0.0f, -28.0f);
        room2StartPos = Vec3(0.0f, 0.0f, 25.0f);  // Checkpoint after puzzle room
        targetCollected = false;
        // Define the safe path (row, col) - creates a winding path
        safePath.push_back({ 0,0 });  // Start
        safePath.push_back({ 1,0 });  // Start

        safePath.push_back({ 2,0 });  // Start
        safePath.push_back({ 2,1 });
        safePath.push_back({ 2,2 });
        safePath.push_back({ 2,3 });
        safePath.push_back({ 2,4 });
        safePath.push_back({ 3,4 });
        safePath.push_back({ 4,4 });  // End // End: bottom right

        buildLevel();
    }

    bool isOnSafePath(int row, int col) {
        for (const auto& pos : safePath) {
            if (pos.first == row && pos.second == col) {
                return true;
            }
        }
        return false;
    }

private:
    void buildLevel() {
        cleanup();

        GLuint wallTex = TextureLoader::getWallTexture();
        GLuint floorTex = TextureLoader::getFloorTexture();
        GLuint boxTex = TextureLoader::getBoxTexture();
        GLuint pressurePlateTex = TextureLoader::getPressurePlateTexture();

        // ===== ROOM 1: PRESSURE PLATE PUZZLE ROOM =====
        float room1Length = 30.0f;
        float room1Width = 16.0f;

        // Room 1 walls
        walls.push_back(new Wall(Vec3(0, 3, -35), Vec3(room1Width, 6, 1), wallTex, 4.0f, 1.5f));  // North

        // CREATE A GAP IN THE SOUTH WALL FOR THE DOOR
        // Create two partial walls with a gap in the middle for the vault door
        float doorWidth = 4.0f;  // Door width
        float wallGap = doorWidth + 1.0f;  // Slightly larger than door for clearance

        // Left side of south wall
        float leftWallWidth = (room1Width - wallGap) / 2.0f;
        float leftWallX = -(room1Width / 2.0f) + leftWallWidth / 2.0f;
        walls.push_back(new Wall(Vec3(leftWallX, 3, -5), Vec3(leftWallWidth, 6, 1), wallTex, leftWallWidth / 4.0f, 1.5f));

        // Right side of south wall
        float rightWallX = (room1Width / 2.0f) - leftWallWidth / 2.0f;
        walls.push_back(new Wall(Vec3(rightWallX, 3, -5), Vec3(leftWallWidth, 6, 1), wallTex, leftWallWidth / 4.0f, 1.5f));

        walls.push_back(new Wall(Vec3(-room1Width / 2, 3, -20), Vec3(1, 6, room1Length), wallTex, 7.5f, 1.5f));  // West
        walls.push_back(new Wall(Vec3(room1Width / 2, 3, -20), Vec3(1, 6, room1Length), wallTex, 7.5f, 1.5f));   // East

        // Floor and ceiling for Room 1
        walls.push_back(new Wall(Vec3(0, -0.5f, -20), Vec3(room1Width, 1, room1Length), floorTex, 4.0f, 7.5f));
        walls.push_back(new Wall(Vec3(0, 6.5f, -20), Vec3(room1Width, 1, room1Length), wallTex, 4.0f, 7.5f));

        // Starting platform (safe area before puzzle)
        walls.push_back(new Wall(Vec3(0, 0.1f, -30), Vec3(5.0f, 0.2f, 3.0f), 0.3f, 0.3f, 0.4f));
        // ---------------------------------------------------------
                // CROWN DISPLAY SETUP (Pedestal + Crown + Glass Case)
                // ---------------------------------------------------------

                // 1. The Pedestal (Black Box)
                // We use the RGB Wall constructor (0.1, 0.1, 0.1) which automatically sets useTexture=false.
                // This ensures it is pure black and doesn't mess up your pressure plate textures.
        walls.push_back(new Wall(Vec3(0, 1.0f, 52), Vec3(2.0f, 2.0f, 2.0f), 0.1f, 0.1f, 0.1f));

        // 2. The Crown (Model Target)
        // Positioned on top of the pedestal (y=2.0). 
        // We set y=2.5 so it bobs up and down above the black box.
        modelTarget = new ModelTarget(Vec3(0, 2.5f, 52), "models/Crown.obj", false);

        // 3. The Glass Case (Transparent Box)
        // Sits on the pedestal, encapsulating the crown.
        displayCases.push_back(new DisplayCase(Vec3(0, 3.25f, 52), Vec3(1.8f, 2.5f, 1.8f)));

        // ---------------------------------------------------------
        // Door between Room 1 and Room 2 - POSITIONED IN THE GAP
        // Position it in the center of the gap we created
        doors.push_back(new Door(Vec3(2, 3, -5), Vec3(6, 6, 1), 2, true));
        ladder = new Ladder(Vec3(0, 3, 59), "models/Ladder.obj");

        // Create 5x5 pressure plate grid
        float plateSize = 1.5f;
        float plateSpacing = 2.0f;
        float gridStartX = -4.0f;
        float gridStartZ = -24.0f;

        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                float x = gridStartX + col * plateSpacing;
                float z = gridStartZ + row * plateSpacing;

                // Create pressure plate at this position
                PressurePlate* plate = new PressurePlate(Vec3(x, 0.05f, z), pressurePlateTex);
                pressurePlates.push_back(plate);
            }
        }

        // Exit platform (safe area after puzzle)
        walls.push_back(new Wall(Vec3(0, 0.1f, -8), Vec3(5.0f, 0.2f, 3.0f), 0.3f, 0.3f, 0.4f));

        // Render path visualization on CEILING as glowing tiles
        for (const auto& pos : safePath) {
            float x = gridStartX + pos.second * plateSpacing;
            float z = gridStartZ + pos.first * plateSpacing;

            // Create glowing green tiles on ceiling showing safe path
            GlowTile* tile = new GlowTile(Vec3(x, 4.5f, z), Vec3(1.4f, 0.1f, 1.4f));
            glowTiles.push_back(tile);
        }

        // Key at the end of puzzle (to unlock door)
        keys.push_back(new Key(Vec3(0, 1.5f, -8), 2));

        // ===== ROOM 2: ORIGINAL VAULT ROOM =====
        float room2Length = 70.0f;
        float room2Width = 50.0f;

        // Room 2 walls - IMPORTANT: Need to create a matching gap in the north wall of Room 2
        walls.push_back(new Wall(Vec3(0, 3, 60), Vec3(room2Width, 6, 1), wallTex, 12.5f, 1.5f));  // South end

        // West wall - but stop before the door gap
        walls.push_back(new Wall(Vec3(-25, 3, 42.5f), Vec3(1, 6, 35), wallTex, 8.75f, 1.5f));  // West wall south part

        // East wall - but stop before the door gap
        walls.push_back(new Wall(Vec3(25, 3, 42.5f), Vec3(1, 6, 35), wallTex, 8.75f, 1.5f));   // East wall south part

        // Floor and ceiling for Room 2
        walls.push_back(new Wall(Vec3(0, -0.5f, 30), Vec3(room2Width, 1, room2Length), floorTex, 12.5f, 17.5f));
        walls.push_back(new Wall(Vec3(0, 6.5f, 30), Vec3(room2Width, 1, room2Length), wallTex, 12.5f, 1.75f));

        // Corridor walls
        walls.push_back(new Wall(Vec3(-12, 2, 30), Vec3(1, 4, room2Length - 20), wallTex, 12.5f, 1.0f));
        walls.push_back(new Wall(Vec3(12, 2, 30), Vec3(1, 4, room2Length - 20), wallTex, 12.5f, 1.0f));

        // Storage crates
        walls.push_back(new Wall(Vec3(-15, 1, 10), Vec3(2, 2, 2), boxTex, 0.25f, 0.2f));
        walls.push_back(new Wall(Vec3(15, 1, 10), Vec3(2, 2, 2), boxTex, 0.25f, 0.2f));
        walls.push_back(new Wall(Vec3(-8, 1, 20), Vec3(2, 2, 2), boxTex, 0.25f, 0.2f));
        walls.push_back(new Wall(Vec3(8, 1, 35), Vec3(2, 2, 2), boxTex, 0.25f, 0.2f));

        // Pits
        pits.push_back(new Pit(Vec3(-5, -1, 15), Vec3(4, 1, 4)));
        pits.push_back(new Pit(Vec3(6, -1, 25), Vec3(5, 1, 5)));
        pits.push_back(new Pit(Vec3(-8, -1, 40), Vec3(4, 1, 4)));
        pits.push_back(new Pit(Vec3(10, -1, 50), Vec3(4, 1, 4)));

        // Collectibles
        modelCollectibles.push_back(new ModelCollectible(Vec3(-15, 1.5f, 20), 10, "models/Diamond.obj"));
        modelCollectibles.push_back(new ModelCollectible(Vec3(12, 1.5f, 30), 10, "models/Diamond.obj"));
        modelCollectibles.push_back(new ModelCollectible(Vec3(-10, 1.5f, 42), 15, "models/Diamond.obj"));
        modelCollectibles.push_back(new ModelCollectible(Vec3(10, 1.5f, 52), 15, "models/Diamond.obj"));

        // Guard dogs
        PatrolEnemy* dog1 = new PatrolEnemy(Vec3(-10, 0, 18), true);
        dog1->addWaypoint(Vec3(-10, 0, 18));
        dog1->addWaypoint(Vec3(-10, 0, 35));
        dog1->addWaypoint(Vec3(-10, 0, 18));
        enemies.push_back(dog1);

        PatrolEnemy* dog2 = new PatrolEnemy(Vec3(10, 0, 38), true);
        dog2->addWaypoint(Vec3(10, 0, 38));
        dog2->addWaypoint(Vec3(10, 0, 48));
        dog2->addWaypoint(Vec3(10, 0, 38));
        enemies.push_back(dog2);

        PatrolEnemy* dog3 = new PatrolEnemy(Vec3(0, 0, 50), true);
        dog3->addWaypoint(Vec3(-8, 0, 50));
        dog3->addWaypoint(Vec3(8, 0, 50));
        dog3->addWaypoint(Vec3(-8, 0, 50));
        enemies.push_back(dog3);

        // Security cameras
        cameras.push_back(new SecurityCamera(Vec3(-15, 5, 25), 45));
        cameras.push_back(new SecurityCamera(Vec3(15, 5, 40), 135));
        cameras.push_back(new SecurityCamera(Vec3(0, 5, 55), 90));

        // Motion sensors
        sensors.push_back(new MotionSensor(Vec3(-8, 2, 12)));
        sensors.push_back(new MotionSensor(Vec3(8, 2, 22)));
        sensors.push_back(new MotionSensor(Vec3(-10, 2, 38)));
        sensors.push_back(new MotionSensor(Vec3(10, 2, 48)));

        // Laser grids
        lasers.push_back(new LaserGrid(Vec3(-11, 1, 27), Vec3(11, 1, 27), 2));
        lasers.push_back(new LaserGrid(Vec3(-11, 2, 42), Vec3(11, 2, 42), 2));
    }
};
#endif // LEVEL_H
