#include "BarController.h"
#include "affichage.h"
#include "serie.h"
#include <Wire.h>

// Pins TFT (Defined for VSPI)
#define TFT_SCLK 18
#define TFT_MOSI 23
#define TFT_CS 15
#define TFT_RST 13
#define TFT_DC 4

// Constantes
const unsigned long SEND_TIMEOUT = 500;
const unsigned long LED01_DURATION = 300;
const unsigned long TIMEOUT_CONNEXION = 5000;
#if TYPE_CPU == 3
const unsigned long DUREE_CLIGNO_ANNUL = 2000;
const unsigned long CLIGNO_DISCO_INTERVAL = 500;
#endif

const char *messages[] = {
  "Bienvenue !", "Profitez !", "Cheers !", "Happy Hour",
  "Flech ok !", "Bar ouvert", "Prenez ", "Ambiance"
};
const int messageCount = sizeof(messages) / sizeof(messages[0]);

BarController::BarController()
    : _tft(TFT_CS, TFT_DC, TFT_RST),
      _ee(0x50, I2C_DEVICESIZE_24LC32),
      _soundManager(26), // 26 is BUZZER_PIN (buz)
      _buttons{ezButton(Val_Pin_Env), ezButton(Val_Pin_Cre), ezButton(Val_Pin_Ann)},
      _etatActuel(ETAT_INIT),
      _setupTermine(false),
      _etapeSetup(0),
      _previousMillisSetup(0),
      _isConnected(false),
      _sendStatus(false),
      _ecranInitialise(false),
      _ok(255),
      _TotCn(0),
      _TotCn_prev(0),
      _cmptrjrn(0),
      _totcmptr(0),
      _screenHoldUntil(0),
      _sendStartTime(0),
      _isSending(false),
      _scrollY(-20),
      _messageScrolling(false),
      _previousMillisScroll(0),
      _currentMsg(""),
      _led01(25),
      _led01On(false),
      _led01StartTime(0),
      _previousConnectionCheck(0),
      _previousConnectionState(false)
{
    // Initialize specific CPU 3 variables
    #if TYPE_CPU == 3
    _ledClignotementActif = false;
    _ledClignoStart = 0;
    _clignotementAnnulationActif = false;
    _clignoAnnulStart = 0;
    _annulationCreditExces = false;
    _annulationCreditReset = false;
    _previousClignoDiscoMillis = 0;
    _ledRedDiscoState = false;
    #endif
}

void BarController::setup() {
    setupNonBloquant();
}

void BarController::initialiserModules() {
    _tft.initR(INITR_GREENTAB);
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setRotation(3);

    _soundManager.init();
    // jouerSon(4); -> SOUND_STARTUP
}

void BarController::setupNonBloquant() {
    unsigned long currentMillis = millis();

    switch (_etapeSetup) {
        case 0:
            pinMode(_led01, OUTPUT);

#if TYPE_CPU == 3
            pinMode(LdRed1, OUTPUT);
            pinMode(LdBlu1, OUTPUT);
            digitalWrite(LdRed1, HIGH);  // Allume la LED titre rouge
            digitalWrite(LdBlu1, HIGH);  // Allume la LED titre bleue
#endif

            for (int i = 0; i < BUTTON_COUNT; i++) {
                _buttons[i].setDebounceTime(30);
                _buttons[i].setCountMode(COUNT_FALLING);
            }

            initialiserModules();
            // _soundManager.play(SOUND_STARTUP); // Done inside initialiserModules call in original? No, it was in setup.cpp
            _soundManager.play(SOUND_STARTUP);

            Serial.begin(115200);

            WiFi.mode(WIFI_STA);
            WiFi.disconnect();

            _ee.begin();
            Wire.begin(21, 22);

            _previousMillisSetup = currentMillis;
            _etapeSetup++;
            break;

        case 1:
            if (currentMillis - _previousMillisSetup >= 1500) {
                afficherNumSerie(_tft);
                _soundManager.play(SOUND_BIP);
                _previousMillisSetup = currentMillis;
                _etapeSetup++;
            }
            break;

        case 2:
             // Attente de 6 secondes (remplace delay(6000) de serie.cpp)
            if (currentMillis - _previousMillisSetup >= 6000) {
                afficherStart(_tft);
                _previousMillisSetup = currentMillis;
                _etapeSetup++;
            }
            break;

        case 3:
            if (currentMillis - _previousMillisSetup >= 4000) {
                _etapeSetup++;
            }
            break;

        case 4:
            // Init Network
            NetworkManager::getInstance().setOnReceiveCallback(
                [this](const struct_message& data, const uint8_t* mac){ this->onDataReceived(data, mac); }
            );
            NetworkManager::getInstance().setOnSendCallback(
                [this](bool success){ this->onDataSent(success); }
            );

            NetworkManager::getInstance().init(broadcastAddress);

            _dataSent.cp1 = 0;
            _dataSent.cp2 = _cmptrjrn;
            _dataSent.cp3 = _totcmptr;
            _dataSent.fs1 = 1;
            _dataSent.fp1 = false;

            if (NetworkManager::getInstance().sendData(_dataSent)) {
                // OK
            } else {
                _tft.fillScreen(ST77XX_RED);
                _tft.setTextSize(2);
                _tft.setCursor(10, 40);
                _tft.println("   Jeu\n\n OFF line");
            }

            _setupTermine = true;
            _etatActuel = ETAT_INIT;
            _messageScrolling = false;
            _scrollY = -20;
            break;
    }
}

void BarController::onDataReceived(const struct_message& data, const uint8_t* mac) {
    memcpy(&_dataRcvr, &data, sizeof(struct_message));
    _previousConnectionCheck = millis();

    _cmptrjrn = _dataRcvr.cp2;
    _totcmptr = _dataRcvr.cp3;

    // Toggle LED02 logic omitted as LED02 pin var not fully tracked in main.cpp snippets effectively
}

void BarController::onDataSent(bool success) {
    _sendStatus = success;
    _isConnected = success;

    digitalWrite(_led01, HIGH);
    _led01On = true;
    _led01StartTime = millis();
}

void BarController::checkConnection() {
    if (millis() - _previousConnectionCheck > TIMEOUT_CONNEXION) {
        _isConnected = false;
    } else {
        _isConnected = true;
    }
}

void BarController::loop() {
    static unsigned long previousLoopMillis = 0;
    const unsigned long loopInterval = 100;

    unsigned long currentMillis = millis();

    if (currentMillis - previousLoopMillis < loopInterval) return;
    previousLoopMillis = currentMillis;

    if (!_setupTermine) {
        setupNonBloquant();
    } else {
        _soundManager.update();

        for (int i = 0; i < 3; i++) _buttons[i].loop();

        checkConnection();

#if TYPE_CPU == 3
        gererLEDTransfert(_sendStatus);
        gererEtatsLEDs();
#endif

        if (_isConnected != _previousConnectionState) {
            _previousConnectionState = _isConnected;

            if (!_isConnected && _TotCn > 0) {
                _TotCn = 0;
                #if TYPE_CPU == 3
                _annulationCreditExces = false;
                #endif

                _ok = 8;
                _soundManager.play(SOUND_CRITICAL_ERROR);
                ecran();
                _etatActuel = ETAT_TIMEOUT_ECRAN;
                _ecranInitialise = false;
                _screenHoldUntil = currentMillis + 4000;
            }
             else if (_etatActuel != ETAT_TIMEOUT_ECRAN) { // Fixed logic
                 _ok = 1;
                 _ecranInitialise = false;
                 ecran();
             }
        }

        // Button Logic
        // Note: ezButton pressed state is transient.
        if (_buttons[1].isPressed()) { // Credit
            if (_etatActuel == ETAT_ATTENTE) {
                if (!_isConnected) {
                    _soundManager.play(SOUND_CRITICAL_ERROR);
                    _ok = 9;
                    ecran();
                    _screenHoldUntil = currentMillis + 3000;
                    _etatActuel = ETAT_TIMEOUT_ECRAN;
                } else {
                    _TotCn += Val_TotCn;
                    _soundManager.play(SOUND_BIP);
                    _ok = 3;
                    ecran();
                    _screenHoldUntil = currentMillis + 100;
                }
            }
        }

        // Logic for BUTTON 1 (Env) and 3 (Ann)
        // Check if pressed logic is needed inside ATTENTE
        if (_etatActuel == ETAT_ATTENTE) {
             if ((long)(currentMillis - _screenHoldUntil) > 0 && _TotCn == 0) {
                 _ok = 1;
                 ecran();
             }

             if (_buttons[0].isPressed()) { // Env
                  _soundManager.play(SOUND_BIP);
                  _etatActuel = (_TotCn == 0) ? ETAT_COMPTEURS : ETAT_ANNULATION;
             }

             if (_buttons[2].isPressed()) { // Ann
                  _soundManager.play(SOUND_BUTTON);

                  if (_TotCn > max_cred) {
                    #if TYPE_CPU == 3
                    _annulationCreditExces = true;
                    #endif
                    _TotCn = 0;
                    _ok = 8;
                    ecran();
                    _soundManager.play(SOUND_CRITICAL_ERROR);
                    _screenHoldUntil = currentMillis + 4000;
                    _ecranInitialise = false;
                    _etatActuel = ETAT_TIMEOUT_ECRAN;
                  } else if (_TotCn > 0) {
                    _etatActuel = ETAT_ENVOI_CREDIT;
                  } else {
                    _etatActuel = ETAT_PUB;
                  }
             }
        }

        switch (_etatActuel) {

            case ETAT_INIT:
                _ok = 1;
                ecran();
                _etatActuel = ETAT_ATTENTE;
                break;

            case ETAT_ATTENTE:
                 // Handled above
                break;

            case ETAT_ANNULATION:
                #if TYPE_CPU == 3
                _annulationCreditExces = false;
                _annulationCreditReset = true;
                #endif
                _ok = 8;
                _TotCn = 0;
                _soundManager.play(SOUND_ERROR);
                ecran();
                _screenHoldUntil = currentMillis + 4000;
                _etatActuel = ETAT_TIMEOUT_ECRAN;
                break;

            case ETAT_ENVOI_CREDIT:
                _dataSent.cp1 = _TotCn;
                _dataSent.fs1 = 1;
                _dataSent.fp1 = false;
                NetworkManager::getInstance().sendData(_dataSent);
                _isSending = true;
                _sendStartTime = currentMillis;
                _etatActuel = ETAT_RESULTAT_ENVOI;
                break;

            case ETAT_RESULTAT_ENVOI:
                if ((long)(currentMillis - _sendStartTime) >= SEND_TIMEOUT) {
                    _isSending = false;

                    if (_sendStatus) {
                        _tft.fillScreen(ST77XX_BLACK);
                        _tft.setTextColor(ST77XX_WHITE);
                        _tft.setTextSize(2);
                        _tft.setCursor(10, 40);
                        _tft.println("Transfert");
                        _tft.println(" reussi");
                        _soundManager.play(SOUND_SUCCESS);
                        _screenHoldUntil = currentMillis + 1000;
                    } else {
                        _tft.fillScreen(ST77XX_RED);
                        _tft.setTextSize(2);
                        _tft.setCursor(10, 40);
                        _tft.println("   Jeu");
                        _tft.println(" ");
                        _tft.println(" OFF line");
                        _soundManager.play(SOUND_CRITICAL_ERROR);
                        _screenHoldUntil = currentMillis + 3000;
                        _ecranInitialise = false;
                    }

                    _TotCn = 0;
                    _etatActuel = ETAT_TIMEOUT_ECRAN;
                }
                break;

            case ETAT_PUB:
                _ok = 4;
                ecran();
                _screenHoldUntil = currentMillis + 3000;
                _etatActuel = ETAT_TIMEOUT_ECRAN;
                break;

            case ETAT_COMPTEURS:
                _ok = 6;
                ecran();
                _screenHoldUntil = currentMillis + 3000;
                _etatActuel = ETAT_TIMEOUT_ECRAN;
                break;

            case ETAT_TIMEOUT_ECRAN:
                if ((long)(currentMillis - _screenHoldUntil) > 0 &&
                    (!_buttons[1].getState() == LOW || (currentMillis - _screenHoldUntil) > 5000)) {

                    if (_TotCn == 0) _ok = 1;
                    ecran();
                    for (int i = 0; i < 3; i++) _buttons[i].resetCount(); // Optional
                    _etatActuel = ETAT_ATTENTE;
                    _ecranInitialise = false;
                }
                break;

             default: break;
        }

        if (_etatActuel == ETAT_ATTENTE && _TotCn != _TotCn_prev) {
            _ok = (_TotCn > 0) ? 3 : 1;
            ecran();
            _screenHoldUntil = currentMillis + ((_TotCn > 0) ? 700 : 0);
            _TotCn_prev = _TotCn;
        }

        if (_led01On && currentMillis - _led01StartTime >= LED01_DURATION) {
            digitalWrite(_led01, LOW);
            _led01On = false;
        }
    }
}

void BarController::ecran() {
    switch (_ok) {
        case 1:
            // "Bar onLine" / Scrolling
            checkConnection(); // Ensure connected state is fresh
            if (_isConnected) {
                if (!_ecranInitialise) {
                    _tft.fillScreen(ST77XX_BLUE);  // Fond bleu
                    _tft.fillRect(0, 100, _tft.width(), 40, ST77XX_BLUE);
                    _tft.setTextColor(ST77XX_YELLOW);
                    _tft.setCursor(10, 110);
                    _tft.println("Bar onLine");
                    _ecranInitialise = true;
                }

                unsigned long now = millis();
                if (!_messageScrolling) {
                    _scrollY = 0;
                    _currentMsg = messages[random(0, messageCount)];
                    _messageScrolling = true;
                    _previousMillisScroll = now;
                }

                if (now - _previousMillisScroll >= 50) {
                    _previousMillisScroll = now;

                    _tft.fillRect(0, 0, _tft.width(), 100, ST77XX_BLUE);

                    _tft.setCursor(10, _scrollY);
                    _tft.setTextColor(ST77XX_WHITE);
                    _tft.println(_currentMsg);
                    _scrollY += 4;

                    if (_scrollY > 80) {
                        _messageScrolling = false;
                        _tft.fillRect(0, 0, _tft.width(), 100, ST77XX_BLUE);
                    }
                }
            }
            if (!_isConnected) {
                if (!_ecranInitialise) {
                    _tft.fillScreen(ST77XX_RED);
                    _tft.setTextColor(ST77XX_YELLOW);
                    _tft.setCursor(10, 60);
                    _tft.println("Jeux off");
                    _ecranInitialise = true;
                    _messageScrolling = false;
                }
            }
            break;

        case 3:
            _tft.fillScreen(ST77XX_BLACK);
            _messageScrolling = false;
            _scrollY = 0;
            _tft.fillScreen(ST77XX_BLACK);
            _tft.setCursor(3, 6);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.println("Cred/jeux ");
            _tft.setCursor(41, 34);
            _tft.print(_TotCn * Val_Cred);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.setCursor(10, 60);
            _tft.print(" En Euro:");
            _tft.setTextSize(5);
            _tft.setCursor(40, 85);
            _tft.print(_TotCn);
            _tft.setTextSize(2);
            break;

        case 4: {
            _messageScrolling = false;
            _scrollY = 0;
            _tft.fillScreen(ST77XX_BLUE);

            int y = 5;
            int hauteurTexte = 8;
            int interligne = 1;

            for (int i = 0; i < nbLignes; i++) {
                _tft.setTextColor(lignes[i].couleur);
                _tft.setTextSize(lignes[i].tailleTexte);
                _tft.setCursor(0, y);
                _tft.print(lignes[i].texte);
                y += hauteurTexte * lignes[i].tailleTexte + interligne;
            }
            _tft.setTextSize(2);
            }
            break;

        case 6:
            _messageScrolling = false;
            _scrollY = 0;
            _tft.fillScreen(ST77XX_BLACK);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.setCursor(2, 5);
            _tft.println("Journalier");
            _tft.setCursor(10, 30);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.println(_cmptrjrn);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.setCursor(5, 55);
            _tft.println("Total");
            _tft.setTextColor(ST77XX_WHITE);
            _tft.setCursor(10, 82);
            _tft.println(_totcmptr);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.setTextSize(1);
            _tft.setCursor(10, 110);
            _tft.println("En Euros");
            _tft.setTextSize(2);
            break;

        case 8:
            _messageScrolling = false;
            _scrollY = 0;
            _tft.fillScreen(ST77XX_RED);
            _tft.setCursor(10, 2);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.print("Operation");
            _tft.setTextSize(4);
            _tft.setCursor(48, 25);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.print(_TotCn);
            _tft.setTextSize(2);
            _tft.setCursor(10, 59);
            #if TYPE_CPU == 3
            if (_annulationCreditExces) {
                _tft.setTextColor(ST77XX_WHITE);
                _tft.println(("Euro > ") + String(max_cred));
            }
            #endif
            _tft.setCursor(10, 83);
            _tft.println("Credits");
            _tft.setCursor(10, 103);
            _tft.println("annules !");
            break;

        case 9:
            _messageScrolling = false;
            _scrollY = 0;
            _tft.fillScreen(ST77XX_RED);
            _tft.setTextColor(ST77XX_WHITE);
            _tft.setCursor(0, 20);
            _tft.setTextSize(2);
            _tft.println("Jeu distant");
            _tft.setCursor(25, 45);
            _tft.println("eteint");
            _tft.setCursor(10, 80);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.println("Credit");
            _tft.setCursor(10, 100);
            _tft.println("desactive");
            _tft.setTextSize(2);
            break;

        default:
            _ecranInitialise = false;
            _messageScrolling = false;
            _scrollY = 0;
            break;
    }
}

#if TYPE_CPU == 3

void BarController::gererLEDTransfert(bool transfertOK) {
  static unsigned long previousClignoMillis = 0;
  static bool ledState = false;

  if (_isSending) {
    _ledClignotementActif = true;
    _ledClignoStart = millis();
    previousClignoMillis = 0;
  }

  if (_ledClignotementActif) {
    unsigned long currentMillis = millis();

    unsigned long dureeCligno = transfertOK ? 2500 : 1500;
    unsigned long vitesseCligno = transfertOK ? 120 : 250;

    if (currentMillis - previousClignoMillis >= vitesseCligno) {
      previousClignoMillis = currentMillis;
      ledState = !ledState;

      if (transfertOK) {
        digitalWrite(LdBlu1, ledState);
        digitalWrite(LdRed1, LOW);
      } else {
        digitalWrite(LdRed1, ledState);
        digitalWrite(LdBlu1, LOW);
      }
    }

    if (currentMillis - _ledClignoStart > dureeCligno) {
      _ledClignotementActif = false;
      digitalWrite(LdBlu1, LOW);
      digitalWrite(LdRed1, LOW);
    }
  }
}


void BarController::gererEtatsLEDs() {
  if (_ledClignotementActif) return;

  unsigned long now = millis();

  // === Gestion du clignotement en cas de dépassement crédit ===
  if ((_annulationCreditExces || _annulationCreditReset) && !_clignotementAnnulationActif) {
    _clignotementAnnulationActif = true;
    _clignoAnnulStart = now;
  }

  if (_clignotementAnnulationActif) {
    digitalWrite(LdRed1, (now / 200) % 2);
    digitalWrite(LdBlu1, LOW);

    if (now - _clignoAnnulStart >= DUREE_CLIGNO_ANNUL) {
      _clignotementAnnulationActif = false;
      _annulationCreditExces = false;
      _annulationCreditReset = false;
      digitalWrite(LdRed1, LOW);
    }
    return;
  }

  // === 🔴 Clignotement si déconnecté ===
  if (!_isConnected) {
    if (now - _previousClignoDiscoMillis >= CLIGNO_DISCO_INTERVAL) {
      _previousClignoDiscoMillis = now;
      _ledRedDiscoState = !_ledRedDiscoState;
      digitalWrite(LdRed1, _ledRedDiscoState);
    }
    digitalWrite(LdBlu1, LOW);
    return;
  }

  // === État normal si connecté ===
  digitalWrite(LdRed1, LOW);
  digitalWrite(LdBlu1, (_TotCn == 0) ? HIGH : LOW);
}

#endif
