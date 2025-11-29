//Flechettes
#pragma once

#ifndef MODULE_NUM_SERIE_H
#define MODULE_NUM_SERIE_H

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"
#include <utils.h>

// Déclarations externes (à définir ailleurs)
extern uint8_t broadcastAddress[]; // Adresse MAC du peer (ESP-NOW)
void debugPrint(const String &msg);
//jouerSon(1);
void afficherNumSerie(Adafruit_ST7735 &tft) {
  // Effacer l'écran et définir les couleurs de texte
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);

  // Adresse MAC locale
  String mac = WiFi.macAddress();

  tft.setTextSize(1);
  tft.setCursor(0, 05);
  tft.println("Esp-32");

  tft.setCursor(0, 25);
  tft.println(mac);

  // Adresse MAC peer (ESP-NOW)
  String macString = formatMacAddress(broadcastAddress);
  tft.setCursor(0, 45);
  tft.println("Peer:");

  tft.setCursor(0, 65);
  tft.println(macString);

  tft.setCursor(0, 85);
  tft.print(num_serie);

 // Signature
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(0, 105);
  tft.println("By Aut. de Longdoz");
  tft.setTextColor(ST77XX_WHITE);

 

  delay(6000);  // Affichage temporaire

  // Nettoyage de l'écran final
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
 // jouerSon(1);
}

#endif // MODULE_NUM_SERIE_H

