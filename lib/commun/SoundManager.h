#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <utils.h>
// Mock functions for native tests
void pinMode(int pin, int mode);
void digitalWrite(int pin, int val);
void tone(int pin, int freq, int duration = 0);
void noTone(int pin);
unsigned long millis();
#define OUTPUT 1
#define LOW 0
#endif

class SoundManager {
public:
    enum SoundEffect {
        NONE = 0,
        BIP = 1,
        SUCCESS = 2,
        ERROR = 3,
        STARTUP = 4,
        BUTTON = 5,
        CRITICAL_ERROR = 6
    };

    SoundManager(int pin);
    void begin();
    void play(int effect);
    void update();

private:
    int _pin;
    unsigned long _soundMillis;
    int _soundActiveDuration;
    int _currentState; // 0: idle, 1: single tone, 2: sequence

    // Sequence handling
    struct SoundElement {
        int frequency;
        int duration;
    };

    static const int MAX_SEQUENCE = 10;
    SoundElement _sequence[MAX_SEQUENCE];
    int _sequenceSize;
    int _sequenceIndex;
};

#endif
