# Système de Gestion de Crédits pour Jeu de Fléchettes

Ce projet est un système basé sur Arduino (ESP32) permettant de gérer à distance les crédits d'une machine de jeu de fléchettes depuis un bar. Il utilise le protocole **ESP-NOW** pour une communication sans fil fiable et directe entre le module du bar et celui du jeu.

## Fonctionnalités

*   **Gestion à distance :** Le barman peut ajouter des crédits sur le boîtier de commande et les envoyer sans fil à la machine de fléchettes.
*   **Comptabilité :** Suivi des crédits journaliers et totaux (en Euros ou unités). Les compteurs sont stockés de manière persistante (NVRAM) côté jeu et affichés côté bar.
*   **Feedback :** Écrans TFT sur les deux modules pour afficher l'état de la connexion, les crédits en cours, et les messages d'erreur.
*   **Hardware :** Utilisation de relais pour simuler l'insertion de pièces dans la machine.

## Architecture du Projet

Le projet est divisé en deux dossiers principaux :

### 1. Module Bar (`bar/cmptr_bar`)
Ce module est le contrôleur situé au bar.

*   **Matériel :**
    *   ESP32
    *   Écran TFT ST7735
    *   3 Boutons (Menu/Annuler, Ajouter Crédit, Envoyer)
    *   LEDs et Buzzer pour notifications
*   **Fonctions :**
    *   Ajout de crédits virtuels.
    *   Envoi des crédits vers le module Jeu.
    *   Réception et affichage des statistiques (Compteur Journalier, Compteur Total).
    *   Affichage de l'état de connexion ("Bar onLine" / "Jeux off").

### 2. Module Jeu (`jeux/Flechettes`)
Ce module est installé dans ou près de la machine de fléchettes.

*   **Matériel :**
    *   ESP32
    *   Écran TFT ST7735
    *   Relais (pour déclencher le crédit sur la machine)
    *   Module RTC (DS1307 ou DS3231) pour la sauvegarde des compteurs (NVRAM)
    *   PCF8574 (Extension d'entrées/sorties)
*   **Fonctions :**
    *   Réception des ordres de crédit depuis le Bar.
    *   Activation du relais pour créditer physiquement la machine.
    *   Sauvegarde sécurisée des compteurs (Journalier et Total).
    *   Envoi des mises à jour des compteurs vers le Bar.
    *   Boutons locaux pour la remise à zéro des compteurs (Journalier ou Total) avec confirmation.

## Installation et Configuration

### Prérequis Logiciels
*   Arduino IDE
*   Bibliothèques nécessaires (à installer via le gestionnaire de bibliothèques) :
    *   `Adafruit GFX Library`
    *   `Adafruit ST7735 and ST7789 Library`
    *   `ezButton`
    *   `I2C_eeprom`
    *   `PCF8574`
    *   `DS1307new` (ou équivalent pour RTC)

### Configuration des Adresses MAC
Le protocole ESP-NOW nécessite de connaître l'adresse MAC du destinataire.
1.  Obtenez l'adresse MAC de chaque ESP32 (utilisez un sketch scanner ou regardez les logs série au démarrage).
2.  Dans `cmptr_bar.ino` (Module Bar), mettez à jour `broadcastAddress` avec l'adresse MAC du Module Jeu.
3.  Dans `Flechettes10.ino` (Module Jeu), mettez à jour `broadcastAddress` avec l'adresse MAC du Module Bar.

### Branchements (Indicatif)
*Vérifiez les fichiers `.ino` et `config_cpu.h` pour les broches exactes selon votre version de PCB.*

*   **Écran TFT :** SPI (SCLK, MOSI, CS, DC, RST)
*   **Boutons :** Connectés aux broches définies (ex: `Val_Pin_Env`, `Val_Pin_Cre`, `Val_Pin_Ann` pour le bar).
*   **I2C (RTC, EEPROM, PCF8574) :** SDA (GPIO 21), SCL (GPIO 22) sur ESP32 standard.

## Utilisation

1.  **Au Bar :**
    *   Appuyez sur le bouton "Crédit" pour incrémenter le montant.
    *   Appuyez sur "Envoyer" pour transférer les crédits à la machine.
    *   Le système confirme le transfert par un son et un message.
2.  **Côté Jeu :**
    *   Le module reçoit le signal et active le relais.
    *   Les compteurs sont mis à jour et renvoyés au bar.
    *   Boutons de maintenance : Appui long pour remettre à zéro le compteur journalier ou total.

## Structure des Dossiers

*   `bar/` : Code source pour le module émetteur (Bar).
*   `jeux/` : Code source pour le module récepteur (Jeu de fléchettes).
