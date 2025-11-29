/* Flechette10.ino
 * voir version.cpp
 * Correction librairie Ardafruit expressif  et certaines librairies
 
  C:\Users\user\Documents\Arduino
  c:\Users\user\Documents\Arduino\libraries\esp_now
 */

/*
 * Flechette07.ino
 * Voir version.cpp pour le suivi des versions.
 * Correction des bibliothèques Adafruit, Espressif et autres dépendances.
 * 
 * Répertoire projet :
 *   C:\Users\user\Documents\Arduino
 * Bibliothèque ESP-NOW :
 *   c:\Users\user\Documents\Arduino\libraries\esp_now
 */

#include <Arduino.h>
#include "son.h"
#include "serie.h"
#include "affichage.h"
#include "setup.h"
#include "config.h"
#include <utils.h>
#include <I2C_eeprom.h>
#include <Wire.h>             // Bibliothèque nécessaire pour I2C
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST77xx.h>  // Hardware-specific library
#include <Adafruit_ST7735.h>  // Hardware-specific library
#include <ezButton.h>
#include <SPI.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneWire.h>
#include <Preferences.h>
#include <PCF8574.h>
#include <DS1307new.h>

#define DS1307_ID 0x68

uint16_t result = 0;
 
#define PCF_ADDRESS 0x20
//#define PCF_ADDRESS 0x20
//#define PCF_ADDRESS 0x38 //pcf 8574 A

PCF8574 pcf8574(PCF_ADDRESS);
uint8_t pcf_address_definie = PCF_ADDRESS;

bool etatLed01 = false;
uint8_t led01 = 19;
uint8_t Nvr1, Nvr2;

#if ESP_IDF_VERSION_MAJOR >= 5
  void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status)
#else
  void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
#endif

{
  result = !result;
  pcf8574.write(0, result);

  if (status != ESP_NOW_SEND_SUCCESS) {
   // Serial.println("Delivery Fail");
  }

  pcf8574.write(1, status);

  // Allumer ou éteindre la LED01 et mémoriser son état
  etatLed01 = status;
  digitalWrite(led01, etatLed01);
}

// =====================================================
// 🔹 Fonctions utilitaires I2C à placer avant setup()
// =====================================================

void resetI2C() {
  Wire.end();
  delay(50);
  Wire.begin(SDA, SCL);
  delay(50);
}

bool waitForRTC() {
  for (int i = 0; i < 15; i++) {  // essaie pendant environ 3 secondes
    Wire.beginTransmission(0x68); // adresse du DS1307 / DS3231
    byte err = Wire.endTransmission();
    if (err == 0) return true;    // RTC présent
    delay(200);
  }
  return false; // échec
}

// =====================================================
// 🔹 Vérifie et initialise la NVRAM du RTC
// =====================================================
bool verifierPileRTC(Adafruit_ST7735 &tft) {
  uint8_t Nvr1, Nvr2;

  // Lecture des deux octets de signature
  RTC.getRAM(0, (uint8_t *)&Nvr1, 1);
  RTC.getRAM(55, (uint8_t *)&Nvr2, 1);
  delay(100);

  if (Nvr1 == 0x5A && Nvr2 == 0xA5) {
    // ✅ Pile et NVRAM valides
    tft.setTextColor(ST77XX_GREEN);
    tft.println("NvRam  OK");
    tft.println("Pile   OK");
    return true;
  } else {
    // ⚠️ Données corrompues → réinitialisation NVRAM
    tft.setTextColor(ST77XX_RED);
    tft.println("NvRam  NOT OK");
    tft.println("Pile   NOT OK");

    // Effacement complet
    uint8_t zero = 0;
    for (int i = 0; i < 56; i++) {
      RTC.setRAM(i, (uint8_t *)&zero, 1);
    }

    // Nouvelle signature
    Nvr1 = 0x5A;
    Nvr2 = 0xA5;
    RTC.setRAM(0, (uint8_t *)&Nvr1, 1);
    RTC.setRAM(55, (uint8_t *)&Nvr2, 1);

    delay(100);
    return false;
  }
}







// ********************************************************************************
void sequenceFinaleSetup(unsigned long temps) {
  //Serial.println("Appel de sequenceFinaleSetup");
}


//void sequenceFinaleSetup(unsigned long currentMillis);

I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC32);

#define TFT_SCLK 18  // SPI clock
#define TFT_MOSI 23  // SPI Data
#define TFT_CS 15    // Display enable (Chip select)
#define TFT_RST 13
#define TFT_DC 4  // register select

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

bool pcf8574Present = false;

unsigned long previousMillisLoop = 0;

bool affichageConfirmation = false;

bool resetEnCours = false;
unsigned long momentConfirmation = 0;


unsigned long dernierAffichageCpt = 0;
const unsigned long intervalleAffichageCpt = 1000;  // 50 ms

bool pubEnCours = false;
unsigned long pubStartTime = 0;
const unsigned long dureeAffichagePub = 3000;  // 3 secondes


unsigned long resetStartTime = 0;
const unsigned long dureeAffichageReset = 3000;  // 3 secondes

bool affichageResetFait = false;

const unsigned long delaiEntreImpulsions = 1000;  // ← délai entre deux impulsions (en ms)
unsigned long dernierCreditEnvoye = 0;            // ← chronomètre de la dernière impulsion

const unsigned long dureeImpulsion = 1;  // Durée impulsion relais
// === Variables ===
bool relaisEnCours = false;
unsigned long relaisStartTime = 0;
unsigned int compteurCredits;


const uint8_t BUTTON_1_PIN = 32;
const uint8_t BUTTON_2_PIN = 35;
const uint8_t BUTTON_3_PIN = 34;
const uint8_t buz = 26;

// Ajouter en haut du fichier avec les autres variables globales :
bool lastStateBtn2 = LOW;
bool lastStateBtn3 = LOW;
bool forcerMajEcran = false;        // ← Réinitialisation cpt()
int modeAffichage = 0;              // 0 = normal, 2 = bouton2, 3 = bouton3
bool premierAffichageFait = false;  // Pour forcer cpt() au tout début
void cpt();
void afficherRemiseAZero();
void afficherPublicite();


unsigned long dernierAppui = 0;
const unsigned long debounceDelay = 200;  // 200 ms pour l’anti-rebond

enum ModeEcran { MODE_CPT,
                 MODE_BOUTON1,
                 MODE_BOUTON2,
                 MODE_BOUTON3 };

ModeEcran modeEcran = MODE_CPT;

int etatRemiseTotale = 0;  // 0 = attente, 1 = affichage fait, 2 = reset fait

int prev_Mcmptr1 = -1;
int prev_cmptrjrn = -1;
int prev_totcmptr = -1;
int prev_compteurCredits = -1;


bool connecte;
uint8_t rel01 = 25;
uint8_t rel02 = 27;

bool ledState1 = LOW;
//uint16_t Mcmptr1, Mcmptr2, Mcmptr3;
uint16_t Mcmptr1, Mcmptr2, Mcmptr3;


uint16_t cmptr01 = 0;
bool Mfp1, Mfs1;
uint16_t cmptrjrn, totcmptr;

uint16_t EnvTot, Read_EnvTot;
bool SendStts;

uint8_t address = 1;
uint8_t nDevices = 0;
bool setupTermine = false;
uint8_t etapeSetup = 0;
unsigned long previousMillisSetup = 0;

ezButton buttonArray[] = {
  ezButton(BUTTON_1_PIN),
  ezButton(BUTTON_2_PIN),
  ezButton(BUTTON_3_PIN),
};

uint16_t TimeIsSet;
uint8_t Echantillon_ms_precedent = 0;
uint16_t duree = 0;
uint16_t compteur = 0;


// *****************************111111111111
// À définir dans ton fichier principal ou ailleurs :
//extern uint8_t broadcastAddress[]; // Adresse MAC du peer (ESP-NOW)
String getMacString(const uint8_t *mac);
void debugPrint(const String &msg);

// *******************1111111***************************


//void sequenceFinaleSetup(unsigned long currentMillis);


// uint8_t broadcastAddress[] = {0x5C, 0x01, 0x3B, 0x6A, 0x12, 0x64};  // Adresse du peer
esp_now_peer_info_t peerInfo;  // Déclare peerInfo

uint16_t arr1[2];  // Déclare arr1 pour stocker les compteurs

// Structure pour contenir les données reçues
typedef struct struct_message {
  uint16_t cp1;  // Incrément compteur
  uint16_t cp2;  // Compteur journalier
  uint16_t cp3;  // Compteur total
  bool fp1;
  bool fs1;
} struct_message;

struct_message dataSent, dataRcvr;

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
  memcpy(&dataRcvr, incomingData, sizeof(dataRcvr));
  Mfp1 = 0;

  // Si on reçoit des crédits (cp1 > 0), on remet compteurCredits à zéro
  if ((dataRcvr.cp1 > 0) && (Mcmptr1 == 0)) {
    compteurCredits = 0;
    //Serial.println("Réception de crédits : compteurCredits remis à zéro.");
  }

  Mcmptr1 += dataRcvr.cp1;  // ⬅️ on ajoute au lieu d’écraser
  Mcmptr2 = dataRcvr.cp2;
  Mcmptr3 = dataRcvr.cp3;
  Mfp1 = dataRcvr.fp1;
  Mfs1 = dataRcvr.fs1;
  //Serial.println(dataRcvr.cp1);
  //Serial.print("Bool: ");
  //Serial.print(dataRcvr.fp1);
  //Serial.print("; ");
  //Serial.println(dataRcvr.fs1);
  memset(&dataRcvr, 0, sizeof(dataRcvr));
}

void writeToNVram(void) {
  RTC.setRAM( 5, (uint8_t *)&cmptrjrn, sizeof(uint16_t));
  RTC.setRAM( 7, (uint8_t *)&totcmptr, sizeof(uint16_t));
  RTC.setRAM( 9, (uint8_t *)&Mcmptr1, sizeof(uint16_t));
  RTC.setRAM(16, (uint8_t *)&compteurCredits, sizeof(uint16_t));
  //delay(100);  // Code pour écrire dans la NVRAM (ou EEPROM)
}

void readFromNVram(void) {
  RTC.getRAM( 5, (uint8_t *)&cmptrjrn, sizeof(uint16_t));
  RTC.getRAM( 7, (uint8_t *)&totcmptr, sizeof(uint16_t));
  RTC.getRAM( 9, (uint8_t *)&Mcmptr1, sizeof(uint16_t));
  RTC.getRAM(16, (uint8_t *)&compteurCredits, sizeof(uint16_t));
  //delay(100);  // Code pour lire depuis la NVRAM (ou EEPROM)
}

void SendCmptr(void) {
  dataSent.cp1 = cmptr01;
  dataSent.cp2 = cmptrjrn;
  dataSent.cp3 = totcmptr;
  esp_now_send(broadcastAddress, (uint8_t *)&dataSent, sizeof(dataSent));
  dataSent.fs1 = 0;
}

//********************************************************************************

void declencherImpulsionRelais();

void ajouterCredit() {
  compteurCredits++;
  //Serial.print("Crédit ajouté. Total = ");
  //Serial.println(compteurCredits);
  declencherImpulsionRelais();
}

void declencherImpulsionRelais() {
  if (relaisEnCours) return;  // Empêche de relancer l’impulsion si en cours

  //Serial.println("Relais ACTIVÉ");
  digitalWrite(rel02, HIGH);  // Active relais (ou LOW si relais actif à LOW)
  relaisStartTime = millis();
  relaisEnCours = true;
}

void gestionRelais() {
  if (relaisEnCours && millis() - relaisStartTime >= dureeImpulsion) {
    //Serial.println("Relais DÉSACTIVÉ");
    digitalWrite(rel02, LOW);  // Désactive relais (ou HIGH si relais actif à LOW)
    relaisEnCours = false;
  }
}

void lancerPublicite() {
  // Cette fonction ne fait que démarrer la pub, pas la gérer
  if (!pubEnCours) {
    afficherPublicite();      // Affiche la publicité une fois
    pubStartTime = millis();  // Démarre le chrono
    pubEnCours = true;
  }
}

void verifierFinPublicite() {
  if (pubEnCours && millis() - pubStartTime >= dureeAffichagePub) {
    pubEnCours = false;
    modeEcran = MODE_CPT;
    forcerMajEcran = true;
  }
}


void gererAppuiLongRemiseJournalier() {
  static bool boutonAppuye = false;
  static unsigned long debutAppui = 0;
  static unsigned long dernierBip = 0;
  static bool resetFait = false;
  const unsigned long dureeAppuiNecessaire = 3000;

  if (digitalRead(BUTTON_3_PIN) == LOW) {
    if (!boutonAppuye) {
      boutonAppuye = true;
      debutAppui = millis();
      dernierBip = 0;
      resetFait = false;

      tft.fillScreen(ST77XX_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(10, 30);
      tft.println("Clear Temp");
    }

    unsigned long maintenant = millis();
    unsigned long dureeAppui = maintenant - debutAppui;

    if (!resetFait) {
      if (maintenant - dernierBip >= 500) {
        jouerSon(0);
        dernierBip = maintenant;
      }

      // Affichage de la barre de progression UNIQUEMENT si reset non fait
      int largeurTotale = 120;
      int hauteurBarre = 10;
      int posX = 10;
      int posY = 70;
      int largeurRemplie = map(dureeAppui, 0, dureeAppuiNecessaire, 0, largeurTotale);

      tft.drawRect(posX, posY, largeurTotale, hauteurBarre, ST77XX_WHITE);
      tft.fillRect(posX + 1, posY + 1, largeurRemplie, hauteurBarre - 2, ST77XX_GREEN);
    }

    if (dureeAppui >= dureeAppuiNecessaire && !resetFait) {
      cmptrjrn = 0;
      cmptr01 = 0;

      writeToNVram();
      dataSent.cp2 = cmptrjrn;
      esp_now_send(broadcastAddress, (uint8_t *)&dataSent, sizeof(dataSent));
      dataSent.fs1 = 0;

      tft.fillScreen(ST77XX_GREEN);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(10, 40);
      tft.println("Compteur");
      tft.setCursor(10, 55);
      tft.println("journalier");
      tft.setCursor(10, 70);
      tft.println("remis a");
      tft.setCursor(10, 85);
      tft.println("zero");

      jouerSon(2);

      resetEnCours = true;
      momentConfirmation = millis();

      modeEcran = MODE_CPT;
      forcerMajEcran = true;

      resetFait = true;
    }

  } else {
    if (boutonAppuye && !resetFait) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextSize(2);
      tft.setCursor(20, 50);
      tft.setTextColor(ST77XX_RED);
      tft.println("Annule...");
      resetEnCours = true;
      momentConfirmation = millis();
      modeEcran = MODE_CPT;
      forcerMajEcran = true;
    }
    boutonAppuye = false;
    resetFait = false;
  }
}




void gererAppuiLongRemiseTotaux() {
  static bool boutonAppuye = false;
  static unsigned long debutAppui = 0;
  static unsigned long dernierBip = 0;
  static bool resetFait = false;
  const unsigned long dureeAppuiNecessaire = 3000;

  if (digitalRead(BUTTON_1_PIN) == LOW) {
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
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(10, 30);
      tft.println("Clear Cpts");
      etatRemiseTotale = 1;
    }

    if (!resetFait) {
      // Bips pendant l'appui long
      if (maintenant - dernierBip >= 500) {
        jouerSon(0);
        dernierBip = maintenant;
      }

      // Affichage de la barre de progression
      int largeurTotale = 120;
      int hauteurBarre = 10;
      int posX = 10;
      int posY = 70;
      int largeurRemplie = map(dureeAppui, 0, dureeAppuiNecessaire, 0, largeurTotale);

      tft.drawRect(posX, posY, largeurTotale, hauteurBarre, ST77XX_WHITE);
      tft.fillRect(posX + 1, posY + 1, largeurRemplie, hauteurBarre - 2, ST77XX_ORANGE);
    }

    if (dureeAppui >= dureeAppuiNecessaire && !resetFait) {
      cmptrjrn = 0;
      cmptr01 = 0;
      totcmptr = 0;

      writeToNVram();
      dataSent.cp2 = cmptrjrn;
      dataSent.cp3 = totcmptr;
      esp_now_send(broadcastAddress, (uint8_t *)&dataSent, sizeof(dataSent));
      dataSent.fs1 = 0;

      tft.fillScreen(ST77XX_ORANGE);
      tft.setTextSize(2);
      tft.setTextColor(ST77XX_BLACK);
      tft.setCursor(20, 40);
      tft.println("Tout les");
      tft.setCursor(20, 55);
      tft.println("compteurs");
      tft.setCursor(20, 70);
      tft.println("remis a");
      tft.setCursor(20, 85);
      tft.println("zero");

      jouerSon(2);

      resetEnCours = true;
      momentConfirmation = millis();

      modeEcran = MODE_CPT;
      forcerMajEcran = true;

      resetFait = true;
      etatRemiseTotale = 2;
    }

  } else {
    if (boutonAppuye && !resetFait) {
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextSize(2);
      tft.setCursor(20, 50);
      tft.setTextColor(ST77XX_RED);
      tft.println("Annule...");
      resetEnCours = true;
      momentConfirmation = millis();
      modeEcran = MODE_CPT;
      forcerMajEcran = true;
    }

    boutonAppuye = false;
    resetFait = false;
    etatRemiseTotale = 0;
  }
}


// === Constantes de délai ===
const unsigned long DELAI_I2C_SCAN     = 1500;
const unsigned long DELAI_FIN_SCAN     = 100;
const unsigned long DELAI_FIN_Nvram    = 1500;
const unsigned long DELAI_FIN_Peer     = 2000;
const unsigned long DELAI_FINAL_STEP1  = 500;
const unsigned long DELAI_FINAL_STEP2  = 3000;
const unsigned long DELAI_SON1         = 1000;
const unsigned long DELAI_RELAIS_ON    = 5000;
const unsigned long DELAI_RELAIS_OFF   = 1800;
const unsigned long DELAI_START_SCREEN = 5000;
const unsigned long DELAI_FIN_SETUP    = 2000;
const unsigned long intervalLoop       = 100;  // ralentit à ~100 fois/sec

void setupNonBloquant();

void setup() {
  setupNonBloquant();
}









void setupNonBloquant() {
  unsigned long currentMillis = millis();

  static bool rtcPresent = true;
 
 
  switch (etapeSetup) {
    case 0:
      // Initialisation des E/S et modules
      pinMode(BUTTON_1_PIN, INPUT_PULLUP);
      pinMode(BUTTON_2_PIN, INPUT_PULLUP);
      pinMode(BUTTON_3_PIN, INPUT_PULLUP);
      initialiserSon();
      initialiserModules();

      Serial.begin(115200);

      tft.initR(INITR_GREENTAB);
      tft.fillScreen(ST77XX_BLACK);
      tft.setRotation(3);

      Wire.begin(21, 22);
      WiFi.mode(WIFI_STA);
      ee.begin();

      afficherNum();
      previousMillisSetup = currentMillis;
      etapeSetup++;
      break;

    case 1:
      if (currentMillis - previousMillisSetup >= DELAI_I2C_SCAN) {
        afficherNumSerie(tft);
        afficherSetup();
        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 2:
      if (currentMillis - previousMillisSetup >= DELAI_I2C_SCAN) {
        tft.setTextSize(2);
        tft.fillScreen(ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(ST77XX_ORANGE);
        tft.println("Scan I2C...");
        tft.setTextColor(ST77XX_WHITE);
        address = 1;
        nDevices = 0;
        etapeSetup++;
        previousMillisSetup = currentMillis;
      }
      break;


    case 3:
      if (currentMillis - previousMillisSetup >= 100) {  // ← ajout d'un petit délai entre chaque adresse
        if (address < 127) {
          Wire.beginTransmission(address);
          byte error = Wire.endTransmission();
          if (error == 0) {
            tft.setTextSize(2);
            tft.setTextColor(ST77XX_WHITE);
            tft.print("I2C  >0x");
            tft.println(address, HEX);
            jouerSon(1);
            nDevices++;
          }
          address++;
          previousMillisSetup = currentMillis;  // ← MAJ du temps ici
        } else {
          etapeSetup++;
          previousMillisSetup = currentMillis;
        }
      }
      break;


    case 4:
      rtcPresent = RTC.isPresent();
      if (!rtcPresent) {
        //tft.fillScreen(ST77XX_RED);
        //tft.setCursor(0, 40);
        tft.setTextColor(ST77XX_RED);
        tft.println("");
        tft.println("HW111> OFF");
        tft.setTextColor(ST77XX_WHITE);
        tft.println("");
      } else {
        tft.println("HW111> OK");
      }
      previousMillisSetup = currentMillis;
      etapeSetup++;
      break;











    case 5:
      if (currentMillis - previousMillisSetup >= DELAI_FIN_SCAN) {
        if (nDevices == 0) {
          tft.println("Er03 > I2C");
        } else {
          //tft.println("Scan > OK");
        }

        if (esp_now_init() != ESP_OK) {
          tft.println("Er01 > ESP-NOW");
          setupTermine = true;  // Abandon
          return;
        } else {
          tft.println("NOW  > Ok");
        }

        etapeSetup++;
        previousMillisSetup = currentMillis;
      }
      break;



 



    case 6:
      {
        // tft.fillScreen(ST77XX_BLACK);
        bool adresseCorrecte = false;
        bool pcfDetecte = false;
        uint8_t adresseTrouvee = 0;

        // Balayage des adresses I2C
        for (uint8_t addr = 0x20; addr <= 0x3F; addr++) {
          Wire.beginTransmission(addr);
          if (Wire.endTransmission() == 0) {
            // Test de lecture d'un registre du PCF8574 ?
            // Pas possible directement, donc on suppose que c'est un PCF8574
            pcfDetecte = true;
            adresseTrouvee = addr;
            break;  // Si tu veux prendre le premier trouvé
          }
        }

        if (pcfDetecte) {
          if (!rtcPresent) {
            tft.println("");
          }
          if (adresseTrouvee == PCF_ADDRESS) {
            pcf8574.begin(adresseTrouvee);
            tft.setTextColor(ST77XX_GREEN);
            tft.print("PCF  >0x");
            tft.println(adresseTrouvee, HEX);
            jouerSon(2);

          } else {
            // PCF 8574 pas sur adresse définie
            tft.setTextColor(ST77XX_RED);
            tft.print("PCFer>0x");
            tft.println(pcf_address_definie, HEX);
            jouerSon(2);
          }

        } else {
          tft.setTextColor(ST77XX_RED);
          tft.setTextSize(2);
          tft.println("PCF  >OFF");
          // PCF8574 Absent
          jouerSon(0);
        }

        // Blocage volontaire si erreur
        if (!pcfDetecte || adresseTrouvee != PCF_ADDRESS) {
          // while (1)  ;
            
        }
        previousMillisSetup = currentMillis;
        etapeSetup++;
        break;
      }





    case 7:
  if (currentMillis - previousMillisSetup >= DELAI_FIN_Nvram) {        
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 2);

    bool pileOK = verifierPileRTC(tft);  // ⬅️ Vérifie la NVRAM et la pile

    if (pileOK) {
      tft.setTextColor(ST77XX_GREEN);
      tft.println("");
      tft.println("Initialisation OK");
      jouerSon(1);  // ✅ petit bip aigu si tout va bien
    } else {
      tft.setTextColor(ST77XX_YELLOW);
      tft.println("");
      tft.println("Effacement NVRAM fait");
      jouerSon(0);  // ⚠️ bip grave si pile absente ou mémoire effacée
    }

    previousMillisSetup = currentMillis;
    etapeSetup++;
  }
  break;

 
    case 8:
     if (currentMillis - previousMillisSetup >= DELAI_FIN_Peer) {
      //Serial.print("Étape 8 - DELAI atteint après ");
      //Serial.print(currentMillis - previousMillisSetup);
      //Serial.println(" ms");

      pinMode(rel01, OUTPUT);
      pinMode(rel02, OUTPUT);
      digitalWrite(rel02, LOW);
      pinMode(led01, OUTPUT);

      pcf8574.write(0, LOW);
      pcf8574.write(1, LOW);
      for (int i = 2; i < 8; i++) {
        pcf8574.write(i, HIGH);
      }

      esp_now_register_recv_cb(OnDataRecv);
      esp_now_register_send_cb(OnDataSent);

      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, broadcastAddress, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;

      esp_now_del_peer(peerInfo.peer_addr);  // En cas de doublon
      esp_err_t result = esp_now_add_peer(&peerInfo);
      //Serial.print("esp_now_add_peer() result: ");
      //Serial.println(result);  // 0 = OK
      tft.setCursor(0, 98);
      if (result != ESP_OK) {
       tft.setTextColor(ST77XX_RED);
       tft.setTextSize(2);
      
       tft.println("Er02 > Peer");
       setupTermine = true;
       return;
      } else {
       tft.setTextColor(ST77XX_GREEN);
       tft.setTextSize(2);
       //Serial.println(">>> Affichage TFT : Peer > Ok ");
        
       tft.println("Peer  Ok");
      
       delay(1000);  // ← test : laisse le temps de lire
     }

    ledState1 = 0;
    digitalWrite(led01, ledState1);

    readFromNVram();

    // afficherInit();

    previousMillisSetup = currentMillis;
    etapeSetup++;
  }
  break;

 
    case 9:
      if (currentMillis - previousMillisSetup >= DELAI_FINAL_STEP2) {
        if (rtcPresent) {
          RTC.getRAM(50, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
          if (TimeIsSet != 0xaa55) {
            TimeIsSet = 0xaa55;
            RTC.setRAM(50, (uint8_t *)&TimeIsSet, sizeof(uint16_t));
          }
        }

        ledState1 = 0;
        digitalWrite(led01, ledState1);

        readFromNVram();
        
        afficherInit();

        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 10:
      if (currentMillis - previousMillisSetup >= 1000) {
        jouerSon(1);

        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 11:
      if (currentMillis - previousMillisSetup >= 5000) {
        jouerSon(1);
        digitalWrite(rel01, HIGH);
        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 12:
      if (currentMillis - previousMillisSetup >= 1800) {
        digitalWrite(rel01, LOW);
        jouerSon(1);
        //Serial.println(WiFi.macAddress());
        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 13:
      if (currentMillis - previousMillisSetup >= 5000) {
        jouerSon(2);
        afficherStart();
        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 14:
      if (currentMillis - previousMillisSetup >= 2000) {
        digitalWrite(rel02, LOW);
        setupTermine = true;
        forcerMajEcran = true;         // Force le premier affichage
        premierAffichageFait = false;  // ← important
        Mcmptr1 = 0;                   // ← Remise à zéro des points/crédits reçus
        compteurCredits = 0;
        //Serial.println("Setup non bloquant terminé !");  // Initialisation
      }
      sequenceFinaleSetup(currentMillis);
      break;
    default:
      tft.println("Err > Setup etape inconnue");
      break;
  }
}

// ************************************************************
//*************************************************************

void loop() {
  unsigned long maintenant = millis();

  // Exécute le loop toutes les ----- ms max
  if (maintenant - previousMillisLoop < intervalLoop) return;
  previousMillisLoop = maintenant;

  // Setup non bloquant
  if (!setupTermine) {
    setupNonBloquant();
  } else {

    // Affichage initial
    if (!premierAffichageFait) {
      cpt();  // Affiche immédiatement cpt() une fois au début
      premierAffichageFait = true;
    }

    // Vérifie si la publicité est terminée
    verifierFinPublicite();

    // Allume une sortie du PCF8574
    pcf8574.write(0, HIGH);

    // Gestion du crédit
    if (Mcmptr1 > 0 && !relaisEnCours && maintenant - dernierCreditEnvoye >= delaiEntreImpulsions) {
      jouerSon(0);
      ajouterCredit();
      dernierCreditEnvoye = maintenant;

      Mcmptr1--;
      cmptr01++;
      cmptrjrn++;
      totcmptr++;

      if (Mcmptr1 == 0) jouerSon(5);

      writeToNVram();
      forcerMajEcran = true;
    }

    // Lecture des boutons
    bool b1 = digitalRead(BUTTON_1_PIN) == LOW;
    bool b2 = digitalRead(BUTTON_2_PIN) == LOW;
    bool b3 = digitalRead(BUTTON_3_PIN) == LOW;

    // Détermination du mode écran
    if (b1) {
      modeEcran = MODE_BOUTON1;
    } else if (b2) {
      modeEcran = MODE_BOUTON2;
    } else if (b3) {
      modeEcran = MODE_BOUTON3;
    }

    // Si une pub est en cours
    if (pubEnCours) {
      lancerPublicite();  // Gère lui-même sa durée
    }

    static bool enVueSpeciale = false;

    // Gestion de l'affichage selon le mode
    switch (modeEcran) {
      case MODE_CPT:
        if (!resetEnCours && (maintenant - dernierAffichageCpt >= intervalleAffichageCpt)) {
          dernierAffichageCpt = maintenant;
          cpt();
        }
        break;

      case MODE_BOUTON1:
        enVueSpeciale = true;
        gererAppuiLongRemiseTotaux();
        break;

      case MODE_BOUTON2:
        enVueSpeciale = true;
        lancerPublicite();  // Encore ici, au cas où pubEnCours n’est pas encore vrai
        break;

      case MODE_BOUTON3:
        enVueSpeciale = true;
        gererAppuiLongRemiseJournalier();
        break;
    }

    // Retour au mode compteur si aucun bouton n’est appuyé
    if (!b1 && !b2 && !b3) {
      if (enVueSpeciale) {
        enVueSpeciale = false;
        forcerMajEcran = true;
      }
      modeEcran = MODE_CPT;
    }

    // Envoi des compteurs (à distance ?)
    SendCmptr();

    // Forcer mise à jour de l’écran
    if (forcerMajEcran && !resetEnCours) {
      cpt();
      forcerMajEcran = false;
    }

    // Gère l'état du relais
    gestionRelais();
  }
  // Débloque l’affichage après confirmation
  if (resetEnCours && millis() - momentConfirmation >= 1500) {
    resetEnCours = false;
    forcerMajEcran = true;
  }
}

// *********************************************************
void afficherRemiseAZero() {
  static bool bouton1Appuye = false;
  static bool confirmationEnCours = false;
  static unsigned long debutConfirmation = 0;

  if (!confirmationEnCours) {
    // Phase 1 : affichage de la demande de remise à zéro


    // Réinitialisation
    cmptrjrn = 0;
    cmptr01 = 0;
    writeToNVram();

    dataSent.cp2 = cmptrjrn;
    dataSent.fs1 = 0;
    esp_now_send(broadcastAddress, (uint8_t *)&dataSent, sizeof(dataSent));

    jouerSon(2);  // Feedback sonore

    // Phase 2 : confirmation visuelle

    tft.fillScreen(ST77XX_GREEN);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLACK);


    tft.setCursor(20, 40);
    tft.println("Compteur");

    tft.setCursor(20, 60);
    tft.println("Journalier");

    tft.setCursor(20, 80);
    tft.println("remis a");

    tft.setCursor(20, 100);
    tft.println("zero");

    confirmationEnCours = true;
    debutConfirmation = millis();
  } else {
    // Attend la fin de la confirmation
    if (millis() - debutConfirmation >= 1500) {
      // Retour à l'état normal
      confirmationEnCours = false;
      resetEnCours = false;
      modeEcran = MODE_CPT;
      forcerMajEcran = true;

      if (digitalRead(BUTTON_1_PIN) == HIGH) {
        bouton1Appuye = false;  // Autorise un nouvel appui
      }
    }
  }
}

void cpt() {
  if (modeEcran != MODE_CPT) return;

  static bool etatPrecedent = !etatLed01;


  bool changementEtatConnexion = (etatLed01 != etatPrecedent);

  // Détecte changement des compteurs
  bool changementCompteurs =
    (Mcmptr1 != prev_Mcmptr1 || cmptrjrn != prev_cmptrjrn || totcmptr != prev_totcmptr || compteurCredits != prev_compteurCredits);

  if (changementEtatConnexion || changementCompteurs || forcerMajEcran) {
    tft.fillScreen(etatLed01 ? ST77XX_RED : ST77XX_BLUE);
    tft.setTextSize(1);
    tft.setCursor(5, 20);
    tft.setTextColor(ST77XX_WHITE);
    tft.print(etatLed01 ? "Non connecte au bar" : "Connecte au bar");

    // Réaffiche les compteurs
    tft.setTextColor(ST7735_YELLOW);
    tft.setTextSize(2);

    uint16_t bgColor = etatLed01 ? ST77XX_RED : ST77XX_BLUE;
    for (int i = 0; i < 4; i++) {
      tft.fillRect(20, 40 + (i * 20), 100, 20, bgColor);
    }

    tft.setCursor(1, 35);
    tft.print("   :");
    tft.println(Mcmptr1);  
    tft.setCursor(1, 60);
    tft.print("Tep:");
    tft.println(cmptrjrn);
    tft.setCursor(1, 84);
    tft.print("Tot:");
    tft.println(totcmptr);
    tft.setCursor(1, 109);
    tft.print("Cre:") ;
    tft.println(compteurCredits);
  
    // Mise à jour des états précédents
    etatPrecedent = etatLed01;
    prev_Mcmptr1 = Mcmptr1;
    prev_cmptrjrn = cmptrjrn;
    prev_totcmptr = totcmptr;
    prev_compteurCredits = compteurCredits;

    forcerMajEcran = false;
  }
}
