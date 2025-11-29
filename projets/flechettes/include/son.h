//Flechettes
#pragma once

#ifndef MODULE_SON_H
#define MODULE_SON_H

void initialiserSon();
void jouerSon(unsigned char typeSon);

// Définir les constantes ici ou dans un autre fichier commun
#define SON_BIP        0
#define SON_SUCCES     1
#define SON_ERREUR     2
#define SON_DEMARRAGE  3
#define SON_BOUTON     4
#define SON_GRAVE      5

#endif
