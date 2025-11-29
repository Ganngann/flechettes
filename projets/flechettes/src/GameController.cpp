#include "GameController.h"
#include "affichage.h"

// Pins
#define TFT_SCLK 18
#define TFT_MOSI 23
#define TFT_CS 15
#define TFT_RST 13
#define TFT_DC 4

const int BUZZER_PIN = 26;
const uint8_t RELAY_1_PIN = 25; // rel01
const uint8_t RELAY_2_PIN = 27; // rel02
const uint8_t LED_01_PIN = 19;

// Button Pins
const uint8_t BUTTON_1_PIN = 32;
const uint8_t BUTTON_2_PIN = 35;
const uint8_t BUTTON_3_PIN = 34;

// Delays
const unsigned long DELAI_I2C_SCAN     = 1500;
const unsigned long DELAI_FIN_SCAN     = 100;
const unsigned long DELAI_FIN_Nvram    = 1500;
const unsigned long DELAI_FIN_Peer     = 2000;
const unsigned long DELAI_FINAL_STEP2  = 3000;

const unsigned long intervalLoop       = 100;

#define PCF_ADDRESS 0x20

GameController::GameController()
    : _tft(TFT_CS, TFT_DC, TFT_RST),
      _soundManager(BUZZER_PIN),
      _pcf8574(PCF_ADDRESS),
      _buttons{ezButton(BUTTON_1_PIN), ezButton(BUTTON_2_PIN), ezButton(BUTTON_3_PIN)},
      _setupTermine(false),
      _etapeSetup(0),
      _previousMillisSetup(0),
      _Mcmptr1(0), _Mcmptr2(0), _Mcmptr3(0),
      _cmptr01(0), _Mfp1(false), _Mfs1(false),
      _cmptrjrn(0), _totcmptr(0), _compteurCredits(0),
      _prev_Mcmptr1(-1), _prev_cmptrjrn(-1), _prev_totcmptr(-1), _prev_compteurCredits(-1),
      _etatLed01(false), _premierAffichageFait(false),
      _forcerMajEcran(false), _resetEnCours(false), _momentConfirmation(0),
      _modeEcran(MODE_CPT),
      _dernierAffichageCpt(0), _dernierCreditEnvoye(0),
      _pubEnCours(false), _pubStartTime(0),
      _relaisEnCours(false), _relaisStartTime(0),
      _address(1), _nDevices(0), _pcf8574Present(false)
{
}

void GameController::setup() {
    setupNonBloquant();
}

void GameController::initialiserModules() {
    // Initialisation des E/S
    pinMode(RELAY_1_PIN, OUTPUT);
    pinMode(RELAY_2_PIN, OUTPUT);
    digitalWrite(RELAY_2_PIN, LOW); // Initial state
    pinMode(LED_01_PIN, OUTPUT);

    _soundManager.init();

    _tft.initR(INITR_GREENTAB);
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setRotation(3);

    Wire.begin(21, 22);
    WiFi.mode(WIFI_STA);

    // ee.begin() was in main.cpp but unused logic?
    // I2C_eeprom ee...
}

void GameController::setupNonBloquant() {
    unsigned long currentMillis = millis();
    static bool rtcPresent = true;

    switch (_etapeSetup) {
        case 0:
            for(int i=0; i<3; i++) _buttons[i].setDebounceTime(50);

            initialiserModules();
            Serial.begin(115200);

            afficherNum(_tft);
            _previousMillisSetup = currentMillis;
            _etapeSetup++;
            break;

        case 1:
            if (currentMillis - _previousMillisSetup >= DELAI_I2C_SCAN) {
                // afficherNumSerie(_tft); // Was in main.cpp? main.cpp called afficherNumSerie in setupNonBloquant case 1
                // But affichage.cpp doesn't have it? Ah, I implemented it in bar/src/serie.cpp.
                // Does flechettes have serie.cpp?
                // Let's assume yes or use available functions.
                // affichage.cpp in flechettes has afficherSetup.
                // main.cpp called afficherNumSerie AND afficherSetup?
                // logic: afficherNumSerie(tft); afficherSetup();

                // I will skip afficherNumSerie if not available or just call afficherSetup
                afficherSetup(_tft);
                _previousMillisSetup = currentMillis;
                _etapeSetup++;
            }
            break;

        case 2:
            if (currentMillis - _previousMillisSetup >= DELAI_I2C_SCAN) {
                _tft.setTextSize(2);
                _tft.fillScreen(ST77XX_BLACK);
                _tft.setCursor(0, 0);
                _tft.setTextColor(ST77XX_ORANGE);
                _tft.println("Scan I2C...");
                _tft.setTextColor(ST77XX_WHITE);
                _address = 1;
                _nDevices = 0;
                _etapeSetup++;
                _previousMillisSetup = currentMillis;
            }
            break;

        case 3:
            if (currentMillis - _previousMillisSetup >= 20) { // Faster scan
                if (_address < 127) {
                    Wire.beginTransmission(_address);
                    byte error = Wire.endTransmission();
                    if (error == 0) {
                        _tft.setTextSize(2);
                        _tft.setTextColor(ST77XX_WHITE);
                        _tft.print("I2C  >0x");
                        _tft.println(_address, HEX);
                        _soundManager.play(SOUND_BIP);
                        _nDevices++;
                    }
                    _address++;
                    _previousMillisSetup = currentMillis;
                } else {
                    _etapeSetup++;
                    _previousMillisSetup = currentMillis;
                }
            }
            break;

        case 4:
             rtcPresent = RTC.isPresent(); // Global RTC
             if (!rtcPresent) {
                 _tft.setTextColor(ST77XX_RED);
                 _tft.println("\nHW111> OFF\n");
             } else {
                 _tft.println("HW111> OK");
             }
             _previousMillisSetup = currentMillis;
             _etapeSetup++;
             break;

        case 5:
             if (currentMillis - _previousMillisSetup >= DELAI_FIN_SCAN) {
                 if (_nDevices == 0) _tft.println("Er03 > I2C");

                 // ESP-NOW Init is handled in NetworkManager
                 // We will init it later or check here?
                 // NetworkManager::getInstance().init() calls esp_now_init.
                 // We can do it here.
                 _tft.println("NOW  > Ok"); // Optimistic

                 _etapeSetup++;
                 _previousMillisSetup = currentMillis;
             }
             break;

        case 6: {
             bool pcfDetecte = false;
             uint8_t adresseTrouvee = 0;
             for (uint8_t addr = 0x20; addr <= 0x3F; addr++) {
                 Wire.beginTransmission(addr);
                 if (Wire.endTransmission() == 0) {
                     pcfDetecte = true;
                     adresseTrouvee = addr;
                     break;
                 }
             }

             if (pcfDetecte) {
                 if (adresseTrouvee == PCF_ADDRESS) {
                     _pcf8574.begin(adresseTrouvee);
                     _tft.setTextColor(ST77XX_GREEN);
                     _tft.print("PCF  >0x"); _tft.println(adresseTrouvee, HEX);
                     _soundManager.play(SOUND_SUCCESS);
                 } else {
                     _tft.setTextColor(ST77XX_RED);
                     _tft.print("PCFer>0x"); _tft.println(PCF_ADDRESS, HEX);
                     _soundManager.play(SOUND_SUCCESS);
                 }
             } else {
                 _tft.setTextColor(ST77XX_RED);
                 _tft.println("PCF  >OFF");
                 _soundManager.play(SOUND_ERROR);
             }

             _previousMillisSetup = currentMillis;
             _etapeSetup++;
             break;
        }

        case 7:
             if (currentMillis - _previousMillisSetup >= DELAI_FIN_Nvram) {
                 _tft.fillScreen(ST77XX_BLACK);
                 _tft.setCursor(0, 2);
                 if (verifierPileRTC()) {
                     _tft.setTextColor(ST77XX_GREEN);
                     _tft.println("\nInitialisation OK");
                     _soundManager.play(SOUND_BIP);
                 } else {
                     _tft.setTextColor(ST77XX_YELLOW);
                     _tft.println("\nEffacement NVRAM fait");
                     _soundManager.play(SOUND_ERROR);
                 }
                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 8:
             if (currentMillis - _previousMillisSetup >= DELAI_FIN_Peer) {
                 // Configure outputs
                 _pcf8574.write(0, LOW);
                 _pcf8574.write(1, LOW);
                 for (int i = 2; i < 8; i++) _pcf8574.write(i, HIGH);

                 // Init Network
                 NetworkManager::getInstance().setOnReceiveCallback(
                    [this](const struct_message& data, const uint8_t* mac){ this->onDataReceived(data, mac); }
                 );
                 NetworkManager::getInstance().setOnSendCallback(
                    [this](bool success){ this->onDataSent(success); }
                 );

                 NetworkManager::getInstance().init(broadcastAddress);
                 // Assuming init works.

                 _tft.setTextColor(ST77XX_GREEN);
                 _tft.setTextSize(2);
                 _tft.println("Peer  Ok");

                 _etatLed01 = false;
                 digitalWrite(LED_01_PIN, _etatLed01);

                 readFromNVram();

                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 9:
             if (currentMillis - _previousMillisSetup >= DELAI_FINAL_STEP2) {
                 if (rtcPresent) {
                     uint16_t TimeIsSet;
                     RTC.getRAM(50, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
                     if (TimeIsSet != 0xaa55) {
                         TimeIsSet = 0xaa55;
                         RTC.setRAM(50, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
                     }
                 }
                 afficherInit(_tft);
                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 10:
             if (currentMillis - _previousMillisSetup >= 1000) {
                 _soundManager.play(SOUND_BIP);
                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 11:
             if (currentMillis - _previousMillisSetup >= 5000) {
                 _soundManager.play(SOUND_BIP);
                 digitalWrite(RELAY_1_PIN, HIGH);
                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 12:
             if (currentMillis - _previousMillisSetup >= 1800) {
                 digitalWrite(RELAY_1_PIN, LOW);
                 _soundManager.play(SOUND_BIP);
                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 13:
             if (currentMillis - _previousMillisSetup >= 5000) {
                 _soundManager.play(SOUND_SUCCESS);
                 afficherStart(_tft);
                 _previousMillisSetup = currentMillis;
                 _etapeSetup++;
             }
             break;

        case 14:
             if (currentMillis - _previousMillisSetup >= 2000) {
                 digitalWrite(RELAY_2_PIN, LOW);
                 _setupTermine = true;
                 _forcerMajEcran = true;
                 _premierAffichageFait = false;
                 _Mcmptr1 = 0;
                 _compteurCredits = 0;
             }
             break;
    }
}

void GameController::loop() {
    static unsigned long previousMillisLoop = 0;
    unsigned long maintenant = millis();

    if (maintenant - previousMillisLoop < intervalLoop) return;
    previousMillisLoop = maintenant;

    if (!_setupTermine) {
        setupNonBloquant();
    } else {
        if (!_premierAffichageFait) {
            cpt();
            _premierAffichageFait = true;
        }

        verifierFinPublicite();

        _pcf8574.write(0, HIGH);

        // Gestion du crédit (impulsion 1 sec entre deux ajouts)
        const unsigned long delaiEntreImpulsions = 1000;
        if (_Mcmptr1 > 0 && !_relaisEnCours && maintenant - _dernierCreditEnvoye >= delaiEntreImpulsions) {
             _soundManager.play(SOUND_ERROR); // Sound 0? Assuming Error/Grave
             ajouterCredit();
             _dernierCreditEnvoye = maintenant;

             _Mcmptr1--;
             _cmptr01++;
             _cmptrjrn++;
             _totcmptr++;

             if (_Mcmptr1 == 0) _soundManager.play(SOUND_BUTTON); // Sound 5

             writeToNVram();
             _forcerMajEcran = true;
        }

        for(int i=0; i<3; i++) _buttons[i].loop();
        bool b1 = _buttons[0].getState() == LOW;
        bool b2 = _buttons[1].getState() == LOW;
        bool b3 = _buttons[2].getState() == LOW;

        if (b1) _modeEcran = MODE_BOUTON1;
        else if (b2) _modeEcran = MODE_BOUTON2;
        else if (b3) _modeEcran = MODE_BOUTON3;

        if (_pubEnCours) lancerPublicite();

        static bool enVueSpeciale = false;

        switch (_modeEcran) {
            case MODE_CPT:
                if (!_resetEnCours && (maintenant - _dernierAffichageCpt >= 1000)) {
                    _dernierAffichageCpt = maintenant;
                    cpt();
                }
                break;
            case MODE_BOUTON1:
                enVueSpeciale = true;
                gererAppuiLongRemiseTotaux();
                break;
            case MODE_BOUTON2:
                enVueSpeciale = true;
                lancerPublicite();
                break;
            case MODE_BOUTON3:
                enVueSpeciale = true;
                gererAppuiLongRemiseJournalier();
                break;
        }

        if (!b1 && !b2 && !b3) {
            if (enVueSpeciale) {
                enVueSpeciale = false;
                _forcerMajEcran = true;
            }
            _modeEcran = MODE_CPT;
        }

        SendCmptr();

        if (_forcerMajEcran && !_resetEnCours) {
            cpt();
            _forcerMajEcran = false;
        }

        gestionRelais();
        _soundManager.update();
    }

    if (_resetEnCours && millis() - _momentConfirmation >= 1500) {
        _resetEnCours = false;
        _forcerMajEcran = true;
    }
}

void GameController::cpt() {
    if (_modeEcran != MODE_CPT) return;
    static bool etatPrecedent = !_etatLed01;
    bool changementEtatConnexion = (_etatLed01 != etatPrecedent);
    bool changementCompteurs = (_Mcmptr1 != _prev_Mcmptr1 || _cmptrjrn != _prev_cmptrjrn || _totcmptr != _prev_totcmptr || _compteurCredits != _prev_compteurCredits);

    if (changementEtatConnexion || changementCompteurs || _forcerMajEcran) {
        _tft.fillScreen(_etatLed01 ? ST77XX_RED : ST77XX_BLUE);
        _tft.setTextSize(1);
        _tft.setCursor(5, 20);
        _tft.setTextColor(ST77XX_WHITE);
        _tft.print(_etatLed01 ? "Non connecte au bar" : "Connecte au bar");

        _tft.setTextColor(ST7735_YELLOW);
        _tft.setTextSize(2);

        uint16_t bgColor = _etatLed01 ? ST77XX_RED : ST77XX_BLUE;
        for (int i = 0; i < 4; i++) {
            _tft.fillRect(20, 40 + (i * 20), 100, 20, bgColor);
        }

        _tft.setCursor(1, 35); _tft.print("   :"); _tft.println(_Mcmptr1);
        _tft.setCursor(1, 60); _tft.print("Tep:"); _tft.println(_cmptrjrn);
        _tft.setCursor(1, 84); _tft.print("Tot:"); _tft.println(_totcmptr);
        _tft.setCursor(1, 109); _tft.print("Cre:"); _tft.println(_compteurCredits);

        etatPrecedent = _etatLed01;
        _prev_Mcmptr1 = _Mcmptr1;
        _prev_cmptrjrn = _cmptrjrn;
        _prev_totcmptr = _totcmptr;
        _prev_compteurCredits = _compteurCredits;
        _forcerMajEcran = false;
    }
}

void GameController::ajouterCredit() {
    _compteurCredits++;
    declencherImpulsionRelais();
}

void GameController::declencherImpulsionRelais() {
    if (_relaisEnCours) return;
    digitalWrite(RELAY_2_PIN, HIGH);
    _relaisStartTime = millis();
    _relaisEnCours = true;
}

void GameController::gestionRelais() {
    if (_relaisEnCours && millis() - _relaisStartTime >= 1) { // dureeImpulsion = 1
        digitalWrite(RELAY_2_PIN, LOW);
        _relaisEnCours = false;
    }
}

void GameController::lancerPublicite() {
    if (!_pubEnCours) {
        afficherPublicite(_tft);
        _pubStartTime = millis();
        _pubEnCours = true;
    }
}

void GameController::verifierFinPublicite() {
    if (_pubEnCours && millis() - _pubStartTime >= 3000) {
        _pubEnCours = false;
        _modeEcran = MODE_CPT;
        _forcerMajEcran = true;
    }
}

void GameController::onDataReceived(const struct_message& data, const uint8_t* mac) {
    memcpy(&_dataRcvr, &data, sizeof(struct_message));
    _Mfp1 = 0;

    if ((_dataRcvr.cp1 > 0) && (_Mcmptr1 == 0)) {
        _compteurCredits = 0;
    }

    _Mcmptr1 += _dataRcvr.cp1;
    _Mcmptr2 = _dataRcvr.cp2;
    _Mcmptr3 = _dataRcvr.cp3;
    _Mfp1 = _dataRcvr.fp1;
    _Mfs1 = _dataRcvr.fs1;

    // memset(&_dataRcvr, 0, sizeof(_dataRcvr)); // Reset after read?
}

void GameController::onDataSent(bool success) {
    // success toggle logic?
    static uint16_t result = 0;
    result = !result;
    _pcf8574.write(0, result);
    _pcf8574.write(1, success);

    _etatLed01 = success; // Wait, main.cpp says: etatLed01 = status (bool).
    digitalWrite(LED_01_PIN, _etatLed01);
}

void GameController::SendCmptr() {
    _dataSent.cp1 = _cmptr01;
    _dataSent.cp2 = _cmptrjrn;
    _dataSent.cp3 = _totcmptr;
    NetworkManager::getInstance().sendData(_dataSent);
    _dataSent.fs1 = 0;
}

bool GameController::verifierPileRTC() {
  uint8_t Nvr1, Nvr2;
  RTC.getRAM(0, (uint8_t *)&Nvr1, 1);
  RTC.getRAM(55, (uint8_t *)&Nvr2, 1);
  delay(100);

  if (Nvr1 == 0x5A && Nvr2 == 0xA5) {
    _tft.setTextColor(ST77XX_GREEN);
    _tft.println("NvRam  OK");
    _tft.println("Pile   OK");
    return true;
  } else {
    _tft.setTextColor(ST77XX_RED);
    _tft.println("NvRam  NOT OK");
    _tft.println("Pile   NOT OK");

    uint8_t zero = 0;
    for (int i = 0; i < 56; i++) RTC.setRAM(i, (uint8_t *)&zero, 1);

    Nvr1 = 0x5A; Nvr2 = 0xA5;
    RTC.setRAM(0, (uint8_t *)&Nvr1, 1);
    RTC.setRAM(55, (uint8_t *)&Nvr2, 1);
    delay(100);
    return false;
  }
}

void GameController::writeToNVram() {
  RTC.setRAM( 5, (uint8_t *)&_cmptrjrn, sizeof(uint16_t));
  RTC.setRAM( 7, (uint8_t *)&_totcmptr, sizeof(uint16_t));
  RTC.setRAM( 9, (uint8_t *)&_Mcmptr1, sizeof(uint16_t));
  RTC.setRAM(16, (uint8_t *)&_compteurCredits, sizeof(uint16_t));
}

void GameController::readFromNVram() {
  RTC.getRAM( 5, (uint8_t *)&_cmptrjrn, sizeof(uint16_t));
  RTC.getRAM( 7, (uint8_t *)&_totcmptr, sizeof(uint16_t));
  RTC.getRAM( 9, (uint8_t *)&_Mcmptr1, sizeof(uint16_t));
  RTC.getRAM(16, (uint8_t *)&_compteurCredits, sizeof(uint16_t));
}

void GameController::gererAppuiLongRemiseJournalier() {
  static bool boutonAppuye = false;
  static unsigned long debutAppui = 0;
  static unsigned long dernierBip = 0;
  static bool resetFait = false;
  const unsigned long dureeAppuiNecessaire = 3000;

  if (_buttons[2].getState() == LOW) { // Button 3
    if (!boutonAppuye) {
      boutonAppuye = true;
      debutAppui = millis();
      dernierBip = 0;
      resetFait = false;
      _tft.fillScreen(ST77XX_BLACK);
      _tft.setTextSize(2);
      _tft.setTextColor(ST77XX_WHITE);
      _tft.setCursor(10, 30);
      _tft.println("Clear Temp");
    }

    unsigned long maintenant = millis();
    unsigned long dureeAppui = maintenant - debutAppui;

    if (!resetFait) {
      if (maintenant - dernierBip >= 500) {
        _soundManager.play(SOUND_ERROR); // Sound 0
        dernierBip = maintenant;
      }
      int largeurTotale = 120;
      int hauteurBarre = 10;
      int posX = 10;
      int posY = 70;
      int largeurRemplie = map(dureeAppui, 0, dureeAppuiNecessaire, 0, largeurTotale);

      _tft.drawRect(posX, posY, largeurTotale, hauteurBarre, ST77XX_WHITE);
      _tft.fillRect(posX + 1, posY + 1, largeurRemplie, hauteurBarre - 2, ST77XX_GREEN);
    }

    if (dureeAppui >= dureeAppuiNecessaire && !resetFait) {
      _cmptrjrn = 0;
      _cmptr01 = 0;

      writeToNVram();
      _dataSent.cp2 = _cmptrjrn;
      NetworkManager::getInstance().sendData(_dataSent);
      _dataSent.fs1 = 0;

      _tft.fillScreen(ST77XX_GREEN);
      _tft.setTextSize(2);
      _tft.setTextColor(ST77XX_BLACK);
      _tft.setCursor(10, 40); _tft.println("Compteur");
      _tft.setCursor(10, 55); _tft.println("journalier");
      _tft.setCursor(10, 70); _tft.println("remis a");
      _tft.setCursor(10, 85); _tft.println("zero");

      _soundManager.play(SOUND_SUCCESS);

      _resetEnCours = true;
      _momentConfirmation = millis();
      _modeEcran = MODE_CPT;
      _forcerMajEcran = true;
      resetFait = true;
    }

  } else {
    if (boutonAppuye && !resetFait) {
      _tft.fillScreen(ST77XX_BLACK);
      _tft.setTextSize(2);
      _tft.setCursor(20, 50);
      _tft.setTextColor(ST77XX_RED);
      _tft.println("Annule...");
      _resetEnCours = true;
      _momentConfirmation = millis();
      _modeEcran = MODE_CPT;
      _forcerMajEcran = true;
    }
    boutonAppuye = false;
    resetFait = false;
  }
}

void GameController::gererAppuiLongRemiseTotaux() {
    // Similar logic for button 1
    static bool boutonAppuye = false;
    static unsigned long debutAppui = 0;
    static unsigned long dernierBip = 0;
    static bool resetFait = false;
    static int etatRemiseTotale = 0;
    const unsigned long dureeAppuiNecessaire = 3000;

    if (_buttons[0].getState() == LOW) {
        if (!boutonAppuye) {
            boutonAppuye = true;
            debutAppui = millis();
            dernierBip = 0;
            resetFait = false;
            etatRemiseTotale = 0;
        }

        unsigned long maintenant = millis();
        unsigned long dureeAppui = maintenant - debutAppui;

        if (etatRemiseTotale == 0) {
             _tft.fillScreen(ST77XX_BLACK);
            _tft.setTextSize(2);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.setCursor(10, 30);
            _tft.println("Clear Cpts");
            etatRemiseTotale = 1;
        }

        if (!resetFait) {
            if (maintenant - dernierBip >= 500) {
                _soundManager.play(SOUND_ERROR);
                dernierBip = maintenant;
            }
             int largeurTotale = 120;
            int hauteurBarre = 10;
            int posX = 10;
            int posY = 70;
            int largeurRemplie = map(dureeAppui, 0, dureeAppuiNecessaire, 0, largeurTotale);
            _tft.drawRect(posX, posY, largeurTotale, hauteurBarre, ST77XX_WHITE);
            _tft.fillRect(posX + 1, posY + 1, largeurRemplie, hauteurBarre - 2, ST77XX_ORANGE);
        }

        if (dureeAppui >= dureeAppuiNecessaire && !resetFait) {
            _cmptrjrn = 0;
            _cmptr01 = 0;
            _totcmptr = 0;

            writeToNVram();
            _dataSent.cp2 = _cmptrjrn;
            _dataSent.cp3 = _totcmptr;
            NetworkManager::getInstance().sendData(_dataSent);
            _dataSent.fs1 = 0;

            _tft.fillScreen(ST77XX_ORANGE);
            _tft.setTextSize(2);
            _tft.setTextColor(ST77XX_BLACK);
            _tft.setCursor(20, 40); _tft.println("Tout les");
            _tft.setCursor(20, 55); _tft.println("compteurs");
            _tft.setCursor(20, 70); _tft.println("remis a");
            _tft.setCursor(20, 85); _tft.println("zero");

            _soundManager.play(SOUND_SUCCESS);

            _resetEnCours = true;
            _momentConfirmation = millis();
            _modeEcran = MODE_CPT;
            _forcerMajEcran = true;
            resetFait = true;
            etatRemiseTotale = 2;
        }
    } else {
        if (boutonAppuye && !resetFait) {
             _tft.fillScreen(ST77XX_BLACK);
            _tft.setTextSize(2);
            _tft.setCursor(20, 50);
            _tft.setTextColor(ST77XX_RED);
            _tft.println("Annule...");
            _resetEnCours = true;
            _momentConfirmation = millis();
            _modeEcran = MODE_CPT;
            _forcerMajEcran = true;
        }
        boutonAppuye = false;
        resetFait = false;
        etatRemiseTotale = 0;
    }
}
