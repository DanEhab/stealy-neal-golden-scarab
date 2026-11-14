/*
 * Renderer.h - Enhanced rendering utilities for HUD and screens
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <windows.h>
#include <glut.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Renderer {
private:
    // Animation variables for music button
    float musicButtonHoverTime = 0.0f;
    bool musicButtonHovered = false;
    float musicButtonPulse = 0.0f;

public:
    Renderer() {}

    void renderText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1280, 0, 720);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glRasterPos2f(x, y);

        for (const char* c = text; *c != '\0'; c++) {
            glutBitmapCharacter(font, *c);
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    /*
     * Render a beautiful music toggle button - REDESIGNED
     * Position: Top-right corner
     * Features: Rounded square, glass-morphism effect, smooth animations
     */
    void renderMusicButton(bool musicOn) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1280, 0, 720);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Button position (top-right corner)
        float btnX = 1190;
        float btnY = 650;
        float btnSize = 55;
        float cornerRadius = 15;  // For rounded corners

        // Update pulse animation
        musicButtonPulse += 0.025f;
        if (musicButtonPulse > 2.0f * M_PI) {
            musicButtonPulse -= 2.0f * M_PI;
        }

        // Smooth scale effect
        float scale = 1.0f + 0.04f * sin(musicButtonPulse);
        float scaledSize = btnSize * scale;

        // === OUTER GLOW - REMOVED FOR MUSIC ON ===
        // No glow effect when music is on (per user request)

        // === MAIN BUTTON BACKGROUND (Rounded Rectangle) ===
        // Draw using approximation with triangle fan
        glBegin(GL_TRIANGLE_FAN);
        
        // Center point
        glVertex2f(btnX, btnY);
        
        // Create rounded rectangle using circle segments at corners
        int segments = 50;
        float halfSize = scaledSize / 2.0f;
        
        for (int i = 0; i <= segments; i++) {
            float t = (float)i / segments;
            float angle = t * 2.0f * M_PI;
            
            // Calculate position on rounded square
            float x, y;
            
            if (angle < M_PI / 2) {
                // Top-right corner
                x = btnX + halfSize - cornerRadius + cos(angle) * cornerRadius;
                y = btnY + halfSize - cornerRadius + sin(angle) * cornerRadius;
            } else if (angle < M_PI) {
                // Top-left corner
                x = btnX - halfSize + cornerRadius + cos(angle) * cornerRadius;
                y = btnY + halfSize - cornerRadius + sin(angle) * cornerRadius;
            } else if (angle < 3 * M_PI / 2) {
                // Bottom-left corner
                x = btnX - halfSize + cornerRadius + cos(angle) * cornerRadius;
                y = btnY - halfSize + cornerRadius + sin(angle) * cornerRadius;
            } else {
                // Bottom-right corner
                x = btnX + halfSize - cornerRadius + cos(angle) * cornerRadius;
                y = btnY - halfSize + cornerRadius + sin(angle) * cornerRadius;
            }
            
            // Apply gradient color (darker at edges)
            float distFromCenter = sqrt((x - btnX) * (x - btnX) + (y - btnY) * (y - btnY)) / halfSize;
            
            if (musicOn) {
                // Blue gradient when ON
                float r = 0.05f + distFromCenter * 0.05f;
                float g = 0.4f - distFromCenter * 0.15f;
                float b = 0.75f - distFromCenter * 0.2f;
                glColor4f(r, g, b, 0.95f);
            } else {
                // Red-gray gradient when MUTED
                float r = 0.35f - distFromCenter * 0.1f;
                float g = 0.15f - distFromCenter * 0.08f;
                float b = 0.15f - distFromCenter * 0.08f;
                glColor4f(r, g, b, 0.92f);
            }
            
            glVertex2f(x, y);
        }
        glEnd();

        // === GLOSSY OVERLAY (top half brighter for glass effect) ===
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(btnX, btnY);
        
        for (int i = 0; i <= segments / 2; i++) {
            float t = (float)i / (segments / 2);
            float angle = t * M_PI;  // Only top half
            
            float x, y;
            if (angle < M_PI / 2) {
                x = btnX + halfSize - cornerRadius + cos(angle) * cornerRadius;
                y = btnY + halfSize - cornerRadius + sin(angle) * cornerRadius;
            } else {
                x = btnX - halfSize + cornerRadius + cos(angle) * cornerRadius;
                y = btnY + halfSize - cornerRadius + sin(angle) * cornerRadius;
            }
            
            glColor4f(1.0f, 1.0f, 1.0f, 0.12f);  // Subtle white overlay
            glVertex2f(x, y);
        }
        glEnd();

        // === BORDER (glowing edge) ===
        glLineWidth(2.5f);
        
        if (musicOn) {
            glColor4f(0.2f, 0.9f, 1.0f, 0.9f);  // Bright cyan border
        } else {
            glColor4f(0.8f, 0.4f, 0.4f, 0.85f);  // Muted red border
        }

        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            float t = (float)i / segments;
            float angle = t * 2.0f * M_PI;
            
            float x, y;
            if (angle < M_PI / 2) {
                x = btnX + halfSize - cornerRadius + cos(angle) * cornerRadius;
                y = btnY + halfSize - cornerRadius + sin(angle) * cornerRadius;
            } else if (angle < M_PI) {
                x = btnX - halfSize + cornerRadius + cos(angle) * cornerRadius;
                y = btnY + halfSize - cornerRadius + sin(angle) * cornerRadius;
            } else if (angle < 3 * M_PI / 2) {
                x = btnX - halfSize + cornerRadius + cos(angle) * cornerRadius;
                y = btnY - halfSize + cornerRadius + sin(angle) * cornerRadius;
            } else {
                x = btnX + halfSize - cornerRadius + cos(angle) * cornerRadius;
                y = btnY - halfSize + cornerRadius + sin(angle) * cornerRadius;
            }
            
            glVertex2f(x, y);
        }
        glEnd();
        glLineWidth(1.0f);

        // === ICON (improved design) ===
        glLineWidth(3.0f);

        if (musicOn) {
            // MUSIC NOTE ICON (cleaner design)
            glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
            
            float noteX = btnX - 10;
            float noteY = btnY - 14;
            
            // Note stem (vertical line)
            glBegin(GL_LINES);
            glVertex2f(noteX + 10, noteY);
            glVertex2f(noteX + 10, noteY + 28);
            glEnd();

            // Note head (filled ellipse)
            glBegin(GL_TRIANGLE_FAN);
            glVertex2f(noteX, noteY);
            for (int i = 0; i <= 20; i++) {
                float angle = (2.0f * M_PI * i) / 20;
                float x = noteX + cos(angle) * 7;
                float y = noteY + sin(angle) * 5;
                glVertex2f(x, y);
            }
            glEnd();

            // Eighth note flag (smooth curve)
            glBegin(GL_LINE_STRIP);
            glVertex2f(noteX + 10, noteY + 28);
            glVertex2f(noteX + 18, noteY + 25);
            glVertex2f(noteX + 16, noteY + 20);
            glVertex2f(noteX + 20, noteY + 17);
            glEnd();
            
            // Add musical staff lines (decorative)
            glLineWidth(1.5f);
            glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
            for (int i = 0; i < 3; i++) {
                float lineY = noteY - 5 + i * 4;
                glBegin(GL_LINES);
                glVertex2f(noteX - 12, lineY);
                glVertex2f(noteX + 24, lineY);
                glEnd();
            }
            
        } else {
            // MUTED ICON (speaker with slash) - FIXED POSITIONING
            glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
            
            // Centered speaker icon
            float iconX = btnX - 16;
            float iconY = btnY - 10;

            // Speaker body (rectangular base)
            glBegin(GL_QUADS);
            glVertex2f(iconX, iconY + 6);
            glVertex2f(iconX, iconY + 14);
            glVertex2f(iconX + 6, iconY + 14);
            glVertex2f(iconX + 6, iconY + 6);
            glEnd();
            
            // Speaker cone (triangle pointing right)
            glBegin(GL_TRIANGLES);
            glVertex2f(iconX + 6, iconY + 4);
            glVertex2f(iconX + 6, iconY + 16);
            glVertex2f(iconX + 14, iconY + 10);
            glEnd();

            // FIXED: Extended X slash to complete the mute icon
            glLineWidth(4.5f);
            glColor4f(1.0f, 0.15f, 0.15f, 0.95f);
            glBegin(GL_LINES);
            // First diagonal slash from bottom-left to top-right
            glVertex2f(iconX - 4, iconY);
            glVertex2f(iconX + 24, iconY + 20);
            // Second diagonal slash from bottom-right to top-left
            glVertex2f(iconX + 24, iconY);
            glVertex2f(iconX - 4, iconY + 20);
            glEnd();
            
            // Outer glow for slash (lighter red)
            glLineWidth(2.5f);
            glColor4f(1.0f, 0.4f, 0.4f, 0.7f);
            glBegin(GL_LINES);
            // First diagonal glow
            glVertex2f(iconX - 4, iconY);
            glVertex2f(iconX + 24, iconY + 20);
            // Second diagonal glow
            glVertex2f(iconX + 24, iconY);
            glVertex2f(iconX - 4, iconY + 20);
            glEnd();
        }
        glLineWidth(1.0f);

        // === TEXT LABELS (FIXED CENTERING) ===
        glColor4f(1.0f, 1.0f, 1.0f, 0.95f);
        
        // Calculate text width to center it properly
        if (musicOn) {
            // "Music ON" - centered at btnX - 26
            renderText(btnX - 26, btnY - 42, "Music ON", GLUT_BITMAP_HELVETICA_12);
        } else {
            // "MUTED" - centered at btnX - 20 (shorter text, adjust offset)
            renderText(btnX - 20, btnY - 42, "MUTED", GLUT_BITMAP_HELVETICA_12);
        }

        // Instruction with subtle background
        glColor4f(0.0f, 0.0f, 0.0f, 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(btnX - 30, btnY - 60);
        glVertex2f(btnX + 30, btnY - 60);
        glVertex2f(btnX + 30, btnY - 48);
        glVertex2f(btnX - 30, btnY - 48);
        glEnd();
        
        glColor4f(0.9f, 0.9f, 0.9f, 0.8f);
        renderText(btnX - 25, btnY - 55, "(Press M)", GLUT_BITMAP_HELVETICA_10);

        glDisable(GL_BLEND);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderHUD(int score, int level, bool flashlightOn = false, bool musicOn = true) {
        glDisable(GL_LIGHTING);

        // Score display
        std::stringstream ss;
        ss << "Score: " << score;
        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(20, 680, ss.str().c_str(), GLUT_BITMAP_HELVETICA_18);

        // Level display
        ss.str("");
        ss << "Level: " << level;
        renderText(20, 650, ss.str().c_str(), GLUT_BITMAP_HELVETICA_18);

        // Flashlight indicator (Level 2)
        if (level == 2) {
            ss.str("");
            ss << "Flashlight: " << (flashlightOn ? "ON (F)" : "OFF (F)");
            if (flashlightOn) {
                glColor3f(1.0f, 1.0f, 0.0f);
            }
            else {
                glColor3f(0.5f, 0.5f, 0.5f);
            }
            renderText(20, 620, ss.str().c_str(), GLUT_BITMAP_HELVETICA_18);
        }

        // Controls hint
        glColor3f(0.8f, 0.8f, 0.8f);
        if (level == 1) {
            renderText(20, 30, "WASD/Arrows: Move | Space: Jump | C: Toggle Camera | M: Music | Mouse: Look",
                GLUT_BITMAP_HELVETICA_12);
            renderText(20, 15, "Find the KEY to unlock doors!",
                GLUT_BITMAP_HELVETICA_12);
        }
        else {
            renderText(20, 30, "WASD/Arrows: Move | Space: Jump | C: Camera | F: Flashlight | M: Music | Mouse: Look",
                GLUT_BITMAP_HELVETICA_12);
            renderText(20, 15, "Avoid pits and pressure plates! Find the KEY to escape!",
                GLUT_BITMAP_HELVETICA_12);
        }

        // Render the beautiful music button
        renderMusicButton(musicOn);

        glEnable(GL_LIGHTING);
    }

    void renderGameOver() {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // Dark overlay
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1280, 0, 720);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Semi-transparent red overlay
        glColor4f(0.5f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();

        glDisable(GL_BLEND);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        // Game Over text
        glColor3f(1.0f, 0.0f, 0.0f);
        renderText(480, 400, "GAME OVER", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(420, 340, "You were detected!", GLUT_BITMAP_HELVETICA_18);
        renderText(450, 300, "Press R to restart level", GLUT_BITMAP_HELVETICA_18);
        renderText(480, 260, "Press ESC to exit", GLUT_BITMAP_HELVETICA_18);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderGameWin(int finalScore) {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        // Dark overlay with green tint
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1280, 0, 720);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Semi-transparent green overlay
        glColor4f(0.0f, 0.3f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(1280, 0);
        glVertex2f(1280, 720);
        glVertex2f(0, 720);
        glEnd();

        glDisable(GL_BLEND);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        // Victory text
        glColor3f(0.0f, 1.0f, 0.0f);
        renderText(450, 450, "MISSION COMPLETE!", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(1.0f, 1.0f, 1.0f);
        renderText(380, 380, "You successfully stole the artifacts!", GLUT_BITMAP_HELVETICA_18);

        std::stringstream ss;
        ss << "Final Score: " << finalScore;
        renderText(500, 340, ss.str().c_str(), GLUT_BITMAP_HELVETICA_18);

        renderText(460, 280, "Press R to play again", GLUT_BITMAP_HELVETICA_18);
        renderText(480, 240, "Press ESC to exit", GLUT_BITMAP_HELVETICA_18);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

    void renderCrosshair() {
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, 1280, 0, 720);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);

        float centerX = 640;
        float centerY = 360;
        float size = 10;

        glBegin(GL_LINES);
        glVertex2f(centerX - size, centerY);
        glVertex2f(centerX + size, centerY);
        glVertex2f(centerX, centerY - size);
        glVertex2f(centerX, centerY + size);
        glEnd();

        glLineWidth(1.0f);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
};

#endif // RENDERER_H