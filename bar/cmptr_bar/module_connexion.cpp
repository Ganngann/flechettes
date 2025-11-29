 #include "module_connexion.h"
 #include <Arduino.h>
// Définition réelle des variables (une seule fois ici)
bool isConnected = false;
unsigned long lastReceivedTime = 0;

void mettreAJourConnexion() {
  if (millis() - lastReceivedTime > TIMEOUT) {
    isConnected = false;
  } else {
    isConnected = true;
  }
}
