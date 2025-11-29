# Guide de Configuration pour le Développement (PlatformIO)

Ce projet utilise **PlatformIO** pour la gestion des dépendances et la compilation.

## Structure du Projet (Restructurée)

Le projet est organisé sous le dossier `projets/` :

*   `projets/bar/` : Contrôleur du bar (anciennement `cmptr_bar`).
*   `projets/flechettes/` : Jeu de fléchettes (anciennement `Flechettes`).
*   `lib/commun/` : Bibliothèque partagée (code commun).

Chaque projet suit la structure standard PlatformIO :
*   `src/` : Code source (`.cpp`).
*   `include/` : En-têtes (`.h`).

## Installation et Lancement

Pour travailler sur un module (exemple : `projets/bar`) :

1.  Ouvrez VS Code.
2.  Ouvrez le dossier racine du dépôt ou le dossier spécifique du projet (`projets/bar`).
3.  PlatformIO détectera le projet.

## Nomenclature

*   Fichiers en *snake_case* (`son.cpp`, `affichage.cpp`).
*   Point d'entrée : `main.cpp`.
*   Configuration : `config.h`.

## Commandes

*   **Compiler** : `pio run` (depuis le dossier du projet).
*   **Téléverser** : `pio run --target upload`.
