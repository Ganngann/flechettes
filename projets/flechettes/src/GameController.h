#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <Arduino.h>
#include <ezButton.h>
#include <Adafruit_ST7735.h>
#include <PCF8574.h>
#include <DS1307new.h>
#include <Wire.h>
#include "SharedData.h"
#include "SoundManager.h"
#include "NetworkManager.h"
#include "config.h"

enum ModeEcran {
    MODE_CPT,
    MODE_BOUTON1,
    MODE_BOUTON2,
    MODE_BOUTON3
};

class GameController {
public:
    GameController();
    void setup();
    void loop();

private:
    // Components
    Adafruit_ST7735 _tft;
    SoundManager _soundManager;
    PCF8574 _pcf8574;

    // Inputs
    static const int BUTTON_COUNT = 3;
    ezButton _buttons[BUTTON_COUNT];

    // State
    bool _setupTermine;
    int _etapeSetup;
    unsigned long _previousMillisSetup;

    // Game State
    uint16_t _Mcmptr1, _Mcmptr2, _Mcmptr3;
    uint16_t _cmptr01;
    bool _Mfp1, _Mfs1;
    uint16_t _cmptrjrn, _totcmptr;
    uint16_t _compteurCredits;

    // Previous states for display
    int _prev_Mcmptr1;
    int _prev_cmptrjrn;
    int _prev_totcmptr;
    int _prev_compteurCredits;
    bool _etatLed01;
    bool _premierAffichageFait;

    // Logic flags
    bool _forcerMajEcran;
    bool _resetEnCours;
    unsigned long _momentConfirmation;
    ModeEcran _modeEcran;

    // Timers
    unsigned long _dernierAffichageCpt;
    unsigned long _dernierCreditEnvoye;

    // Pub
    bool _pubEnCours;
    unsigned long _pubStartTime;

    // Relais
    bool _relaisEnCours;
    unsigned long _relaisStartTime;

    // Network
    struct_message _dataSent;
    struct_message _dataRcvr;

    // Address
    uint8_t _address; // I2C address scan var
    uint8_t _nDevices;
    bool _pcf8574Present;

    // Methods
    void setupNonBloquant();
    void cpt();
    void gestionRelais();
    void declencherImpulsionRelais();
    void ajouterCredit();
    void lancerPublicite();
    void verifierFinPublicite();

    void gererAppuiLongRemiseJournalier();
    void gererAppuiLongRemiseTotaux();

    // Helpers
    void initialiserModules();
    bool verifierPileRTC();
    void writeToNVram();
    void readFromNVram();
    void SendCmptr();

    // Callbacks
    void onDataReceived(const struct_message& data, const uint8_t* mac);
    void onDataSent(bool success);
};

#endif
