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

## Tests et Vérification Obligatoire

Une suite de tests obligatoires a été mise en place. **Avant de soumettre tout changement, vous DEVEZ exécuter le script de vérification.**

1.  Exécuter `./check_changes.sh` à la racine du projet.
2.  Le script lance les tests unitaires (natifs) et vérifie la compilation pour l'ESP32.
3.  Si le script échoue, corrigez les erreurs avant de soumettre.

### Ajouter des tests
*   Ajoutez des fichiers `.cpp` dans le dossier `test/test_native/` du projet concerné.
*   Utilisez la syntaxe Unity.
*   Pour tester la logique partagée (`lib/commun`), préférez des tests natifs (environnement `native`) qui s'exécutent rapidement sur la machine de développement.
