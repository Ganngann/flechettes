#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

#include "SharedData.h"
#include <stdint.h>

class GameLogic {
public:
    struct Config {
        unsigned long relayPulseDuration;
        unsigned long delayBetweenPulses;
    };

    struct Input {
        unsigned long currentMillis;
        // Received data
        bool dataReceived;
        struct_message receivedData;
    };

    struct Output {
        int soundToPlay; // -1 none
        bool relayActive;
        bool updateScreen;
        bool saveToNvram;
    };

    GameLogic(Config config);

    Output update(Input input);

    // Persistent state (should be loaded/saved)
    uint16_t getPendingCredits() const { return Mcmptr1; }
    uint16_t getTotalCredits() const { return compteurCredits; }

    // Setters for initialization
    void setStats(uint16_t pending, uint16_t credits, uint16_t daily, uint16_t total) {
        Mcmptr1 = pending;
        compteurCredits = credits;
        cmptrjrn = daily;
        totcmptr = total;
    }

private:
    Config config;

    uint16_t Mcmptr1; // Received credits queue
    uint16_t compteurCredits;
    uint16_t cmptrjrn;
    uint16_t totcmptr;
    uint16_t cmptr01;

    bool relayActive;
    unsigned long relayStartTime;
    unsigned long lastCreditSentTime;
};

#endif
