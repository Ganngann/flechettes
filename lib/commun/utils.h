#ifndef UTILS_H
#define UTILS_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <string>
#include <stdint.h>
// Use std::string as a drop-in for String in native tests
using String = std::string;
#endif

String formatMacAddress(const uint8_t *mac);

#endif
