#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <stdint.h>
#include <stdbool.h>
#endif

// Structure pour contenir les données reçues/envoyées via ESP-NOW
typedef struct struct_message {
  uint16_t cp1;  // Incrément compteur / nbr de points envoyé
  uint16_t cp2;  // Compteur journalier
  uint16_t cp3;  // Compteur total
  bool fp1;
  bool fs1;
} struct_message;

#endif
