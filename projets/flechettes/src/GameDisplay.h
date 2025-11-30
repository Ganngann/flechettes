#ifndef GAME_DISPLAY_H
#define GAME_DISPLAY_H

#include <Adafruit_ST7735.h>

class GameDisplay {
public:
    GameDisplay(int8_t cs, int8_t dc, int8_t rst);
    void begin();

    // Initial screens
    void showStartup();
    void showVersion();
    void showSetup();
    void showInit();
    void showStart();
    void showAd();

    // Status screens
    void showI2CScan();
    void showI2CResult(uint8_t address, bool success);
    void showHW111(bool status);
    void showScanResult(int nDevices);
    void showESPNowError();
    void showPCFStatus(uint8_t addr, bool success);
    void showNVRAMStatus(bool batteryOk);
    void showPeerStatus(bool success);

    // Main Counter Display
    void updateCounters(bool isConnected, uint16_t pending, uint16_t daily, uint16_t lifetime, uint16_t current, bool forceUpdate = false);

    // Reset Screens
    void showResetJournalierProgress(unsigned long duration, unsigned long maxDuration);
    void showResetTotauxProgress(unsigned long duration, unsigned long maxDuration);
    void showResetJournalierDone();
    void showResetTotauxDone();
    void showResetCancel();

    // Confirmation
    void showResetConfirmation(); // From afficherRemiseAZero

    Adafruit_ST7735& getTft() { return _tft; }

private:
    Adafruit_ST7735 _tft;

    // Cache for updateCounters to avoid flicker
    bool _lastConnected;
    uint16_t _lastPending;
    uint16_t _lastDaily;
    uint16_t _lastLifetime;
    uint16_t _lastCurrent;
    bool _firstRun;
};

#endif
