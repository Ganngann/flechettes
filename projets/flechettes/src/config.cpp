#include "config.h"
#include <Arduino.h>


const char* version   = "31/10/2025";
const char* num_serie = "jeux/25/004";
const char* num_soft  = "Vers:10.00";

//uint8_t broadcastAddress[] = {   0x94, 0x54, 0xc5, 0x62, 0xf1, 0x24 };  // peer bar
// uint8_t broadcastAddress[] = { 0x78,0x1C,0x3C,0xA9,0x1A,0x74 };  // peer bar
uint8_t broadcastAddress[] = { 0x48,0xE7,0x29,0xAC,0x92,0xF0 };  // peer bar



// Texte d’info
const char* lignes[] = {
  "Commerciale",
  "",
  "Pole Games SA",
  "",
  "  polegames@gmail.com",
  "",
  "hardware & software",
  "",
  "  A. D  L." ,
  "",
  "   nig.administration",
  "",
  "@laposte.net"
};

const int nbLignes = sizeof(lignes) / sizeof(lignes[0]);




/*
 a copier broadcastAddress[] =
    N°    Bar                              Jeu                              Vente     Date
25/0002   78:1C:3C:A6:D8:C8                40:22:D8:F0:DE:AC                Stock  
25/0003   78:1c:3c:a8:e5:9c                48:e7:29:96:98:c4                Stock  
25/0004   0x68,0x25,0xdd,0xfd,0x94,0xf8    78:1c:3c:a4:c9:f8    Stock  
25/0005   0x78,0x1c,0x3c,0xa5,0x98,0x1c    0x78,0x1c,0x3c,0xa8,0x63,0xbc    Stock  


*/

