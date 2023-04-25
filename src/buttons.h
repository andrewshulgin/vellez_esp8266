#ifndef VELLEZ_ESP8266_BUTTONS_H
#define VELLEZ_ESP8266_BUTTONS_H

#include <Arduino.h>

#include "callbacks.h"

#define DEBOUNCE_DELAY 50

class Buttons {
public:
    Buttons(int btn1_pin, int btn2_pin, bool inverted);
    void begin();
    void set_btn1_callback(void_callback_t);
    void set_btn2_callback(void_callback_t);
    void process();
private:
    struct button {
        int pin;
        int stable_state;
        int last_state;
        unsigned long last_change;
        void_callback_t callback;
    };
    bool _inverted;
    bool _handle_buttons;
    button btn1;
    button btn2;
    bool has_been_clicked(button);
};

#endif // VELLEZ_ESP8266_BUTTONS_H
