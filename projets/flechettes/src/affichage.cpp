 #include "affichage.h"
#include <WiFi.h>
#include "config.h"
 

void afficherNum() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(05, 80);
  tft.println("Num. serie");
  tft.setCursor(30, 105);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_ORANGE);
  tft.println(version);
  tft.setCursor(28, 120);
  tft.println(num_soft);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 18, 20, 45, 5, ST77XX_YELLOW);
  tft.fillRoundRect(69, 18, 20, 45, 5, ST77XX_YELLOW); 
}



void afficherSetup() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(28, 80);
  tft.println("Setup");

  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 18, 20, 45, 5, ST77XX_RED);
  tft.fillRoundRect(69, 18, 20, 45, 5, ST77XX_RED);
}

void afficherStart() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(28, 80);
  tft.println("Start");
  tft.setCursor(05, 105);
  tft.println(num_soft);

  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 18, 20, 45, 5, ST77XX_GREEN);
  tft.fillRoundRect(69, 18, 20, 45, 5, ST77XX_GREEN);
}

void afficherInit() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(25, 80);
  tft.println("Version");

  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 18, 20, 45, 5, ST77XX_BLUE);
  tft.fillRoundRect(69, 18, 20, 45, 5, ST77XX_BLUE);

  //tft.setTextSize(1);
  tft.setCursor(05, 110);
  tft.println(version);
}

void afficherPublicite() {
  tft.fillScreen(ST77XX_BLUE);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(0, 10);
  for (int i = 0; i < nbLignes; i++) {
    tft.setTextSize(i == 0 ? 2 : 1);
    tft.println(lignes[i]);
  }
  tft.setTextSize(2);
}

