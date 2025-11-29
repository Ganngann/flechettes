 #include "son.h"
#include <Arduino.h>

//#define BUZZER_PIN 26
const int BUZZER_PIN = 26;

void initialiserSon() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

// État interne du son
unsigned long sonMillis = 0;
int etatSon = 0;
int sonActif = 0;
int sequenceIndex = 0;

// Structure pour les sons composés
struct SonElement {
  int frequence;
  int duree;
};

SonElement sequenceSon[10];
int sequenceTaille = 0;

void jouerSon(int effet) {
  if (etatSon != 0) return; // Évite d’interrompre un son en cours

  switch (effet) {
    case 1:  // Bip simple
      tone(BUZZER_PIN, 1000);
      sonMillis = millis();
      sonActif = 200;
      etatSon = 1;
      break;

    case 2:  // Succès
      tone(BUZZER_PIN, 1500);
      sonMillis = millis();
      sonActif = 150;
      etatSon = 1;
      break;

    case 3:  // Erreur
      tone(BUZZER_PIN, 500);
      sonMillis = millis();
      sonActif = 300;
      etatSon = 1;
      break;

    case 4:  // Démarrage (séquence de 3 sons)
      sequenceTaille = 3;
      sequenceSon[0] = {800, 100};
      sequenceSon[1] = {1000, 100};
      sequenceSon[2] = {1200, 100};
      sequenceIndex = 0;
      etatSon = 2;
      break;

    case 5:  // Bouton
      tone(BUZZER_PIN, 1200);
      sonMillis = millis();
      sonActif = 100;
      etatSon = 1;
      break;

    case 6:  // Erreur grave (séquence complexe)
      sequenceTaille = 6;
      sequenceSon[0] = {200, 200};
      sequenceSon[1] = {600, 200};
      sequenceSon[2] = {1200, 150};
      sequenceSon[3] = {1800, 100};
      sequenceSon[4] = {400, 300};
      sequenceSon[5] = {100, 400};
      sequenceIndex = 0;
      etatSon = 2;
      break;

    default:
      noTone(BUZZER_PIN);
      etatSon = 0;
      break;
  }
}

void actualiserSon() {
  if (etatSon == 1) {
    if (millis() - sonMillis >= sonActif) {
      noTone(BUZZER_PIN);
      etatSon = 0;
    }
  } else if (etatSon == 2) {
    if (millis() - sonMillis >= sonActif) {
      if (sequenceIndex < sequenceTaille) {
        tone(BUZZER_PIN, sequenceSon[sequenceIndex].frequence);
        sonMillis = millis();
        sonActif = sequenceSon[sequenceIndex].duree;
        sequenceIndex++;
      } else {
        noTone(BUZZER_PIN);
        etatSon = 0;
      }
    }
  }
}

 

