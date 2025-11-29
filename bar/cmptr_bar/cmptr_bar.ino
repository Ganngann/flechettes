/*
 cmptr07
 Version 07 
 voir version.cpp
 */



#include <Arduino.h>
#include "module_son.h"
#include "module_num_serie.h"
#include "module_affichage.h"
#include "setup_module.h"
#include "version.h"
#include "variables_globales.h"
#include "module_connexion.h"

#include "config_cpu.h"

#include <ezButton.h>
#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST77xx.h>  // Hardware-specific library
#include <Adafruit_ST7735.h>  // Hardware-specific library
#include <SPI.h>
#include <esp_now.h>
#include <WiFi.h>
#include <OneWire.h>
#include <Preferences.h>
#include <EEPROM.h>
#include <Wire.h>

#include <DS1307new.h>
#include <I2C_eeprom.h>
//#include <DS1307new.h>
//#include <DS1307Lib.h>
//Adresse I2C de la DS1307
//#define DS1307_ID 0x68
//RTC_DS1307 rtc;

#include <esp_idf_version.h>

I2C_eeprom ee(0x50, I2C_DEVICESIZE_24LC32);

// === VARIABLES GLOBALES ===
uint16_t led01 = 25;
bool led01On = false;
unsigned long led01StartTime = 0;
const unsigned long LED01_DURATION = 300;


// ✅ à ajouter
#if TYPE_CPU == 3
bool clignotementLEDActif = false;
bool clignotementAnnulationActif = false;
unsigned long clignoAnnulStart = 0;
const unsigned long DUREE_CLIGNO_ANNUL = 2000;  // ⏱️ 2 secondes
unsigned long previousClignoDiscoMillis = 0;
bool ledRedDiscoState = false;
const unsigned long CLIGNO_DISCO_INTERVAL = 500;
#endif

bool SendStts;

#if ESP_IDF_VERSION_MAJOR >= 5
void OnDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status) {
  SendStts = (status == ESP_NOW_SEND_SUCCESS);
  isConnected = SendStts;

  digitalWrite(led01, HIGH);
  led01On = true;
  led01StartTime = millis();
}
#else
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  SendStts = (status == ESP_NOW_SEND_SUCCESS);
  isConnected = SendStts;

  digitalWrite(led01, HIGH);
  led01On = true;
  led01StartTime = millis();
}
#endif


// variables non-bloquantes pour le scroll vertical
unsigned long previousMillisScroll = 0;
int scrollY = -20;
const int scrollSpeed = 50;  // ms entre chaque pas
bool messageScrolling = false;
const char *currentMsg = "";

// messages
const char *messages[] = {
  "Bienvenue !", "Profitez !", "Cheers !", "Happy Hour",
  "Flech ok !", "Bar ouvert", "Prenez ", "Ambiance"
};
const int messageCount = sizeof(messages) / sizeof(messages[0]);


#define TFT_SCLK 18  // SPI clock
#define TFT_MOSI 23  // SPI Data
#define TFT_CS 15    // Display enable (Chip select), if not enabled will not talk on SPI bus

#define TFT_RST 13
#define TFT_DC 4  // register select (stands for Data Control perhaps!)

// **********************************JM
//uint8_t broadcastAddress[] = { 0x48, 0xE7, 0x29, 0x96, 0x98, 0xC4 };
// **********************************TM

unsigned long lastPressTime = 0;    // Temps de la dernière pression
unsigned long debounceDelay = 250;  // Délai de rebond (en milliseconde) pour eviter de détecté plusieurs fois une pression trop rapide

const uint8_t BUTTON_CRE = 2;
const uint8_t BUTTON_NUM = 3;



const uint8_t BUTTON_1_PIN = Val_Pin_Env;
const uint8_t BUTTON_2_PIN = Val_Pin_Cre;
const uint8_t BUTTON_3_PIN = Val_Pin_Ann;



const uint8_t buz = 26;

typedef struct struct_message {
  uint16_t cp1;  // incrément compteur nbr de points envoyé
  uint16_t cp2;  // compteur journalier
  uint16_t cp3;  // compteur total
  bool fp1;
  bool fs1;
} struct_message;

String getMacString(const unsigned char *mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr),
           "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(macStr);
}


enum State {
  ETAT_INIT,
  ETAT_ATTENTE,
  ETAT_AJOUT_CREDIT,
  ETAT_ANNULATION,
  ETAT_ENVOI_CREDIT,
  ETAT_RESULTAT_ENVOI,
  ETAT_INFO_ZERO,
  ETAT_TIMEOUT_ECRAN,
  ETAT_PUB,        // ✅ ajouté
  ETAT_COMPTEURS,  // ✅ ajouté
  ETAT_CREDIT,
  ETAT_JEU,  // ✅ ajouté  nouvel état pour gérer affichage crédit
};

State etatActuel = ETAT_INIT;

bool b1_state, b2_state, b3_state;
bool b1_lastState = false, b2_lastState = false, b3_lastState = false;
unsigned long lastB1Millis = 0, lastB2Millis = 0, lastB3Millis = 0;
const unsigned long debounceB1 = 300, debounceB2 = 50, debounceB3 = 300;

bool isSending = false;
unsigned long sendStartTime = 0;
const unsigned long SEND_TIMEOUT = 500;

bool previousConnectionState = false;
static uint16_t TotCn_prev = 0;

struct_message dataSent;
struct_message dataRcvr;


uint16_t cmptrjrn;
uint16_t totcmptr;

uint16_t EnvTot;
uint16_t Read_EnvTot;


uint16_t CptTot_dpt = 0;
uint16_t TpsCred_dep = 0;
uint16_t TpsCred_int = 0;
uint16_t Diff_mise = 0;

//  ✅✅✅✅✅✅✅✅✅✅✅✅✅ Déclaration ajoutée
bool ecranInitialise = false;
bool annulationCreditExces = false;
bool annulationCreditReset = false;  // ✅ Déclaration ajoutée

uint8_t lastOk = 255;  // impossible au démarrage, donc forcera un affichage au début



uint8_t valbtp[3] = { 0, 0, 0 };
uint16_t serv = 0;
uint16_t TotAv = 0;
uint16_t TotCn = 0;
uint16_t ok = 0;


uint16_t ledState = LOW;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);


bool ledState1 = false;
uint16_t led02 = 27;
bool ledState2 = LOW;
uint16_t sttcpt = 0;
uint16_t ticcpt = 0;
uint16_t Cpt = 0;
uint16_t lastScreenUpdate = 0;
unsigned long screenHoldUntil = 0;  // pour geler l’affichage après un écran temporaire


static uint16_t previousTotCn = 0;
unsigned long lastCreditTime = 0;
const unsigned long creditDebounce = 500;  // 0,5 seconde


uint16_t Rcmptr1;
uint16_t Rcmptr2;
uint16_t Rcmptr3;
bool Rfp1;
bool Rfs1;

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



// Variable to add info about peer
esp_now_peer_info_t peerInfo;
//****************************************************************************
//void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
// ***************************************************************************

// === CALLBACK : réception ESP‑NOW ===
void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  memcpy(&dataRcvr, incomingData, sizeof(dataRcvr));

  lastReceivedTime = millis();  // ✅ mise à jour pour le timeout
  //Serial.println("📩 Donnée reçue !");

  // Affichage du MAC source
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5]);
  //Serial.println(macStr);

  // Mettre à jour les données reçues pour affichage
  Rcmptr1 = dataRcvr.cp1;
  cmptrjrn = dataRcvr.cp2;
  totcmptr = dataRcvr.cp3;
  Rfp1 = dataRcvr.fp1;
  Rfs1 = dataRcvr.fs1;


  digitalWrite(led02, ledState2 = !ledState2);


  //Serial.print("📡 Paquet reçu de : ");
}



/*
//        ✅ 1. Correction de OnDataSent() :
// === CALLBACK : envoi ESP‑NOW ===
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  SendStts = (status == ESP_NOW_SEND_SUCCESS);
  isConnected = SendStts;

  //digitalWrite(led01, ledState1 = !ledState1);

  
  digitalWrite(led01, HIGH);
  led01On = true;
  led01StartTime = millis();
  


  //Serial.println(SendStts ? "✅ Send OK" : "❌ Send FAIL");

}
*/

 


void ecran() {
  switch (ok) {
    case 1:
      /*
      //Serial.print("etat connection 1 > : ");
      //Serial.println(isConnected);
      //Serial.print("ecranInitialise 1 > : ");
      //Serial.println(ecranInitialise);
      */
      mettreAJourConnexion();
      if (isConnected) {
        if (!ecranInitialise) {
          tft.fillScreen(ST77XX_BLUE);  // Fond bleu pour tout l'écran
          // Affiche "Bar onLine" en bas, fixe
          tft.fillRect(0, 100, tft.width(), 40, ST77XX_BLUE);
          tft.setTextColor(ST77XX_YELLOW);
          tft.setCursor(10, 110);
          tft.println("Bar onLine");
          ecranInitialise = true;
        }

        unsigned long now = millis();
        if (!messageScrolling) {
          scrollY = 0;
          currentMsg = messages[random(0, messageCount)];
          messageScrolling = true;
          previousMillisScroll = now;
        }

        if (now - previousMillisScroll >= scrollSpeed) {
          previousMillisScroll = now;

          // Efface uniquement la zone de message défilant
          tft.fillRect(0, 0, tft.width(), 100, ST77XX_BLUE);

          // Affiche le texte défilant
          tft.setCursor(10, scrollY);
          tft.setTextColor(ST77XX_WHITE);
          tft.println(currentMsg);
          scrollY += 4;

          if (scrollY > 80) {
            messageScrolling = false;
            // Effacement final de la zone message
            tft.fillRect(0, 0, tft.width(), 100, ST77XX_BLUE);
          }
        }
      }

      // Si plus connecté, écran rouge "Jeux off"
      if (!isConnected) {
        if (!ecranInitialise) {
          tft.fillScreen(ST77XX_RED);
          tft.setTextColor(ST77XX_YELLOW);
          tft.setCursor(10, 60);
          tft.println("Jeux off");
          ecranInitialise = true;
          messageScrolling = false;
        }
      }
      break;

    case 3:
      tft.fillScreen(ST77XX_BLACK);
      messageScrolling = false;  // ✅ on désactive le scroll pour éviter superposition
      scrollY = 0;
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(3, 6);
      tft.setTextColor(ST77XX_YELLOW);
      tft.println("Cred/jeux ");
      tft.setCursor(41, 34);
      tft.print(TotCn * Val_Cred);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(10, 60);
      tft.print(" En Euro:");
      tft.setTextSize(5);
      tft.setCursor(40, 85);
      tft.print(TotCn);
      tft.setTextSize(2);
      break;

    case 4: {
      messageScrolling = false;  // ✅ on désactive le scroll pour éviter superposition
      scrollY = 0;
      tft.fillScreen(ST77XX_BLUE);
      //tft.fillScreen(ST77XX_WHITE);  // Nettoie l’écran
      //tft.setCursor(0, 10);   
      
      // Position de départ
      int y = 5;  // position verticale initiale
     
     int hauteurTexte = 8;           // hauteur en pixels pour tailleTexte = 1
     int interligne = 1;             // espace entre les lignes (ajuste ici)

    for (int i = 0; i < nbLignes; i++) {
     tft.setTextColor(lignes[i].couleur);
     tft.setTextSize(lignes[i].tailleTexte);
     tft.setCursor(0, y);
     tft.print(lignes[i].texte);

      y += hauteurTexte * lignes[i].tailleTexte + interligne;
    } 



      tft.setTextSize(2);
    }
      break;

    case 5:
      messageScrolling = false;  // ✅ on désactive le scroll pour éviter superposition
      scrollY = 0;
      tft.fillScreen(ST77XX_BLACK);
      tft.setCursor(10, 10);
      tft.setTextColor(ST77XX_WHITE);
      tft.print("Clear Cpt");
      tft.setTextSize(4);
      tft.setCursor(40, 42);
      tft.setTextColor(ST77XX_BLUE);
      tft.print(TotCn);
      tft.setTextColor(ST77XX_WHITE);
      tft.setTextSize(2);
      tft.setCursor(0, 93);
      tft.print("..........");
      delay(3000);
      tft.fillScreen(ST77XX_BLACK);
      break;

    case 6:
      messageScrolling = false;  // ✅ on désactive le scroll pour éviter superposition
      scrollY = 0;
      tft.fillScreen(ST77XX_BLACK);
      tft.setTextColor(ST77XX_YELLOW);
      tft.setCursor(2, 5);
      tft.println("Journalier");
      tft.setCursor(10, 30);
      tft.setTextColor(ST77XX_WHITE);
      tft.println(cmptrjrn);
      tft.setTextColor(ST77XX_YELLOW);
      tft.setCursor(5, 55);
      tft.println("Total");
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(10, 82);
      tft.println(totcmptr);
      tft.setTextColor(ST77XX_YELLOW);
      tft.setTextSize(1);
      tft.setCursor(10, 110);
      tft.println("En Euros");
      tft.setTextSize(2);
      break;

    case 8:
      messageScrolling = false;  // ✅ on désactive le scroll pour éviter superposition
      scrollY = 0;
      tft.fillScreen(ST77XX_RED);
      tft.setCursor(10, 2);
      tft.setTextColor(ST77XX_YELLOW);
      tft.print("Operation");
      tft.setTextSize(4);
      tft.setCursor(48, 25);
      tft.setTextColor(ST77XX_WHITE);
      tft.print(TotCn);
      tft.setTextSize(2);
      tft.setCursor(10, 59);
      if (annulationCreditExces) {
        tft.setTextColor(ST77XX_WHITE);
        tft.println(("Euro > ") + String(max_cred));
      }
      tft.setCursor(10, 83);
      tft.println("Credits");
      tft.setCursor(10, 103);
      tft.println("annules !");
      break;

    case 9:  // ✅ Ecran "Connexion absente"
      messageScrolling = false;
      scrollY = 0;
      tft.fillScreen(ST77XX_RED);
      tft.setTextColor(ST77XX_WHITE);
      tft.setCursor(0, 20);
      tft.setTextSize(2);
      tft.println("Jeu distant");
      tft.setCursor(25, 45);
      tft.println("eteint");
      tft.setCursor(10, 80);
      tft.setTextColor(ST77XX_YELLOW);
      tft.println("Credit");
      tft.setCursor(10, 100);
      tft.println("desactive");
      tft.setTextSize(2);
      break;




    default:
      // Réinitialise pour les autres cas
      ecranInitialise = false;
      messageScrolling = false;
      scrollY = 0;
      break;
  }
}

//*********************************************************************
#if TYPE_CPU == 3

bool ledClignotementActif = false;
unsigned long ledClignoStart = 0;
const unsigned long LED_CLIGNO_DUREE = 1500;

void gererLEDTransfert(bool transfertOK) {
  static unsigned long previousClignoMillis = 0;
  static bool ledState = false;

  if (isSending) {
    ledClignotementActif = true;
    ledClignoStart = millis();

    previousClignoMillis = 0;  // Réinitialise
  }

  if (ledClignotementActif) {
    unsigned long currentMillis = millis();

    unsigned long dureeCligno = transfertOK ? 2500 : 1500;  // ⏱️ Durée plus longue si OK
    unsigned long vitesseCligno = transfertOK ? 120 : 250;  // ⚡ Clignote plus vite si OK

    if (currentMillis - previousClignoMillis >= vitesseCligno) {
      previousClignoMillis = currentMillis;
      ledState = !ledState;

      if (transfertOK) {
        digitalWrite(LdBlu1, ledState);  // 🔵 Clignote si OK
        digitalWrite(LdRed1, LOW);
      } else {
        digitalWrite(LdRed1, ledState);  // 🔴 Clignote si échec
        digitalWrite(LdBlu1, LOW);
      }
    }

    if (currentMillis - ledClignoStart > dureeCligno) {
      ledClignotementActif = false;
      digitalWrite(LdBlu1, LOW);
      digitalWrite(LdRed1, LOW);
    }
  }
}


void gererEtatsLEDs() {
  if (ledClignotementActif) return;  // Ne pas interférer avec l'animation de transfert

  unsigned long now = millis();

  // === Gestion du clignotement en cas de dépassement crédit ===
  if ((annulationCreditExces || annulationCreditReset) && !clignotementAnnulationActif) {
    clignotementAnnulationActif = true;
    clignoAnnulStart = now;
  }

  if (clignotementAnnulationActif) {
    digitalWrite(LdRed1, (now / 200) % 2);  // 🔴 Clignote toutes les 200 ms
    digitalWrite(LdBlu1, LOW);

    if (now - clignoAnnulStart >= DUREE_CLIGNO_ANNUL) {
      clignotementAnnulationActif = false;
      annulationCreditExces = false;
      annulationCreditReset = false;
      digitalWrite(LdRed1, LOW);
    }
    return;
  }

  // === 🔴 Clignotement si déconnecté ===
  if (!isConnected) {
    if (now - previousClignoDiscoMillis >= CLIGNO_DISCO_INTERVAL) {
      previousClignoDiscoMillis = now;
      ledRedDiscoState = !ledRedDiscoState;
      digitalWrite(LdRed1, ledRedDiscoState);  // 🔴 clignote
    }
    digitalWrite(LdBlu1, LOW);  // 🔵 reste éteinte
    return;
  }

  // === État normal si connecté ===
  digitalWrite(LdRed1, LOW);                        // 🔴 éteinte
  digitalWrite(LdBlu1, (TotCn == 0) ? HIGH : LOW);  // 🔵 allumée si pas de crédit
}



#endif TYPE_CPU == 3
//*************************************************************************



void setup() {
  setupNonBloquant();  // Appel initial (étape 0)
}


void setupNonBloquant() {
  unsigned long currentMillis = millis();

  switch (etapeSetup) {
    case 0:
      pinMode(led01, OUTPUT);
      pinMode(led02, OUTPUT);
      pinMode(BUTTON_2_PIN, INPUT);

#if TYPE_CPU == 3
      pinMode(LdRed1, OUTPUT);
      pinMode(LdBlu1, OUTPUT);
      digitalWrite(LdRed1, HIGH);  // Allume la LED titre rouge
      digitalWrite(LdBlu1, HIGH);  // Allume la LED titre bleue
#endif


      for (byte i = 0; i < BUTTON_NUM; i++) {
        buttonArray[i].setDebounceTime(30);
        buttonArray[i].setCountMode(COUNT_FALLING);
      }

      initialiserSon();
      initialiserModules();

      Serial.begin(115200);

      tft.initR(INITR_GREENTAB);
      tft.fillScreen(ST77XX_BLACK);
      tft.setRotation(3);

      WiFi.mode(WIFI_STA);
      WiFi.disconnect();

      ee.begin();
      Wire.begin(21, 22);

      previousMillisSetup = currentMillis;
      etapeSetup++;
      break;

    case 1:

      if (currentMillis - previousMillisSetup >= 1500) {
        afficherNumSerie(tft);
        previousMillisSetup = currentMillis;
        etapeSetup++;
      }
      break;

    case 2:
      afficherStart(tft);
      previousMillisSetup = currentMillis;
      etapeSetup++;  // Va à l'étape 3
      break;

    case 3:
      if (currentMillis - previousMillisSetup >= 4000) {
        etapeSetup++;  // Va à l'étape 4
      }
      break;


    case 4:
      if (esp_now_init() != ESP_OK) {
        //Serial.println("❌ ESP-NOW Init Failed");
        tft.fillScreen(ST77XX_RED);
        tft.setTextSize(2);
        tft.setCursor(10, 40);
        tft.println("   ESP-NOW\n\n   Erreur !");
        while (true)
          ;
      }

      //Serial.println("✅ ESP-NOW initialisé");

      esp_now_register_send_cb(OnDataSent);
      esp_now_register_recv_cb(OnDataRecv);

      memset(&peerInfo, 0, sizeof(peerInfo));
      memcpy(peerInfo.peer_addr, broadcastAddress, 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;

      if (!esp_now_is_peer_exist(broadcastAddress)) {
        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
          //Serial.println("❌ Erreur lors de l'ajout du peer");
          tft.fillScreen(ST77XX_RED);
          tft.setTextSize(2);
          tft.setCursor(10, 40);
          tft.println("   Peer\n\n   Erreur !");
          while (true)
            ;
        }
        //Serial.println("✅ Peer ajouté");
      }

      dataSent.cp1 = 0;
      dataSent.cp2 = cmptrjrn;
      dataSent.cp3 = totcmptr;
      dataSent.fs1 = 1;
      dataSent.fp1 = false;

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&dataSent, sizeof(dataSent));
      //Serial.print("📤 Envoi initial : ");
      //Serial.println(result == ESP_OK ? "OK" : "Erreur");

      if (result == ESP_OK) {
        //Serial.println("Message envoyé avec succès");
      } else {
        tft.fillScreen(ST77XX_RED);
        tft.setTextSize(2);
        tft.setCursor(10, 40);
        tft.println("   Jeu\n\n OFF line");
        //Serial.print("Erreur d'envoi : ");
        //Serial.println(result);
      }

      setupTermine = true;
      //Serial.println("✅ Setup terminé, passage à l'état INIT");

      etatActuel = ETAT_INIT;
      messageScrolling = false;
      scrollY = -20;
      break;
  }
}

// ****************************************
unsigned long previousLoopMillis = 0;
const unsigned long loopInterval = 100;  // Intervalle de 100ms

void loop() {
  // 🔄 MODIF ICI : Capturer millis() une seule fois pour cohérence
  unsigned long currentMillis = millis();

  // Tempo non bloquante (remplace delay(100))
  if (currentMillis - previousLoopMillis < loopInterval) return;
  previousLoopMillis = currentMillis;

  // Exécution du setup non bloquant
  if (!setupTermine) {
    setupNonBloquant();
  } else {


    actualiserSon();

    for (int i = 0; i < 3; i++) buttonArray[i].loop();

    mettreAJourConnexion();

// ***************************************************************
#if TYPE_CPU == 3
    gererLEDTransfert(SendStts);  // Lance le clignotement LED en priorité
    gererEtatsLEDs();             // Gère l'état des LED si pas de clignotement
#endif
    // ***************************************************************

    static bool etatPrecedent = false;
    if (isConnected != etatPrecedent) {
      etatPrecedent = isConnected;

      if (!isConnected && TotCn > 0) {
        TotCn = 0;
        annulationCreditExces = false;

        ok = 8;
        jouerSon(6);
        ecran();
        etatActuel = ETAT_TIMEOUT_ECRAN;
        ecranInitialise = false;
        screenHoldUntil = currentMillis + 4000;  // 🔄 MODIF ICI
      }
    }

    if (isConnected != previousConnectionState) {
      previousConnectionState = isConnected;
      if (etatActuel != ETAT_TIMEOUT_ECRAN) {
        ok = 1;
        ecranInitialise = false;
        ecran();
      }
    }

    b1_state = digitalRead(BUTTON_1_PIN) == LOW;
    b2_state = digitalRead(BUTTON_2_PIN) == LOW;  // credit
    b3_state = digitalRead(BUTTON_3_PIN) == LOW;

    static bool b2Enfonce = false;
    if (b2_state && !b2_lastState && currentMillis - lastB2Millis > debounceB2) {
      b2Enfonce = true;
      lastB2Millis = currentMillis;
    }

    if (!b2_state && b2_lastState && b2Enfonce && currentMillis - lastB2Millis > debounceB2) {
      b2Enfonce = false;
      lastB2Millis = currentMillis;

      if (etatActuel == ETAT_ATTENTE) {
        if (!isConnected) {
          jouerSon(6);
          ok = 9;
          ecran();
          screenHoldUntil = currentMillis + 3000;  // 🔄 MODIF ICI
          etatActuel = ETAT_TIMEOUT_ECRAN;
        } else {
          TotCn += Val_TotCn;
          jouerSon(1);
          ok = 3;
          ecran();
          screenHoldUntil = currentMillis + 100;  // 🔄 MODIF ICI
        }
      }
    }

    switch (etatActuel) {

      case ETAT_INIT:
        ok = 1;
        ecran();
        etatActuel = ETAT_ATTENTE;
        break;

      case ETAT_ATTENTE:
        if ((long)(currentMillis - screenHoldUntil) > 0 && TotCn == 0) {  // 🔄 MODIF ICI
          ok = 1;
          ecran();
        }

        if (b1_state && !b1_lastState && currentMillis - lastB1Millis > debounceB1) {
          jouerSon(1);
          etatActuel = (TotCn == 0) ? ETAT_COMPTEURS : ETAT_ANNULATION;
          lastB1Millis = currentMillis;
        }

        if (b3_state && !b3_lastState && currentMillis - lastB3Millis > debounceB3) {
          jouerSon(5);

          if (TotCn > max_cred) {
            noTone(BUZZER_PIN);
            etatSon = 0;

            annulationCreditExces = true;
            TotCn = 0;
            ok = 8;
            ecran();
            jouerSon(6);
            screenHoldUntil = currentMillis + 4000;  // 🔄 MODIF ICI
            ecranInitialise = false;
            etatActuel = ETAT_TIMEOUT_ECRAN;
          } else if (TotCn > 0) {
            etatActuel = ETAT_ENVOI_CREDIT;
          } else {
            etatActuel = ETAT_PUB;
          }

          lastB3Millis = currentMillis;
        }
        break;

      case ETAT_ANNULATION:
        annulationCreditExces = false;
        annulationCreditReset = true;
        ok = 8;
        TotCn = 0;
        jouerSon(3);
        ecran();
        screenHoldUntil = currentMillis + 4000;  // 🔄 MODIF ICI
        etatActuel = ETAT_TIMEOUT_ECRAN;
        break;

      case ETAT_ENVOI_CREDIT:
        dataSent.cp1 = TotCn;
        dataSent.fs1 = 1;
        dataSent.fp1 = false;
        esp_now_send(broadcastAddress, (uint8_t *)&dataSent, sizeof(dataSent));
        isSending = true;
        sendStartTime = currentMillis;
        etatActuel = ETAT_RESULTAT_ENVOI;
        break;

      case ETAT_RESULTAT_ENVOI:
        if ((long)(currentMillis - sendStartTime) >= SEND_TIMEOUT) {  // 🔄 MODIF ICI
          isSending = false;

          if (SendStts) {
            tft.fillScreen(ST77XX_BLACK);
            tft.setTextColor(ST77XX_WHITE);
            tft.setTextSize(2);
            tft.setCursor(10, 40);
            tft.println("Transfert");
            tft.println(" reussi");
            jouerSon(2);
            screenHoldUntil = currentMillis + 1000;  // 🔄 MODIF ICI
          } else {
            tft.fillScreen(ST77XX_RED);
            tft.setTextSize(2);
            tft.setCursor(10, 40);
            tft.println("   Jeu");
            tft.println(" ");
            tft.println(" OFF line");
            jouerSon(6);
            screenHoldUntil = currentMillis + 3000;  // 🔄 MODIF ICI
            ecranInitialise = false;
          }

          TotCn = 0;
          etatActuel = ETAT_TIMEOUT_ECRAN;
        }
        break;

      case ETAT_PUB:
        ok = 4;
        ecran();
        screenHoldUntil = currentMillis + 3000;  // 🔄 MODIF ICI
        etatActuel = ETAT_TIMEOUT_ECRAN;
        break;

      case ETAT_COMPTEURS:
        ok = 6;
        ecran();
        screenHoldUntil = currentMillis + 3000;  // 🔄 MODIF ICI
        etatActuel = ETAT_TIMEOUT_ECRAN;
        break;

      case ETAT_TIMEOUT_ECRAN:
        // if ((long)(currentMillis - screenHoldUntil) > 0 && !b2_state) {  // 🔄 MODIF ICI

        if ((long)(currentMillis - screenHoldUntil) > 0 && (!b2_state || (currentMillis - screenHoldUntil) > 5000)) {

          if (TotCn == 0) ok = 1;
          ecran();
          for (byte i = 0; i < BUTTON_NUM; i++) buttonArray[i].resetCount();
          etatActuel = ETAT_ATTENTE;
          ecranInitialise = false;
        }
        break;
    }

    if (etatActuel == ETAT_ATTENTE && TotCn != TotCn_prev) {
      ok = (TotCn > 0) ? 3 : 1;
      ecran();
      screenHoldUntil = currentMillis + ((TotCn > 0) ? 700 : 0);  // 🔄 MODIF ICI
      TotCn_prev = TotCn;
    }

    if (led01On && currentMillis - led01StartTime >= LED01_DURATION) {
      digitalWrite(led01, LOW);
      led01On = false;
    }

    b1_lastState = b1_state;
    b2_lastState = b2_state;
    b3_lastState = b3_state;
  }
}
