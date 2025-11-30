#ifndef BAR_DISPLAY_H
#define BAR_DISPLAY_H

#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>

struct BarData {
    uint16_t totalCredits;
    uint16_t dailyCredits;
    uint16_t lifetimeCredits;
    bool isConnected;
    bool annulExces;
    bool annulReset;
    uint16_t maxCred;
};

class BarDisplay {
public:
    BarDisplay(int8_t cs, int8_t dc, int8_t rst);
    void begin();

    // Specific screens
    void showStartup(const char* serial);
    void showStart();

    // Main update loop
    void update(int screenId, const BarData& data);

private:
    Adafruit_ST7735 _tft;

    bool _initialized;
    bool _scrolling;
    unsigned long _lastScrollTime;
    int _scrollY;
    const char* _currentMsg;

    // Scrolling messages config
    static const int SCROLL_SPEED = 50;
    static const char* MESSAGES[];
    static const int MSG_COUNT;

    struct TextLine {
        const char* text;
        uint16_t color;
        uint8_t size;
    };
    static const TextLine LINES[];
    static const int LINES_COUNT;

    void _drawScrollingMessage();
    void _drawScreen1(bool isConnected);
    void _drawScreen3(uint16_t totalCredits);
    void _drawScreen4(); // Price list
    void _drawScreen5(uint16_t totalCredits); // Clear Cpt
    void _drawScreen6(uint16_t daily, uint16_t lifetime); // Stats
    void _drawScreen8(uint16_t totalCredits, bool exces, uint16_t maxCred); // Operation annulée
    void _drawScreen9(); // Connexion absente
    void _drawScreen20(); // Transfert réussi
    void _drawScreen21(); // Transfert échoué
};

#endif
