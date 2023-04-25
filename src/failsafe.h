#ifndef VELLEZ_ESP8266_FAILSAFE_H
#define VELLEZ_ESP8266_FAILSAFE_H

#include <Arduino.h>
#include <WiFiManager.h>

#include "settings.h"

class Failsafe {
public:
    Failsafe(
            WiFiManager *wifiManager,
            Settings *settings,
            uint8_t btn1_pin,
            uint8_t btn2_pin,
            uint8_t led_pin,
            bool buttons_inverted,
            bool led_inverted
    );
    bool handle_startup();
private:
    WiFiManager *_wifiManger;
    Settings *_settings;
    uint8_t _btn1_pin;
    uint8_t _btn2_pin;
    uint8_t _led_pin;
    bool _buttons_inverted;
    bool _led_inverted;
    bool btn1_pressed;
    bool btn2_pressed;
    void toggle_led(bool on) const;
    void read_buttons();
    void start_config_portal();
    void reset();
};

#endif // VELLEZ_ESP8266_FAILSAFE_H
