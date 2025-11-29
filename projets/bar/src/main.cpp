/*
 cmptr07 - Refactored to OOP
 */

#include <Arduino.h>
#include "BarController.h"

BarController barController;

void setup() {
    barController.setup();
}

void loop() {
    barController.loop();
}
