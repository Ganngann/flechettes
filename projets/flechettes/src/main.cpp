/*
 Flechette10 - Refactored to OOP
 */

#include <Arduino.h>
#include "GameController.h"

GameController gameController;

void setup() {
    gameController.setup();
}

void loop() {
    gameController.loop();
}
