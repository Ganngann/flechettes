#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "config.h"
#include "serie.h"
#include <utils.h>

// Déclarations externes
extern uint8_t broadcastAddress[];

void afficherNumSerie(Adafruit_ST7735 &tft) {
  // Récupération de l'identifiant unique
  uint64_t chipID = ESP.getEfuseMac();

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
  tft.println("By Aut. de Longdoz.srl");
  tft.setTextColor(ST77XX_WHITE);
  
  // No delay, no sound. Caller handles timing and sound.
}
