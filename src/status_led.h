#ifndef VELLEZ_ESP8266_STATUS_LED_H
#define VELLEZ_ESP8266_STATUS_LED_H

#include <Arduino.h>
#include <Ticker.h>

#define STATUS_LED_INTERVAL 10
#define STATUS_LED_RESOLUTION 20

#define STATUS_LED_MODE_OFF 0
#define STATUS_LED_MODE_STEADY_ON 1
#define STATUS_LED_MODE_SHORT_FLASHES 2
#define STATUS_LED_MODE_STEADY_FLASHES 3

#define STATUS_LED_ON !inverted
#define STATUS_LED_OFF inverted

class StatusLED {
public:
    void begin(uint8_t led_pin, bool inverted_logic);
    void flash(uint8_t count = 0);
    void flash_slowly();
    void flash_fast();
    void toggle(bool on);
    void end();
private:
    Ticker ticker;
    uint8_t pin;
    bool inverted;
    uint8_t mode = STATUS_LED_MODE_OFF;
    uint8_t flash_count;
    uint8_t ticks;
    void tick();
};

#endif // VELLEZ_ESP8266_STATUS_LED_H
