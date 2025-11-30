#include "SoundManager.h"

SoundManager::SoundManager(int pin) : _pin(pin), _soundMillis(0), _soundActiveDuration(0), _currentState(0), _sequenceSize(0), _sequenceIndex(0) {}

void SoundManager::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void SoundManager::play(int effect) {
    if (_currentState != 0) return; // Don't interrupt playing sound

    switch (effect) {
        case BIP:
            tone(_pin, 1000);
            _soundMillis = millis();
            _soundActiveDuration = 200;
            _currentState = 1;
            break;

        case SUCCESS:
            // Bar uses: 1500Hz, 150ms. Flechettes uses sequence.
            // We'll use the Bar version for simplicity or maybe adapt?
            // Let's stick to Bar version as it was more complex in structure.
            tone(_pin, 1500);
            _soundMillis = millis();
            _soundActiveDuration = 150;
            _currentState = 1;
            break;

        case ERROR:
            tone(_pin, 500);
            _soundMillis = millis();
            _soundActiveDuration = 300;
            _currentState = 1;
            break;

        case STARTUP:
            _sequenceSize = 3;
            _sequence[0] = {800, 100};
            _sequence[1] = {1000, 100};
            _sequence[2] = {1200, 100};
            _sequenceIndex = 0;
            _currentState = 2;
            break;

        case BUTTON:
            tone(_pin, 1200);
            _soundMillis = millis();
            _soundActiveDuration = 100;
            _currentState = 1;
            break;

        case CRITICAL_ERROR:
            _sequenceSize = 6;
            _sequence[0] = {200, 200};
            _sequence[1] = {600, 200};
            _sequence[2] = {1200, 150};
            _sequence[3] = {1800, 100};
            _sequence[4] = {400, 300};
            _sequence[5] = {100, 400};
            _sequenceIndex = 0;
            _currentState = 2;
            break;

        default:
            noTone(_pin);
            _currentState = 0;
            break;
    }
}

void SoundManager::update() {
    if (_currentState == 1) {
        if (millis() - _soundMillis >= _soundActiveDuration) {
            noTone(_pin);
            _currentState = 0;
        }
    } else if (_currentState == 2) {
        if (millis() - _soundMillis >= _soundActiveDuration) {
            if (_sequenceIndex < _sequenceSize) {
                tone(_pin, _sequence[_sequenceIndex].frequency);
                _soundMillis = millis();
                _soundActiveDuration = _sequence[_sequenceIndex].duration;
                _sequenceIndex++;
            } else {
                noTone(_pin);
                _currentState = 0;
            }
        }
    }
}
