 #ifndef MODULE_SON_H
#define MODULE_SON_H

 #include <Arduino.h>
 

#pragma once
void initialiserSon();
void jouerSon(int effet);
void actualiserSon();  // <== très important : à appeler régulièrement dans loop()


// Facultatif : si tu veux accéder à etatSon directement
extern int etatSon;

// Facultatif : accès au forçage ou à l'état du buzzer
extern const int BUZZER_PIN;

#endif

