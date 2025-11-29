#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include "utils.h"

// Types de sons disponibles
enum SoundType {
    SOUND_NONE = 0,
    SOUND_BIP = 1,
    SOUND_SUCCESS = 2,
    SOUND_ERROR = 3,
    SOUND_STARTUP = 4,
    SOUND_BUTTON = 5,
    SOUND_CRITICAL_ERROR = 6
};

class SoundManager {
public:
    SoundManager(int pin);
    void init();
    void play(SoundType type);
    void update(); // À appeler dans loop() pour gérer les séquences non-bloquantes

private:
    int _pin;

    // État interne
    unsigned long _sonMillis;
    int _etatSon; // 0: inactif, 1: ton simple, 2: séquence
    unsigned long _sonDuration; // Durée restante du son actuel

    // Pour les séquences
    struct SoundStep {
        int frequency;
        int duration;
    };

    static const int MAX_STEPS = 10;
    SoundStep _sequence[MAX_STEPS];
    int _sequenceLength;
    int _sequenceIndex;

    void startTone(int frequency, int duration);
    void stop();
};

#endif
