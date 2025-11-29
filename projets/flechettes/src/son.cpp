 #include "son.h"
#include <Arduino.h>

const int brocheBuzzer = 26;

void initialiserSon() {
  pinMode(brocheBuzzer, OUTPUT);
  digitalWrite(brocheBuzzer, LOW);
}

void jouerSon(unsigned char typeSon) {
  switch (typeSon) {
    case SON_BIP:
      tone(brocheBuzzer, 1000, 100);
      break;
    case SON_SUCCES:
      tone(brocheBuzzer, 1200, 200);
      delay(100);
      tone(brocheBuzzer, 1400, 200);
      break;
    case SON_ERREUR:
      tone(brocheBuzzer, 400, 300);
      break;
    case SON_DEMARRAGE:
      tone(brocheBuzzer, 900, 150);
      delay(100);
      tone(brocheBuzzer, 1100, 150);
      break;
    case SON_BOUTON:
      tone(brocheBuzzer, 800, 100);
      break;
    case SON_GRAVE:
      tone(brocheBuzzer, 200, 500);
      break;
    default:
      break;
  }
}

