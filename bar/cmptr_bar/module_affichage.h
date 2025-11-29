#ifndef MODULE_AFFICHAGE_H
#define MODULE_AFFICHAGE_H
#include <Arduino.h>


#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
 

// void afficherStart(Adafruit_ST7735 &tft);    // 👈 déclaration à ajouter ici

extern Adafruit_ST7735 tft;

void afficherStart(Adafruit_ST7735 &tft);     // écran 2
//void afficherInit() ;  // écran 3
//void afficherNum()  ;  // écran 4
//void afficherInfo() ;  // écran 5
#endif
