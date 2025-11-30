#include "GameLogic.h"

GameLogic::GameLogic(Config cfg) : config(cfg) {
    Mcmptr1 = 0;
    compteurCredits = 0;
    cmptrjrn = 0;
    totcmptr = 0;
    cmptr01 = 0;
    relayActive = false;
    relayStartTime = 0;
    lastCreditSentTime = 0;
}

GameLogic::Output GameLogic::update(Input input) {
    Output out;
    out.soundToPlay = -1;
    out.relayActive = relayActive; // Maintain current state by default
    out.updateScreen = false;
    out.saveToNvram = false;

    // Handle Incoming Data
    if (input.dataReceived) {
        // Logic from main.cpp:
        // if ((dataRcvr.cp1 > 0) && (Mcmptr1 == 0)) { compteurCredits = 0; }
        if (input.receivedData.cp1 > 0 && Mcmptr1 == 0) {
            compteurCredits = 0;
        }
        Mcmptr1 += input.receivedData.cp1;
        out.updateScreen = true;
    }

    // Handle Relay Deactivation
    if (relayActive && (input.currentMillis - relayStartTime >= config.relayPulseDuration)) {
        relayActive = false;
        out.relayActive = false;
    }

    // Handle Pending Credits (Injecting coins)
    if (Mcmptr1 > 0 && !relayActive && (input.currentMillis - lastCreditSentTime >= config.delayBetweenPulses)) {
        out.soundToPlay = 0;

        // Add Credit
        compteurCredits++;

        // Trigger Relay
        relayActive = true;
        relayStartTime = input.currentMillis;
        out.relayActive = true;

        lastCreditSentTime = input.currentMillis;

        Mcmptr1--;
        cmptr01++;
        cmptrjrn++;
        totcmptr++;

        if (Mcmptr1 == 0) {
             out.soundToPlay = 5;
        }

        out.saveToNvram = true;
        out.updateScreen = true;
    }

    return out;
}
