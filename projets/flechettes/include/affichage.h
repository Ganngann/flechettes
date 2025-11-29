//Flechettes
#pragma once

#ifndef MODULE_AFFICHAGE_H
#define MODULE_AFFICHAGE_H



#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

extern Adafruit_ST7735 tft;

void afficherSetup();  // écran 1
void afficherStart();  // écran 2
void afficherInit() ;  // écran 3
void afficherNum()  ;  // écran 4
void afficherPublicite() ;

#endif
