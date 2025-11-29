 #include "setup.h"
#include "affichage.h"
#include "son.h"
#include "globales.h"

void initialiserModules() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  
  //afficherStart(); // Affichage de démarrage
  jouerSon(4);
}