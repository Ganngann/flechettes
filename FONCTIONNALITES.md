# Documentation des Fonctionnalités du Système (Bar & Fléchettes)

Ce document décrit en détail les fonctionnalités des deux modules constituant le système de gestion de crédits à distance : le module **Bar** (Télécommande) et le module **Fléchettes** (Récepteur).

## Vue d'ensemble
Le système permet de créditer une machine de jeu (fléchettes) à distance depuis un comptoir (bar). Les deux modules communiquent sans fil via le protocole ESP-NOW (basé sur Wi-Fi, sans routeur).

---

## 1. Module Bar (Télécommande / Émetteur)
Ce module est situé au comptoir. Il permet au gérant d'ajouter des crédits, de les envoyer à la machine et de consulter les statistiques (compteurs) reçus de la machine.

### Matériel
*   **Contrôleur :** ESP32 (Type Wemos/DevKit).
*   **Affichage :** Écran LCD TFT couleur (ST7735).
*   **Interface :** 3 Boutons poussoirs, Buzzer, LEDs de statut (Rouge/Bleu).

### Fonctionnalités

#### 🟢 Démarrage et Connexion
*   **Initialisation :** Au démarrage, le module vérifie ses composants et initialise la connexion ESP-NOW.
*   **Indication de Connexion :**
    *   **Connecté :** L'écran affiche "Bar onLine" (sur fond bleu). Les messages défilants sont actifs.
    *   **Déconnecté :** L'écran affiche "Jeux off" (sur fond rouge) et la LED Rouge clignote pour alerter.

#### 🎮 Gestion des Crédits
*   **Ajout de Crédit (Bouton 2 - Milieu) :**
    *   Chaque appui ajoute un crédit au compteur local (`TotCn`).
    *   Joue un son de validation.
    *   Met à jour l'affichage avec le nombre de crédits et leur valeur en Euros (basé sur `Val_Cred`).
*   **Envoi de Crédit (Bouton 3 - Droite) :**
    *   Envoie le montant total des crédits accumulés vers le module Fléchettes.
    *   Affiche "Transfert reussi" (Fond Noir) ou "OFF line" (Fond Rouge) selon le résultat.
    *   Joue une mélodie de succès ou d'échec.
    *   La LED Bleue clignote pendant le transfert réussi.
*   **Annulation (Bouton 1 - Gauche) :**
    *   Si des crédits sont en attente (non envoyés), un appui annule tout (`TotCn = 0`).
    *   Affiche "Operation Credits annules !".
*   **Protection :** Si le nombre de crédits dépasse une limite (`max_cred`, ex: 10), le système alerte, annule les crédits et bloque temporairement l'envoi.

#### 📊 Consultation des Compteurs
*   **Affichage Compteurs (Bouton 1 - Gauche, si crédits = 0) :**
    *   Si aucun crédit n'est en cours, appuyer sur le bouton 1 affiche les compteurs reçus du module Fléchettes :
        *   **Journalier :** Nombre de crédits depuis la dernière remise à zéro journalière.
        *   **Total :** Nombre total de crédits (historique).
*   **Affichage Info / Pub (Bouton 3 - Droite, si crédits = 0) :**
    *   Affiche les informations commerciales ou techniques (configurées dans `config.cpp`).

---

## 2. Module Fléchettes (Récepteur / Jeu)
Ce module est installé dans la machine de jeu. Il remplace ou complète le monnayeur. Il reçoit les ordres de crédit et pilote physiquement la machine.

### Matériel
*   **Contrôleur :** ESP32.
*   **Stockage :** Module RTC DS1307 avec NVRAM (Sauvegarde des compteurs même sans pile, ou via pile bouton).
*   **Sorties :** Relais (pour simuler l'insertion de pièce), PCF8574 (Extension E/S).
*   **Interface :** Écran TFT, 3 Boutons de maintenance sur la carte.

### Fonctionnalités

#### 🟢 Démarrage Sécurisé
*   **Auto-Test :** Au démarrage, le module scanne le bus I2C, vérifie la présence du module RTC, du PCF8574 et l'intégrité de la mémoire NVRAM.
*   **Restauration :** Récupère les compteurs (Journalier, Total) depuis la mémoire non-volatile (NVRAM) pour ne rien perdre en cas de coupure de courant.

#### 💰 Réception et Validation des Crédits
*   **Réception ESP-NOW :** Écoute en permanence les messages du Bar.
*   **Traitement :**
    *   À la réception d'un ordre (ex: 3 crédits), il incrémente son compteur de crédits en attente (`Mcmptr1`).
    *   Il active le **Relais 2** (`rel02`) par impulsions successives (ex: 3 impulsions pour 3 crédits) pour créditer la machine de jeu.
    *   Il met à jour immédiatement les compteurs **Journalier** et **Total** en mémoire et les sauvegarde.
    *   Il renvoie les nouveaux compteurs au Bar pour mise à jour de l'affichage distant.

#### 🛠 Maintenance et Remise à Zéro (Boutons sur carte)
*   **Reset Journalier (Bouton 3 - Droite, Appui Long 3s) :**
    *   Affiche une barre de progression.
    *   Remet le compteur **Journalier** à zéro.
    *   Conserve le compteur Total.
    *   Envoie la mise à jour au Bar.
    *   Confirme par un écran Vert "Compteur journalier remis a zero".
*   **Reset Total (Bouton 1 - Gauche, Appui Long 3s) :**
    *   Remet **TOUS** les compteurs à zéro (Journalier + Total).
    *   Affiche une barre de progression orange.
    *   Confirme par un écran Orange "Tout les compteurs remis a zero".
*   **Mode Info / Pub (Bouton 2 - Milieu) :**
    *   Affiche les informations du fabricant/développeur sur l'écran local.

#### 🖥 Affichage Local
*   L'écran du module Fléchettes affiche en permanence :
    *   État de la connexion ("Connecte au bar" / "Non connecte").
    *   **Reçu :** Dernier montant reçu.
    *   **Tep (Journalier) :** Compteur temporaire.
    *   **Tot (Total) :** Compteur totalisateur.
    *   **Cre :** Crédits techniques.
    *   La couleur de fond change (Bleu = OK, Rouge = Déconnecté/Erreur).

---

## 3. Notes sur les fonctionnalités non implémentées ou absentes
Suite à l'analyse du code source actuel :

*   **Horloge / Heure :** Bien que le module RTC (DS1307) soit présent et testé pour la NVRAM (stockage), **l'affichage de l'heure ou de la date n'est pas implémenté** sur les écrans du Bar ou des Fléchettes. Le composant RTC est utilisé uniquement pour sa mémoire non-volatile.
*   **Enregistrement des Resets (Logs) :** Le système permet de remettre à zéro les compteurs, mais **il n'y a pas d'enregistrement (log) historique** de ces actions (ex: "Reset fait le JJ/MM/AAAA à HH:MM"). Les compteurs sont simplement écrasés à 0.

---

## Résumé Technique Global
*   **Communication :** Bidirectionnelle. Le Bar envoie les ordres de crédit. Le Jeu renvoie les accusés de réception et l'état de ses compteurs.
*   **Fiabilité :** Les données critiques (compteurs) sont stockées dans la NVRAM du module RTC (DS1307), garantissant leur conservation même si l'ESP32 redémarre ou si l'alimentation est coupée.
*   **Son :** Chaque action (Bouton, Erreur, Succès) est accompagnée d'un signal sonore spécifique (Buzzer).
