#include "buttons.h"

Buttons::Buttons(int btn1_pin, int btn2_pin, bool inverted) {
    _handle_buttons = false;
    _inverted = inverted;
    btn1.pin = btn1_pin;
    btn2.pin = btn2_pin;
    btn1.stable_state = inverted;
    btn2.stable_state = inverted;
    btn1.last_state = inverted;
    btn2.last_state = inverted;
    btn1.last_change = 0;
    btn2.last_change = 0;
    btn1.callback = nullptr;
    btn2.callback = nullptr;
}

void Buttons::begin() {
    pinMode(btn1.pin, INPUT_PULLUP);
    pinMode(btn2.pin, INPUT_PULLUP);
}

void Buttons::set_btn1_callback(void_callback_t callback) {
    btn1.callback = callback;
}

void Buttons::set_btn2_callback(void_callback_t callback) {
    btn2.callback = callback;
}

void Buttons::process() {
    if (!_handle_buttons)
        return;
    if (has_been_clicked(btn1)) {
        btn1.callback();
    }
    if (has_been_clicked(btn2)) {
        btn2.callback();
    }
}

bool Buttons::has_been_clicked(button btn) {
    int current_state = digitalRead(btn.pin);
    if (current_state != btn.last_state) {
        btn.last_change = millis();
    }
    if ((millis() - btn.last_change) > DEBOUNCE_DELAY) {
        if (current_state != btn.stable_state) {
            btn.stable_state = current_state;
            if (btn.stable_state == _inverted) {
                return true;
            }
        }
    }
    return false;
}
