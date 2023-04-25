#include "Failsafe.h"

Failsafe::Failsafe(
        WiFiManager *wifiManager,
        Settings *settings,
        uint8_t btn1_pin,
        uint8_t btn2_pin,
        uint8_t led_pin,
        bool buttons_inverted,
        bool led_inverted
) {
    this->_wifiManger = wifiManager;
    this->_settings = settings;
    this->_btn1_pin = btn1_pin;
    this->_btn2_pin = btn2_pin;
    this->_led_pin = led_pin;
    this->_buttons_inverted = buttons_inverted;
    this->_led_inverted = led_inverted;
    this->btn1_pressed = false;
    this->btn2_pressed = false;
}

void Failsafe::toggle_led(bool on) const {
    digitalWrite(this->_led_pin, on ^ !this->_led_inverted);
}

void Failsafe::read_buttons() {
    this->btn1_pressed = digitalRead(this->_btn1_pin) == !this->_buttons_inverted;
    this->btn2_pressed = digitalRead(this->_btn2_pin) == !this->_buttons_inverted;
}

void Failsafe::start_config_portal() {
    char ssid[10];
    sprintf(ssid, "ESP%06X", EspClass::getChipId());
    this->_wifiManger->setConfigPortalTimeout(300);
    this->_wifiManger->startConfigPortal(ssid);
    EspClass::reset();
}

void Failsafe::reset() {
    bool led_on = true;
    while (this->btn2_pressed) {
        delay(25);
        this->toggle_led(led_on);
        read_buttons();
        led_on = !led_on;
    }
    this->_wifiManger->resetSettings();
    this->_settings->reset();
    EspClass::reset();
}

bool Failsafe::handle_startup() {
    pinMode(this->_btn1_pin, INPUT_PULLUP);
    pinMode(this->_btn2_pin, INPUT_PULLUP);
    pinMode(this->_led_pin, OUTPUT);
    digitalWrite(this->_led_pin, this->_led_inverted);
    int counter = 0;
    bool led_on = true;
    read_buttons();
    if (this->btn1_pressed) {
        while (this->btn1_pressed && counter < 20) {
            delay(500);
            this->toggle_led(led_on);
            counter++;
            led_on = !led_on;
            read_buttons();
        }
        if (this->btn1_pressed) {
            this->start_config_portal();
            return true;
        }
    } else if (this->btn2_pressed) {
        while (this->btn2_pressed && counter < 50) {
            delay(200);
            this->toggle_led(led_on);
            counter++;
            led_on = !led_on;
            read_buttons();
        }
        if (this->btn2_pressed) {
            this->reset();
            return true;
        }
    }
    return false;
}
