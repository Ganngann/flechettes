#ifndef BAR_LOGIC_H
#define BAR_LOGIC_H

#include "SharedData.h"
#include <stdint.h>

class BarLogic {
public:
    enum State {
        ETAT_INIT,
        ETAT_ATTENTE,
        ETAT_AJOUT_CREDIT,
        ETAT_ANNULATION,
        ETAT_ENVOI_CREDIT,
        ETAT_RESULTAT_ENVOI,
        ETAT_INFO_ZERO,
        ETAT_TIMEOUT_ECRAN,
        ETAT_PUB,
        ETAT_COMPTEURS,
        ETAT_CREDIT,
        ETAT_JEU
    };

    struct Config {
        uint16_t valTotCn;      // Value to add per credit (e.g. 1)
        uint16_t maxCred;       // Max credits allowed
        unsigned long sendTimeout; // Timeout for network send
    };

    struct Input {
        unsigned long currentMillis;
        bool btn1Down;      // Current state of Button 1 (True = Pressed/Low)
        bool btn2Down;      // Current state of Button 2
        bool btn3Down;      // Current state of Button 3
        bool isConnected;   // Network connection status
        bool sendSuccess;   // Result of the last send operation (valid only if we were sending)
        bool sendDone;      // True if send callback received
    };

    struct Output {
        int soundToPlay;    // -1 = None. IDs match son.h
        int screenId;       // -1 = No change. IDs match ecran() logic
        bool sendMessage;   // True if we need to trigger a send
        struct_message messageData; // Data to send
        bool ledRed;        // LED status
        bool ledBlue;       // LED status
    };

    BarLogic(Config config);

    // Process loop. Call this every iteration.
    Output update(Input input);

    // Getters
    State getState() const { return currentState; }
    uint16_t getTotalCredits() const { return totCn; }
    void setCredits(uint16_t c) { totCn = c; } // For testing or loading from NVS

private:
    Config config;
    State currentState;
    uint16_t totCn;

    unsigned long stateStartTime;
    unsigned long screenHoldUntil;

    // Button debounce/edge tracking
    bool b1Last, b2Last, b3Last;
    bool b2Enfonce;
    unsigned long b1Time, b2Time, b3Time;

    bool isSending;
    unsigned long sendStartTime;

    // Helpers
    void changeState(State newState, unsigned long millis);
};

#endif
