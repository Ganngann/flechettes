 #ifndef MODULE_CONNEXION_H
#define MODULE_CONNEXION_H

#include <Arduino.h>

extern bool isConnected;
extern unsigned long lastReceivedTime;

constexpr unsigned long TIMEOUT = 5000;

void mettreAJourConnexion();

#endif
