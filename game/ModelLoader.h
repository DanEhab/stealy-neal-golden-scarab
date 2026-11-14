/*
 * ModelLoader.h - Enhanced OBJ model loader with material and scaling support
 * Supports: .obj format with vertices, normals, texture coordinates, and materials
 */

#ifndef MODELLOADER_H
#define MODELLOADER_H

#include <windows.h>
#include <glut.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "Vec3.h"
#include <algorithm>

 // Material structure
struct Material {
    std::string name;
    float ambient[3];
    float diffuse[3];
    float specular[3];
    float shininess;

    Material() : shininess(32.0f) {
        ambient[0] = ambient[1] = ambient[2] = 0.2f;
        diffuse[0] = diffuse[1] = diffuse[2] = 0.8f;
        specular[0] = specular[1] = specular[2] = 0.0f;
    }
};

struct Face {
    int v[3];  // Vertex indices
    int vt[3]; // Texture coordinate indices
    int vn[3]; // Normal indices
    std::string materialName; // Material for this face
};

class Model {
private:
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec3> texCoords; // Only using x,y (u,v)
    std::vector<Face> faces;
    std::map<std::string, Material> materials;

    GLuint displayList;
    bool compiled;
    bool useMaterials;

    // Scaling properties
    Vec3 scale;
    Vec3 originalMin;
    Vec3 originalMax;
    Vec3 scaledMin;
    Vec3 scaledMax;

public:
    Model() : displayList(0), compiled(false), useMaterials(false),
        scale(1.0f, 1.0f, 1.0f) {}

    ~Model() {
        if (displayList != 0) {
            glDeleteLists(displayList, 1);
        }
    }

    /*
     * Load material library (.mtl file)
     */
    bool loadMaterial(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("WARNING: Could not open material file: %s\n", filename);
            return false;
        }

        Material currentMaterial;
        std::string line;

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "newmtl") {
                // Save previous material if any
                if (!currentMaterial.name.empty()) {
                    materials[currentMaterial.name] = currentMaterial;
                }
                // Start new material
                currentMaterial = Material();
                iss >> currentMaterial.name;
            }
            else if (type == "Ka") { // Ambient color
                iss >> currentMaterial.ambient[0]
                    >> currentMaterial.ambient[1]
                    >> currentMaterial.ambient[2];
            }
            else if (type == "Kd") { // Diffuse color
                iss >> currentMaterial.diffuse[0]
                    >> currentMaterial.diffuse[1]
                    >> currentMaterial.diffuse[2];
            }
            else if (type == "Ks") { // Specular color
                iss >> currentMaterial.specular[0]
                    >> currentMaterial.specular[1]
                    >> currentMaterial.specular[2];
            }
            else if (type == "Ns") { // Shininess
                iss >> currentMaterial.shininess;
            }
        }

        // Save last material
        if (!currentMaterial.name.empty()) {
            materials[currentMaterial.name] = currentMaterial;
        }

        file.close();
        useMaterials = !materials.empty();
        printf("Loaded %d materials from %s\n", (int)materials.size(), filename);
        return true;
    }

    /*
     * Calculate model bounds
     */
    void calculateBounds() {
        if (vertices.empty()) {
            originalMin = originalMax = Vec3(0, 0, 0);
            scaledMin = scaledMax = Vec3(0, 0, 0);
            return;
        }

        originalMin = originalMax = vertices[0];
        for (const Vec3& v : vertices) {
            if (v.x < originalMin.x) originalMin.x = v.x;
            if (v.y < originalMin.y) originalMin.y = v.y;
            if (v.z < originalMin.z) originalMin.z = v.z;
            if (v.x > originalMax.x) originalMax.x = v.x;
            if (v.y > originalMax.y) originalMax.y = v.y;
            if (v.z > originalMax.z) originalMax.z = v.z;
        }

        // Calculate scaled bounds
        updateScaledBounds();
    }

    /*
     * Update scaled bounds based on current scale
     */
    void updateScaledBounds() {
        scaledMin = originalMin * scale;  // Uses the new Vec3::operator*
        scaledMax = originalMax * scale;  // Uses the new Vec3::operator*
    }

    /*
     * Load .obj file with material support
     */
    bool load(const char* filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            printf("ERROR: Could not open model file: %s\n", filename);
            return false;
        }

        vertices.clear();
        normals.clear();
        texCoords.clear();
        faces.clear();
        materials.clear();

        std::string line;
        std::string currentMaterial = "default";

        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream iss(line);
            std::string type;
            iss >> type;

            if (type == "v") {
                // Vertex
                float x, y, z;
                iss >> x >> y >> z;
                vertices.push_back(Vec3(x, y, z));
            }
            else if (type == "vt") {
                // Texture coordinate
                float u, v;
                iss >> u >> v;
                texCoords.push_back(Vec3(u, v, 0));
            }
            else if (type == "vn") {
                // Normal
                float x, y, z;
                iss >> x >> y >> z;
                normals.push_back(Vec3(x, y, z));
            }
            else if (type == "f") {
                // Face - supports formats: f v1 v2 v3 or f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
                Face face;
                face.materialName = currentMaterial;

                for (int i = 0; i < 3; i++) {
                    std::string vertex;
                    iss >> vertex;

                    // Parse v/vt/vn or v//vn or just v
                    std::replace(vertex.begin(), vertex.end(), '/', ' ');
                    std::istringstream viss(vertex);

                    viss >> face.v[i];
                    face.v[i]--; // OBJ indices start at 1

                    if (viss.peek() != EOF) {
                        viss >> face.vt[i];
                        face.vt[i]--;
                    }
                    else {
                        face.vt[i] = -1;
                    }

                    if (viss.peek() != EOF) {
                        viss >> face.vn[i];
                        face.vn[i]--;
                    }
                    else {
                        face.vn[i] = -1;
                    }
                }
                faces.push_back(face);
            }
            else if (type == "mtllib") {
                // Load material library
                std::string mtlFile;
                iss >> mtlFile;

                // Try to load from same directory as OBJ file
                std::string objPath(filename);
                size_t lastSlash = objPath.find_last_of("/\\");
                if (lastSlash != std::string::npos) {
                    mtlFile = objPath.substr(0, lastSlash + 1) + mtlFile;
                }
                loadMaterial(mtlFile.c_str());
            }
            else if (type == "usemtl") {
                // Use material for following faces
                iss >> currentMaterial;
            }
        }

        file.close();

        // Create default material if none loaded
        if (materials.empty()) {
            Material defaultMat;
            defaultMat.name = "default";
            materials["default"] = defaultMat;
        }

        // Calculate bounds before scaling
        calculateBounds();

        printf("Model loaded: %s (%d vertices, %d faces, %d materials)\n",
            filename, (int)vertices.size(), (int)faces.size(), (int)materials.size());
        printf("Original bounds: (%.2f,%.2f,%.2f) to (%.2f,%.2f,%.2f)\n",
            originalMin.x, originalMin.y, originalMin.z,
            originalMax.x, originalMax.y, originalMax.z);

        compile();
        return true;
    }

    /*
     * Compile model into display list with materials AND scaling
     */
     /*
  * Compile model into display list with materials AND scaling
  */
    void compile() {
        if (compiled) return;

        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE);

        // Apply scaling transformation
        glScalef(scale.x, scale.y, scale.z);

        std::string currentMaterial = "";
        bool materialSet = false;

        glBegin(GL_TRIANGLES);
        for (const Face& face : faces) {
            // Apply material if changed
            if (face.materialName != currentMaterial && useMaterials) {
                currentMaterial = face.materialName;
                auto it = materials.find(currentMaterial);
                if (it != materials.end()) {
                    const Material& mat = it->second;

                    // Apply material properties
                    GLfloat ambient[] = { mat.ambient[0], mat.ambient[1], mat.ambient[2], 1.0f };
                    GLfloat diffuse[] = { mat.diffuse[0], mat.diffuse[1], mat.diffuse[2], 1.0f };
                    GLfloat specular[] = { mat.specular[0], mat.specular[1], mat.specular[2], 1.0f };

                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
                    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
                    materialSet = true;
                }
            }

            // If no materials in file, set a default material
            if (!materialSet && !useMaterials) {
                GLfloat defaultAmbient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
                GLfloat defaultDiffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };
                GLfloat defaultSpecular[] = { 0.0f, 0.0f, 0.0f, 1.0f };

                glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, defaultAmbient);
                glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, defaultDiffuse);
                glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, defaultSpecular);
                glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);
                materialSet = true;
            }

            for (int i = 0; i < 3; i++) {
                // Normal - DO NOT SCALE NORMALS
                if (face.vn[i] >= 0 && face.vn[i] < (int)normals.size()) {
                    Vec3 n = normals[face.vn[i]];
                    // Just normalize the original normal
                    Vec3 normal = n;
                    normal.normalize();
                    glNormal3f(normal.x, normal.y, normal.z);
                }

                // Texture coordinate
                if (face.vt[i] >= 0 && face.vt[i] < (int)texCoords.size()) {
                    Vec3 t = texCoords[face.vt[i]];
                    glTexCoord2f(t.x, t.y);
                }

                // Vertex
                if (face.v[i] >= 0 && face.v[i] < (int)vertices.size()) {
                    Vec3 v = vertices[face.v[i]];
                    glVertex3f(v.x, v.y, v.z);
                }
            }
        }
        glEnd();

        glEndList();
        compiled = true;
    }

    /*
     * Render the model
     */
    void render() {
        if (compiled) {
            glCallList(displayList);
        }
    }

    /*
     * Set scale for the model (uniform scaling)
     */
    void setScale(float uniformScale) {
        setScale(uniformScale, uniformScale, uniformScale);
    }

    /*
     * Set scale for the model (non-uniform scaling)
     */
    void setScale(float x, float y, float z) {
        scale = Vec3(x, y, z);
        updateScaledBounds();

        // Recompile with new scale
        if (compiled) {
            glDeleteLists(displayList, 1);
            compiled = false;
            compile();
        }

        printf("Model scaled to: (%.2f, %.2f, %.2f)\n", scale.x, scale.y, scale.z);
        printf("Scaled bounds: (%.2f,%.2f,%.2f) to (%.2f,%.2f,%.2f)\n",
            scaledMin.x, scaledMin.y, scaledMin.z,
            scaledMax.x, scaledMax.y, scaledMax.z);
    }

    void setScale(const Vec3& newScale) {
        setScale(newScale.x, newScale.y, newScale.z);
    }

    /*
     * Get current scale
     */
    Vec3 getScale() const { return scale; }

    /*
     * Get original model bounds (before scaling)
     */
    void getOriginalBounds(Vec3& min, Vec3& max) const {
        min = originalMin;
        max = originalMax;
    }

    /*
     * Get scaled model bounds (after scaling)
     */
    void getScaledBounds(Vec3& min, Vec3& max) const {
        min = scaledMin;
        max = scaledMax;
    }

    /*
     * Auto-scale model to fit target dimensions
     */
    void autoScaleToFit(float targetWidth, float targetHeight, float targetDepth) {
        if (vertices.empty()) return;

        // Calculate current dimensions
        float currentWidth = originalMax.x - originalMin.x;
        float currentHeight = originalMax.y - originalMin.y;
        float currentDepth = originalMax.z - originalMin.z;

        if (currentWidth > 0.0001f && currentHeight > 0.0001f && currentDepth > 0.0001f) {
            float scaleX = targetWidth / currentWidth;
            float scaleY = targetHeight / currentHeight;
            float scaleZ = targetDepth / currentDepth;

            // Use uniform scaling based on the most limiting dimension
            float uniformScale = ((((((scaleX) < (scaleY)) ? (scaleX) : (scaleY))) < (scaleZ)) ? ((((scaleX) < (scaleY)) ? (scaleX) : (scaleY))) : (scaleZ));

            printf("Auto-scaling model: %.2f x %.2f x %.2f -> target: %.2f x %.2f x %.2f\n",
                currentWidth, currentHeight, currentDepth,
                targetWidth, targetHeight, targetDepth);
            printf("Calculated scale factor: %.4f\n", uniformScale);

            setScale(uniformScale);
        }
    }

    /*
     * Auto-scale model to target height (useful for characters)
     */
    void autoScaleToHeight(float targetHeight) {
        if (vertices.empty()) return;

        float currentHeight = originalMax.y - originalMin.y;
        if (currentHeight > 0.0001f) {
            float scaleFactor = targetHeight / currentHeight;

            printf("Auto-scaling model height: %.2f -> %.2f (scale: %.4f)\n",
                currentHeight, targetHeight, scaleFactor);

            setScale(scaleFactor);
        }
    }

    /*
     * Check if model has materials
     */
    bool hasMaterials() const {
        return useMaterials && !materials.empty();
    }
};

#endif // MODELLOADER_H