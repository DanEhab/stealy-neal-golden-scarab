/*
 * SoundPlayer.h - Simple sound player for Windows
 * Supports: .wav and .mp3 format
 * Uses: Windows Multimedia API (winmm.lib)
 *
 * TO USE:
 * 1. Add to your linker settings: winmm.lib
 * 2. In Visual Studio: Project Properties -> Linker -> Input -> Additional Dependencies -> Add "winmm.lib"
 */

#ifndef SOUNDPLAYER_H
#define SOUNDPLAYER_H

#include <windows.h>
#include <mmsystem.h>
#include <map>
#include <string>
#include <cstdio>

#pragma comment(lib, "winmm.lib")

class SoundPlayer {
private:
    std::map<std::string, std::string> soundFiles;
    bool backgroundMusicPlaying = false;
    int soundCounter = 0;
    
public:
    SoundPlayer() {}
    
    ~SoundPlayer() {
        stopBackgroundMusic();
    }

    /*
     * Register a sound file with a name
     * Example: registerSound("explosion", "sounds/explosion.wav");
     */
    void registerSound(const char* name, const char* filepath) {
        if (name && filepath) {
            soundFiles[name] = filepath;
        }
    }

    /*
     * Play a sound by name
     * async = true: Play in background (non-blocking)
     * async = false: Wait for sound to finish (blocking)
     */
    void play(const char* name, bool async = true) {
        if (!name) return;

        auto it = soundFiles.find(name);
        if (it == soundFiles.end()) {
            printf("WARNING: Sound not found: %s\n", name);
            return;
        }

        const std::string& filename = it->second;
        playFile(filename.c_str(), async);
    }

    /*
     * Play a sound file directly without registration (WAV files)
     */
    void playFile(const char* filepath, bool async = true) {
        // Create unique alias for each sound effect
        char alias[64];
        sprintf(alias, "sfx%d", soundCounter++);

        char cmd[512];
        sprintf(cmd, "open \"%s\" type waveaudio alias %s", filepath, alias);
        mciSendStringA(cmd, NULL, 0, NULL);

        sprintf(cmd, "play %s", alias);
        mciSendStringA(cmd, NULL, 0, NULL);
    }
    
    /*
     * Play background music (supports MP3 and WAV)
     * Automatically loops
     */
    void playBackgroundMusic(const char* filepath = "The Pink Panther Theme Music.mp3") {
        if (!filepath) return;
        
        // Stop any existing music
        stopBackgroundMusic();
        
        // Check if file exists
        DWORD fileAttrib = GetFileAttributesA(filepath);
        if (fileAttrib == INVALID_FILE_ATTRIBUTES) {
            printf("ERROR: Music file not found: %s\n", filepath);
            return;
        }
        
        char cmd[512];
        
        // Open the music file (works for both MP3 and WAV)
        sprintf(cmd, "open \"%s\" type mpegvideo alias bgmusic", filepath);
        MCIERROR err = mciSendStringA(cmd, NULL, 0, NULL);
        
        if (err != 0) {
            // Try as waveaudio if mpegvideo failed
            sprintf(cmd, "open \"%s\" type waveaudio alias bgmusic", filepath);
            err = mciSendStringA(cmd, NULL, 0, NULL);
        }
        
        if (err != 0) {
            printf("ERROR: Failed to open music file: %s\n", filepath);
            return;
        }
        
        // Set to repeat continuously
        mciSendStringA("set bgmusic time format milliseconds", NULL, 0, NULL);
        
        // Play with repeat flag
        sprintf(cmd, "play bgmusic repeat");
        mciSendStringA(cmd, NULL, 0, NULL);
        
        backgroundMusicPlaying = true;
        printf("Background music started: %s\n", filepath);
    }
    
    /*
     * Stop background music
     */
    void stopBackgroundMusic() {
        if (backgroundMusicPlaying) {
            mciSendStringA("stop bgmusic", NULL, 0, NULL);
            mciSendStringA("close bgmusic", NULL, 0, NULL);
            backgroundMusicPlaying = false;
        }
    }
    
    /*
     * Check if background music is playing
     */
    bool isBackgroundMusicPlaying() const {
        return backgroundMusicPlaying;
    }
    
    /*
     * Stop all sounds
     */
    void stopAll() {
        stopBackgroundMusic();
        PlaySoundA(NULL, NULL, 0);
    }

    /*
     * Play looping background music (legacy method - use playBackgroundMusic instead)
     */
    void playLoop(const char* filepath) {
        if (!filepath) return;

        // Check if file exists
        DWORD fileAttrib = GetFileAttributesA(filepath);
        if (fileAttrib == INVALID_FILE_ATTRIBUTES) {
            printf("ERROR: Sound file not found: %s\n", filepath);
            return;
        }

        PlaySoundA(filepath, NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
    }

    /*
     * Check if a sound is registered
     */
    bool hasSound(const char* name) const {
        if (!name) return false;
        return soundFiles.find(name) != soundFiles.end();
    }

    /*
     * Unregister a sound
     */
    void unregisterSound(const char* name) {
        if (name) {
            soundFiles.erase(name);
        }
    }

    /*
     * Clear all registered sounds
     */
    void clearAllSounds() {
        soundFiles.clear();
        stopAll();
    }

    /*
     * Play a system sound
     */
    static void playSystemSound(const char* soundAlias) {
        if (soundAlias) {
            PlaySoundA(soundAlias, NULL, SND_ALIAS | SND_ASYNC);
        }
    }
};

#endif // SOUNDPLAYER_H