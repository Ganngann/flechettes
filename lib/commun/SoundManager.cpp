#include "SoundManager.h"

SoundManager::SoundManager(int pin) : _pin(pin), _etatSon(0), _sequenceLength(0), _sequenceIndex(0) {
}

void SoundManager::init() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void SoundManager::play(SoundType type) {
    // Si un son est déjà en cours, on ne l'interrompt que si c'est une erreur critique
    if (_etatSon != 0 && type != SOUND_CRITICAL_ERROR && _etatSon != 0) return;

    stop(); // Arrêt du son précédent
    _etatSon = 0;
    _sequenceLength = 0;
    _sequenceIndex = 0;

    switch (type) {
        case SOUND_BIP:
            startTone(1000, 200);
            _etatSon = 1;
            break;

        case SOUND_SUCCESS:
            startTone(1500, 150);
            _etatSon = 1;
            break;

        case SOUND_ERROR:
            startTone(500, 300);
            _etatSon = 1;
            break;

        case SOUND_STARTUP:
            _sequenceLength = 3;
            _sequence[0] = {800, 100};
            _sequence[1] = {1000, 100};
            _sequence[2] = {1200, 100};
            _etatSon = 2;
            break;

        case SOUND_BUTTON:
            startTone(1200, 100);
            _etatSon = 1;
            break;

        case SOUND_CRITICAL_ERROR:
            _sequenceLength = 6;
            _sequence[0] = {200, 200};
            _sequence[1] = {600, 200};
            _sequence[2] = {1200, 150};
            _sequence[3] = {1800, 100};
            _sequence[4] = {400, 300};
            _sequence[5] = {100, 400};
            _etatSon = 2;
            break;

        default:
            stop();
            break;
    }

    // Si c'est une séquence, on lance la première étape immédiatement
    if (_etatSon == 2 && _sequenceLength > 0) {
        SoundStep step = _sequence[_sequenceIndex];
        startTone(step.frequency, step.duration);
        _sequenceIndex++;
    }
}

void SoundManager::update() {
    if (_etatSon == 0) return;

    unsigned long currentMillis = millis();

    if (currentMillis - _sonMillis >= _sonDuration) {
        if (_etatSon == 1) {
            // Fin d'un son simple
            stop();
        } else if (_etatSon == 2) {
            // Fin d'une étape de séquence, passage à la suivante
            if (_sequenceIndex < _sequenceLength) {
                SoundStep step = _sequence[_sequenceIndex];
                startTone(step.frequency, step.duration);
                _sequenceIndex++;
            } else {
                stop();
            }
        }
    }
}

void SoundManager::startTone(int frequency, int duration) {
    tone(_pin, frequency);
    _sonMillis = millis();
    _sonDuration = duration;
}

void SoundManager::stop() {
    noTone(_pin);
    _etatSon = 0;
}
