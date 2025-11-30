#include "BarDisplay.h"

const char* BarDisplay::MESSAGES[] = {
  "Bienvenue !", "Profitez !", "Cheers !", "Happy Hour",
  "Flech ok !", "Bar ouvert", "Prenez ", "Ambiance"
};
const int BarDisplay::MSG_COUNT = sizeof(BarDisplay::MESSAGES) / sizeof(BarDisplay::MESSAGES[0]);

const BarDisplay::TextLine BarDisplay::LINES[] = {
  {" Tarif :", ST77XX_YELLOW, 2},
  {" ", ST77XX_WHITE, 1},
  {" 1 Credit", ST77XX_WHITE, 2},
  {"   = ", ST77XX_WHITE, 2},
  {" 0.50 E", ST77XX_WHITE, 2},
  {" ", ST77XX_WHITE, 1},
  {" ", ST77XX_WHITE, 1},
  {" ", ST77XX_WHITE, 1},
  {" Bonne ", ST77XX_YELLOW, 2},
  {" Soiree ", ST77XX_YELLOW, 2}
};
const int BarDisplay::LINES_COUNT = sizeof(BarDisplay::LINES) / sizeof(BarDisplay::LINES[0]);


BarDisplay::BarDisplay(int8_t cs, int8_t dc, int8_t rst)
    : _tft(cs, dc, rst), _initialized(false), _scrolling(false), _lastScrollTime(0), _scrollY(-20), _currentMsg("")
{
}

void BarDisplay::begin() {
    _tft.initR(INITR_GREENTAB);
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setRotation(3);
}

void BarDisplay::showStartup(const char* serial) {
    // Phase 1: Serial
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setCursor(10, 40);
    _tft.setTextColor(ST77XX_GREEN);
    _tft.setTextSize(2);
    _tft.println("Num Serie:");
    _tft.setCursor(10, 70);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.println(serial);
}

void BarDisplay::showStart() {
    // Phase 2: Start
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setCursor(10, 30);
    _tft.setTextColor(ST77XX_RED);
    _tft.setTextSize(2);
    _tft.println("Demarrage...");
    _tft.setCursor(10, 60);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.println("Veuillez");
    _tft.setCursor(10, 80);
    _tft.println("patienter");
}

void BarDisplay::update(int screenId, const BarData& data) {
    switch (screenId) {
        case 1:
            _drawScreen1(data.isConnected);
            break;
        case 3:
            _drawScreen3(data.totalCredits);
            break;
        case 4:
            _drawScreen4();
            break;
        case 5:
            _drawScreen5(data.totalCredits);
            break;
        case 6:
            _drawScreen6(data.dailyCredits, data.lifetimeCredits);
            break;
        case 8:
            _drawScreen8(data.totalCredits, data.annulExces, data.maxCred);
            break;
        case 9:
            _drawScreen9();
            break;
        case 20:
            _drawScreen20();
            break;
        case 21:
            _drawScreen21();
            break;
        default:
             _initialized = false;
             _scrolling = false;
             _scrollY = 0;
            break;
    }
}

void BarDisplay::_drawScreen1(bool isConnected) {
    if (isConnected) {
        if (!_initialized) {
            _tft.fillScreen(ST77XX_BLUE);
            _tft.fillRect(0, 100, _tft.width(), 40, ST77XX_BLUE);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.setCursor(10, 110);
            _tft.println("Bar onLine");
            _initialized = true;
        }

        _drawScrollingMessage();
    } else {
        if (!_initialized) {
            _tft.fillScreen(ST77XX_RED);
            _tft.setTextColor(ST77XX_YELLOW);
            _tft.setCursor(10, 60);
            _tft.println("Jeux off");
            _initialized = true;
            _scrolling = false;
        }
    }
}

void BarDisplay::_drawScrollingMessage() {
    unsigned long now = millis();
    if (!_scrolling) {
        _scrollY = 0;
        _currentMsg = MESSAGES[random(0, MSG_COUNT)];
        _scrolling = true;
        _lastScrollTime = now;
    }

    if (now - _lastScrollTime >= SCROLL_SPEED) {
        _lastScrollTime = now;

        _tft.fillRect(0, 0, _tft.width(), 100, ST77XX_BLUE);
        _tft.setCursor(10, _scrollY);
        _tft.setTextColor(ST77XX_WHITE);
        _tft.println(_currentMsg);
        _scrollY += 4;

        if (_scrollY > 80) {
            _scrolling = false;
            _tft.fillRect(0, 0, _tft.width(), 100, ST77XX_BLUE);
        }
    }
}

void BarDisplay::_drawScreen3(uint16_t totalCredits) {
    const float valCred = 0.50;

    _scrolling = false;
    _scrollY = 0;
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setCursor(3, 6);
    _tft.setTextColor(ST77XX_YELLOW);
    _tft.println("Cred/jeux ");
    _tft.setCursor(41, 34);
    _tft.print(totalCredits * valCred);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.setCursor(10, 60);
    _tft.print(" En Euro:");
    _tft.setTextSize(5);
    _tft.setCursor(40, 85);
    _tft.print(totalCredits);
    _tft.setTextSize(2);
}

void BarDisplay::_drawScreen4() {
    _scrolling = false;
    _scrollY = 0;
    _tft.fillScreen(ST77XX_BLUE);

    int y = 5;
    int hText = 8;
    int space = 1;

    for (int i = 0; i < LINES_COUNT; i++) {
        _tft.setTextColor(LINES[i].color);
        _tft.setTextSize(LINES[i].size);
        _tft.setCursor(0, y);
        _tft.print(LINES[i].text);
        y += hText * LINES[i].size + space;
    }
    _tft.setTextSize(2);
}

void BarDisplay::_drawScreen5(uint16_t totalCredits) {
    _scrolling = false;
    _scrollY = 0;
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setCursor(10, 10);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.print("Clear Cpt");
    _tft.setTextSize(4);
    _tft.setCursor(40, 42);
    _tft.setTextColor(ST77XX_BLUE);
    _tft.print(totalCredits);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.setTextSize(2);
    _tft.setCursor(0, 93);
    _tft.print("..........");
    delay(3000);
    _tft.fillScreen(ST77XX_BLACK);
}

void BarDisplay::_drawScreen6(uint16_t daily, uint16_t lifetime) {
    _scrolling = false;
    _scrollY = 0;
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setTextColor(ST77XX_YELLOW);
    _tft.setCursor(2, 5);
    _tft.println("Journalier");
    _tft.setCursor(10, 30);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.println(daily);
    _tft.setTextColor(ST77XX_YELLOW);
    _tft.setCursor(5, 55);
    _tft.println("Total");
    _tft.setTextColor(ST77XX_WHITE);
    _tft.setCursor(10, 82);
    _tft.println(lifetime);
    _tft.setTextColor(ST77XX_YELLOW);
    _tft.setTextSize(1);
    _tft.setCursor(10, 110);
    _tft.println("En Euros");
    _tft.setTextSize(2);
}

void BarDisplay::_drawScreen8(uint16_t totalCredits, bool exces, uint16_t maxCred) {
    _scrolling = false;
    _scrollY = 0;
    _tft.fillScreen(ST77XX_RED);
    _tft.setCursor(10, 2);
    _tft.setTextColor(ST77XX_YELLOW);
    _tft.print("Operation");
    _tft.setTextSize(4);
    _tft.setCursor(48, 25);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.print(totalCredits);
    _tft.setTextSize(2);
    _tft.setCursor(10, 59);
    if (exces) {
        _tft.setTextColor(ST77XX_WHITE);
        _tft.println(String("Euro > ") + maxCred);
    }
    _tft.setCursor(10, 83);
    _tft.println("Credits");
    _tft.setCursor(10, 103);
    _tft.println("annules !");
}

void BarDisplay::_drawScreen9() {
    _scrolling = false;
    _scrollY = 0;
    _tft.fillScreen(ST77XX_RED);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.setCursor(0, 20);
    _tft.setTextSize(2);
    _tft.println("Jeu distant");
    _tft.setCursor(25, 45);
    _tft.println("eteint");
    _tft.setCursor(10, 80);
    _tft.setTextColor(ST77XX_YELLOW);
    _tft.println("Credit");
    _tft.setCursor(10, 100);
    _tft.println("desactive");
    _tft.setTextSize(2);
}

void BarDisplay::_drawScreen20() {
    _tft.fillScreen(ST77XX_BLACK);
    _tft.setTextColor(ST77XX_WHITE);
    _tft.setTextSize(2);
    _tft.setCursor(10, 40);
    _tft.println("Transfert");
    _tft.println(" reussi");
}

void BarDisplay::_drawScreen21() {
    _tft.fillScreen(ST77XX_RED);
    _tft.setTextSize(2);
    _tft.setCursor(10, 40);
    _tft.println("   Jeu");
    _tft.println(" ");
    _tft.println(" OFF line");
}
