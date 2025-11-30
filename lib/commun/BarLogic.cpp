#include "BarLogic.h"

BarLogic::BarLogic(Config cfg) : config(cfg) {
    currentState = ETAT_INIT;
    totCn = 0;
    stateStartTime = 0;
    screenHoldUntil = 0;
    isSending = false;
    sendStartTime = 0;

    b1Last = false; b2Last = false; b3Last = false;
    b2Enfonce = false;
    b1Time = 0; b2Time = 0; b3Time = 0;
}

void BarLogic::changeState(State newState, unsigned long millis) {
    currentState = newState;
    stateStartTime = millis;
}

BarLogic::Output BarLogic::update(Input input) {
    Output out;
    out.soundToPlay = -1;
    out.screenId = -1;
    out.sendMessage = false;
    out.ledRed = false;
    out.ledBlue = (totCn == 0); // Blue if 0 credit, as per main.cpp

    // Button Edge Detection
    bool b1Clicked = false;
    bool b2Clicked = false;
    bool b3Clicked = false;

    // B1: Press detection (Debounce 300ms)
    if (input.btn1Down && !b1Last && (input.currentMillis - b1Time > 300)) {
        b1Clicked = true;
        b1Time = input.currentMillis;
    }
    b1Last = input.btn1Down;

    // B3: Press detection (Debounce 300ms)
    if (input.btn3Down && !b3Last && (input.currentMillis - b3Time > 300)) {
        b3Clicked = true;
        b3Time = input.currentMillis;
    }
    b3Last = input.btn3Down;

    // B2: Release detection (Debounce 50ms)
    if (input.btn2Down && !b2Last && (input.currentMillis - b2Time > 50)) {
        b2Enfonce = true;
        b2Time = input.currentMillis;
    }
    if (!input.btn2Down && b2Last && b2Enfonce && (input.currentMillis - b2Time > 50)) {
        b2Clicked = true;
        b2Enfonce = false;
        b2Time = input.currentMillis;
    }
    b2Last = input.btn2Down;


    switch (currentState) {
        case ETAT_INIT:
            out.screenId = 1;
            changeState(ETAT_ATTENTE, input.currentMillis);
            break;

        case ETAT_ATTENTE:
             // Note: Screen updates for timeout expiry are handled by transitioning back to this state
             // which usually triggers a screen refresh if needed.

             if (b2Clicked) {
                 if (!input.isConnected) {
                     out.soundToPlay = 6;
                     out.screenId = 9;
                     screenHoldUntil = input.currentMillis + 3000;
                     changeState(ETAT_TIMEOUT_ECRAN, input.currentMillis);
                 } else {
                     totCn += config.valTotCn;
                     out.soundToPlay = 1;
                     out.screenId = 3;
                     screenHoldUntil = input.currentMillis + 100;
                 }
             }
             else if (b1Clicked) {
                 out.soundToPlay = 1;
                 if (totCn == 0) changeState(ETAT_COMPTEURS, input.currentMillis);
                 else changeState(ETAT_ANNULATION, input.currentMillis);
             }
             else if (b3Clicked) {
                 out.soundToPlay = 5;
                 if (totCn > config.maxCred) {
                     totCn = 0;
                     out.soundToPlay = 6;
                     out.screenId = 8;
                     screenHoldUntil = input.currentMillis + 4000;
                     changeState(ETAT_TIMEOUT_ECRAN, input.currentMillis);
                 } else if (totCn > 0) {
                     changeState(ETAT_ENVOI_CREDIT, input.currentMillis);
                 } else {
                     changeState(ETAT_PUB, input.currentMillis);
                 }
             }
             break;

        case ETAT_ANNULATION:
             totCn = 0;
             out.soundToPlay = 3;
             out.screenId = 8;
             screenHoldUntil = input.currentMillis + 4000;
             changeState(ETAT_TIMEOUT_ECRAN, input.currentMillis);
             break;

        case ETAT_ENVOI_CREDIT:
             out.sendMessage = true;
             out.messageData.cp1 = totCn;
             out.messageData.fs1 = true;
             out.messageData.fp1 = false;
             // We don't populate cp2/cp3 here as they are persistent stats managed outside logic for now

             isSending = true;
             sendStartTime = input.currentMillis;
             changeState(ETAT_RESULTAT_ENVOI, input.currentMillis);
             break;

        case ETAT_RESULTAT_ENVOI:
             // Main.cpp waits for timeout regardless of callback speed
             if ((long)(input.currentMillis - sendStartTime) >= (long)config.sendTimeout) {
                 isSending = false;
                 if (input.sendSuccess) {
                     out.screenId = 20; // Success
                     out.soundToPlay = 2;
                     screenHoldUntil = input.currentMillis + 1000;
                 } else {
                     out.screenId = 21; // Fail
                     out.soundToPlay = 6;
                     screenHoldUntil = input.currentMillis + 3000;
                 }
                 totCn = 0;
                 changeState(ETAT_TIMEOUT_ECRAN, input.currentMillis);
             }
             break;

        case ETAT_PUB:
             out.screenId = 4;
             screenHoldUntil = input.currentMillis + 3000;
             changeState(ETAT_TIMEOUT_ECRAN, input.currentMillis);
             break;

        case ETAT_COMPTEURS:
             out.screenId = 6;
             screenHoldUntil = input.currentMillis + 3000;
             changeState(ETAT_TIMEOUT_ECRAN, input.currentMillis);
             break;

        case ETAT_TIMEOUT_ECRAN:
             if ((long)(input.currentMillis - screenHoldUntil) > 0) {
                 if (totCn == 0) out.screenId = 1;
                 else out.screenId = 3;

                 changeState(ETAT_ATTENTE, input.currentMillis);
             }
             break;

        default:
             break;
    }

    return out;
}
