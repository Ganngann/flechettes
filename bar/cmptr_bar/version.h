#ifndef VERSION_H
#define VERSION_H
#include <Arduino.h>

#include <stdint.h>  // pour uint16_t


// Structure pour stocker texte + couleur
struct LigneTexte {
  const char* texte;
  uint16_t couleur;
  uint8_t tailleTexte;  // <-- taille de police
};

extern LigneTexte lignes[];
extern const int nbLignes;

 
extern uint8_t broadcastAddress[] ; // Adresse MAC du peer (ESP-NOW Jeu)

extern const char* version;
extern const char* num_serie;
extern const char* num_soft;



extern const int max_cred;
extern const int Val_TotCn;
extern const int Val_Cred;

extern const int Val_Pin_Env;
extern const int Val_Pin_Cre;
extern const int Val_Pin_Ann;

 
extern const int nbLignes;

extern const int LdRed1;
extern const int LdBlu1;


#endif
