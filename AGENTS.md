# Guide de Configuration pour le Développement (PlatformIO)

Ce projet utilise **PlatformIO** pour la gestion des dépendances et la compilation, assurant que tous les développeurs utilisent les mêmes versions de bibliothèques.

## Prérequis

1.  **VS Code** : Télécharger et installer [Visual Studio Code](https://code.visualstudio.com/).
2.  **PlatformIO** : Installer l'extension "PlatformIO IDE" dans VS Code.

## Structure du Projet

Le projet contient deux modules principaux, chacun configuré comme un projet PlatformIO indépendant :

*   `bar/cmptr_bar/` : Contrôleur du bar.
*   `jeux/Flechettes/` : Récepteur du jeu de fléchettes.

## Installation et Lancement

Pour travailler sur un module (exemple : `bar/cmptr_bar`) :

1.  Ouvrez VS Code.
2.  Faites **File > Open Folder...** et sélectionnez le dossier racine du module (ex: `bar/cmptr_bar/`).
    *   *Note : Vous pouvez aussi ouvrir le dossier racine du dépôt (`/`), mais PlatformIO détectera mieux les projets si vous ouvrez les dossiers spécifiques ou si vous ajoutez les dossiers au workspace.*
3.  PlatformIO va automatiquement télécharger les bibliothèques et outils nécessaires (définis dans `platformio.ini`).

## Commandes Utiles

*   **Compiler** : Cliquez sur l'icône "Check" (✓) dans la barre d'état PlatformIO (en bas) ou exécutez `pio run`.
*   **Téléverser** : Cliquez sur l'icône "Flèche" (→) ou exécutez `pio run --target upload`.
*   **Moniteur Série** : Cliquez sur l'icône "Prise" ou exécutez `pio device monitor`.

## Gestion des Bibliothèques

Les bibliothèques sont listées dans le fichier `platformio.ini` de chaque module sous `lib_deps`.
Si vous devez ajouter une bibliothèque, ajoutez-la dans ce fichier pour qu'elle soit partagée avec les autres développeurs.

Exemple :
```ini
lib_deps =
    adafruit/Adafruit GFX Library @ ^1.11.9
    ...
```
