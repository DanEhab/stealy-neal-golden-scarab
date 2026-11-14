// TextureLoader.h
#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include <windows.h>
#include <glut.h>
#include <string>
#include <fstream>

class TextureLoader {
private:
    struct BitmapHeader {
        unsigned short type;
        unsigned int size;
        unsigned short reserved1;
        unsigned short reserved2;
        unsigned int offset;
        unsigned int headerSize;
        int width;
        int height;
        unsigned short planes;
        unsigned short bitsPerPixel;
        unsigned int compression;
        unsigned int imageSize;
        int xPixelsPerMeter;
        int yPixelsPerMeter;
        unsigned int colorsUsed;
        unsigned int importantColors;
    };

    static GLuint loadBMP(const char* filename) {
        FILE* file = fopen(filename, "rb");
        if (!file) {
            printf("ERROR: Could not open texture file: %s\n", filename);
            return 0;
        }

        BitmapHeader header;
        fread(&header.type, 2, 1, file);
        fread(&header.size, 4, 1, file);
        fread(&header.reserved1, 2, 1, file);
        fread(&header.reserved2, 2, 1, file);
        fread(&header.offset, 4, 1, file);
        fread(&header.headerSize, 4, 1, file);
        fread(&header.width, 4, 1, file);
        fread(&header.height, 4, 1, file);
        fread(&header.planes, 2, 1, file);
        fread(&header.bitsPerPixel, 2, 1, file);
        fread(&header.compression, 4, 1, file);
        fread(&header.imageSize, 4, 1, file);
        fread(&header.xPixelsPerMeter, 4, 1, file);
        fread(&header.yPixelsPerMeter, 4, 1, file);
        fread(&header.colorsUsed, 4, 1, file);
        fread(&header.importantColors, 4, 1, file);

        // Check if it's a BMP file
        if (header.type != 0x4D42) { // "BM"
            printf("ERROR: Not a valid BMP file: %s\n", filename);
            fclose(file);
            return 0;
        }

        // Only support 24-bit RGB
        if (header.bitsPerPixel != 24) {
            printf("ERROR: Only 24-bit BMP files supported: %s\n", filename);
            fclose(file);
            return 0;
        }

        // Seek to pixel data
        fseek(file, header.offset, SEEK_SET);

        // Calculate padding
        int width = header.width;
        int height = header.height;
        int padding = (4 - (width * 3) % 4) % 4;

        // Allocate memory for pixel data
        unsigned char* data = new unsigned char[width * height * 3];

        // Read pixel data (BMP stores rows bottom-to-top)
        for (int y = height - 1; y >= 0; y--) {
            for (int x = 0; x < width; x++) {
                // Read BGR (BMP format) and convert to RGB
                fread(&data[(y * width + x) * 3 + 2], 1, 1, file); // Blue
                fread(&data[(y * width + x) * 3 + 1], 1, 1, file); // Green
                fread(&data[(y * width + x) * 3 + 0], 1, 1, file); // Red
            }
            // Skip padding
            fseek(file, padding, SEEK_CUR);
        }

        fclose(file);

        // Create OpenGL texture
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        delete[] data;

        printf("Loaded texture: %s (%dx%d)\n", filename, width, height);
        return textureID;
    }

    static GLuint loadTextureFromFile(const char* filename) {
        // Try to load as BMP
        GLuint texture = loadBMP(filename);

        if (texture == 0) {
            // Fallback to creating a checkerboard pattern
            printf("Creating fallback texture for: %s\n", filename);
            texture = createCheckerboardTexture();
        }

        return texture;
    }

    static GLuint createCheckerboardTexture() {
        const int SIZE = 64;
        unsigned char data[SIZE][SIZE][3];

        for (int y = 0; y < SIZE; y++) {
            for (int x = 0; x < SIZE; x++) {
                unsigned char color = ((x / 8) % 2) == ((y / 8) % 2) ? 255 : 128;
                data[y][x][0] = color;  // R
                data[y][x][1] = color;  // G
                data[y][x][2] = color;  // B
            }
        }

        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SIZE, SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }

public:
    static GLuint wallTexture;
    static GLuint floorTexture;
    static GLuint boxTexture;
    static GLuint PressurePlateTexture;
    static void loadTextures() {
        PressurePlateTexture = loadTextureFromFile("models/metall.bmp");
        wallTexture = loadTextureFromFile("models/wall.bmp");
        floorTexture = loadTextureFromFile("models/floor.bmp");
        
		boxTexture = loadTextureFromFile("models/box.bmp");
        
        if (wallTexture == 0 || floorTexture == 0 || PressurePlateTexture==0 || boxTexture==0) {
            printf("WARNING: Could not load texture files. Using checkerboard patterns.\n");
        }
    }

    static GLuint getWallTexture() { return wallTexture; }
    static GLuint getFloorTexture() { return floorTexture; }
    static GLuint getBoxTexture() { return boxTexture; }

    static GLuint getPressurePlateTexture() { return PressurePlateTexture; }
};

// Initialize static members
GLuint TextureLoader::wallTexture = 0;
GLuint TextureLoader::floorTexture = 0;
GLuint TextureLoader::boxTexture  = 0;
GLuint TextureLoader::PressurePlateTexture = 0;
#endif // TEXTURELOADER_H