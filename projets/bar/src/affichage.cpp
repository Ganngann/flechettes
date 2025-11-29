#include "affichage.h"
#include <WiFi.h>
#include "config.h"
#include "son.h"
#include "globales.h"

 

 

 
 
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


void afficherStart(Adafruit_ST7735 &tft) { 
  unsigned long debut = millis();
  unsigned long duree = 3000;
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(5, 80);
  tft.println(num_soft);
  tft.fillRoundRect(25, 10, 78, 60, 8, ST77XX_WHITE);
  tft.fillRoundRect(39, 18, 20, 45, 5, ST77XX_BLUE);
  tft.fillRoundRect(69, 18, 20, 45, 5, ST77XX_BLUE);
  tft.setCursor(05, 110);
  tft.println(version);
}
