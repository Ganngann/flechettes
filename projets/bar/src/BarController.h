#ifndef BAR_CONTROLLER_H
#define BAR_CONTROLLER_H

#include <Arduino.h>
#include <ezButton.h>
#include <Adafruit_ST7735.h>
#include <I2C_eeprom.h>
#include <SharedData.h>
#include <SoundManager.h>
#include <NetworkManager.h>
#include "config.h"
#include "config_cpu.h"

// Définitions des états
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

class BarController {
public:
    BarController();
    void setup();
    void loop();

private:
    // --- Composants ---
    Adafruit_ST7735 _tft;
    I2C_eeprom _ee;
    SoundManager _soundManager;

    // --- Entrées ---
    static const int BUTTON_COUNT = 3;
    ezButton _buttons[BUTTON_COUNT];

    // --- État du système ---
    State _etatActuel;
    bool _setupTermine;
    int _etapeSetup;
    unsigned long _previousMillisSetup;

    // --- Logique Métier ---
    bool _isConnected;
    bool _sendStatus; // Remplaçant SendStts
    bool _ecranInitialise;
    uint8_t _ok; // État d'affichage (voir switch case dans ecran())

    uint16_t _TotCn;
    uint16_t _TotCn_prev;
    uint16_t _cmptrjrn;
    uint16_t _totcmptr;

    // Timers et délais
    unsigned long _screenHoldUntil;
    unsigned long _sendStartTime;
    bool _isSending;
    unsigned long _previousConnectionCheck;
    bool _previousConnectionState;

    // Scroll
    int _scrollY;
    bool _messageScrolling;
    unsigned long _previousMillisScroll;
    const char* _currentMsg;

    // LEDs et indicateurs
    uint16_t _led01;
    bool _led01On;
    unsigned long _led01StartTime;

#if TYPE_CPU == 3
    bool _ledClignotementActif;
    unsigned long _ledClignoStart;
    bool _clignotementAnnulationActif;
    unsigned long _clignoAnnulStart;
    bool _annulationCreditExces;
    bool _annulationCreditReset;
    unsigned long _previousClignoDiscoMillis;
    bool _ledRedDiscoState;
#endif

    // Données Réseau
    struct_message _dataSent;
    struct_message _dataRcvr;

    // --- Méthodes ---
    void setupNonBloquant();
    void ecran();
    void handleButtons();
    void checkConnection();

    // Callbacks
    void onDataReceived(const struct_message& data, const uint8_t* mac);
    void onDataSent(bool success);

    // Helpers
    void initialiserModules();
    void chargerCompteurs();
    void sauvegarderCompteurs();

    void gererLEDTransfert(bool transfertOK);
    void gererEtatsLEDs();
};

#endif
