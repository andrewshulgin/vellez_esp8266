#include "status_led.h"

void StatusLED::tick() {
    switch (mode) {
        case STATUS_LED_MODE_SHORT_FLASHES:
            digitalWrite(pin, STATUS_LED_OFF);
            if (flash_count > 0 && ticks % STATUS_LED_RESOLUTION == 0) {
                if (flash_count == 0xFF || ticks <= STATUS_LED_RESOLUTION * (flash_count - 1)) {
                    digitalWrite(pin, STATUS_LED_ON);
                }
            }
            break;
        case STATUS_LED_MODE_STEADY_FLASHES:
            if (ticks == 0) {
                digitalWrite(pin, STATUS_LED_ON);
            } else if (ticks == STATUS_LED_RESOLUTION * 5) {
                digitalWrite(pin, STATUS_LED_OFF);
            }
            break;
        case STATUS_LED_MODE_STEADY_ON:
            if (ticks == 0) {
                digitalWrite(pin, STATUS_LED_ON);
            }
            break;
        case STATUS_LED_MODE_OFF:
        default:
            if (ticks == 0) {
                digitalWrite(pin, STATUS_LED_OFF);
            }
            break;
    }
    ticks++;
    if (ticks > STATUS_LED_RESOLUTION * 10) {
        ticks = 0;
    }
}

void StatusLED::begin(uint8_t led_pin, bool inverted_logic) {
    pin = led_pin;
    inverted = inverted_logic;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, STATUS_LED_OFF);
    Ticker::callback_function_t f{[this] { tick(); }};
    ticker.attach_ms(STATUS_LED_INTERVAL, f);
    mode = STATUS_LED_MODE_OFF;
    flash_count = 0;
}

void StatusLED::flash(uint8_t count) {
    flash_count = count;
    mode = (flash_count == 0 ? STATUS_LED_MODE_STEADY_FLASHES : STATUS_LED_MODE_SHORT_FLASHES);
}

void StatusLED::flash_slowly() {
    flash(0);
}

void StatusLED::flash_fast() {
    flash(0xFF);
}

void StatusLED::toggle(bool on) {
    mode = on ? STATUS_LED_MODE_STEADY_ON : STATUS_LED_MODE_OFF;
}

void StatusLED::end() {
    ticker.detach();
    digitalWrite(pin, STATUS_LED_OFF);
}
