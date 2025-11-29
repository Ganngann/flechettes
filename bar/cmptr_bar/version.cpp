 

 #include "version.h"
 #include <Arduino.h>
 #include "config_cpu.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

 
// adresse jeux
//uint8_t broadcastAddress[] = { 0x40, 0x22, 0xd8, 0xf0, 0xde, 0xac }; // Adresse MAC du peer (ESP-NOW Jeu)
uint8_t broadcastAddress[] = { 0x48, 0xE7, 0x29, 0xB2, 0xE5, 0xFC }; // Adresse MAC du peer (ESP-NOW Jeu)

// Infos version
const char* version   = "31/10/2025";
const char* num_serie = "Bar/25/0005";
const char* num_soft  = "Vers:10.0";

// variable gestion cde crédits
const int max_cred  = 10;
const int Val_TotCn = 1;
const int Val_Cred  = 4;

// Broches selon TYPE_CPU
#if TYPE_CPU == 1
// version cpu standart 
const int Val_Pin_Env = 32; //sw 1  annulation & cpt
const int Val_Pin_Cre = 35; //sw 3  credit
const int Val_Pin_Ann = 34; //sw 2  envoi & pub

#elif TYPE_CPU == 2
//  plexi 2025
//  version cpu 3.0 blanc
const int Val_Pin_Env = 32; //sw 1 annulation & cpt
const int Val_Pin_Cre = 34; //sw 2 envoi & pub
const int Val_Pin_Ann = 35; //sw 3 credit

#elif TYPE_CPU == 3
// version nouveau cpu_bar 2025
const int Val_Pin_Env = 32;
const int Val_Pin_Cre = 34;
const int Val_Pin_Ann = 35;
//const int LdRed1 = 12;
//const int LdBlu1 = 33;

const int LdRed1 = 33;
const int LdBlu1 = 12;

#else
  #error "TYPE_CPU non défini ou incorrect"
#endif

// Texte d’info
 #include "version.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

LigneTexte lignes[] = {
  {"  Commerciale", ST77XX_YELLOW, 1},    
  {"", ST77XX_WHITE, 1},
  {"  Pole Games SA", ST77XX_WHITE, 1},
  {"", ST77XX_WHITE, 1},
  {"  polegames@gmail.com", ST77XX_WHITE, 1},
  {"", ST77XX_WHITE, 1},
  {"  hardware & software", ST77XX_YELLOW, 1},
  {"", ST77XX_WHITE, 1},
  {"  A. D. L.", ST77XX_WHITE, 1},        
  {"", ST77XX_WHITE, 1},
  {"  mirage.bureau@", ST77XX_GREEN, 1},
  {"", ST77XX_GREEN, 1},
  {"  laposte.net", ST77XX_GREEN, 1}
};

const int nbLignes = sizeof(lignes) / sizeof(lignes[0]);


 
 


 

 

