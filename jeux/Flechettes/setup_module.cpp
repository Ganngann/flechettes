 #include "setup_module.h"
#include "module_affichage.h"
#include "module_son.h"

void initialiserModules() {
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);
  //afficherStart(); // Affichage de démarrage
  jouerSon(SON_DEMARRAGE);
}