#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <stdint.h>
#include <stdbool.h>

// Structure du message échangé via ESP-NOW
typedef struct struct_message {
  uint16_t cp1;  // Incrément compteur (points envoyés)
  uint16_t cp2;  // Compteur journalier
  uint16_t cp3;  // Compteur total
  bool fp1;      // Flag P1
  bool fs1;      // Flag S1
} struct_message;

#endif
