#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SoftwareSerial.h>

#define WEBSERVER_H

#include "failsafe.h"
#include "status_led.h"
#include "buttons.h"
#include "vellez.h"
#include "dfplayer.h"
#include "settings.h"
#include "tls.h"
#include "ota.h"
#include "mqtt.h"
#include "web.h"

#ifdef EXTERNAL_LED
#define STATUS_LED_PIN 2
#else
#define STATUS_LED_PIN LED_BUILTIN
#endif
#define VELLEZ_RS485_TX_EN_PIN 4
#define AUDIO_OUT_EN_PIN 5
#define DFPLAYER_TX_PIN 12
#define DFPLAYER_RX_PIN 13
#define BUTTON_1_PIN 14
#define BUTTON_2_PIN 16

#ifndef LED_INVERTED
#define LED_INVERTED true
#endif

#ifndef BUTTONS_INVERTED
#define BUTTONS_INVERTED true
#endif

#define DFPLAYER_BAUD_RATE 9600

#define FIRMWARE_VERSION "v0.0.1"

bool time_synced = false;
bool update_checked = false;
bool mqtt_initialized = false;
bool vellez_active = false;
bool playing = false;

bool reboot_pending = false;
bool stop_pending = false;
bool start_pending = false;

uint16_t pending_track = 0;
bool pending_gong_enabled = false;
uint16_t pending_zones = 0;
uint8_t pending_volume = 0;

BearSSL::X509List x509(trusted_tls_ca);

WiFiManager wifiManager;
Settings settings;
Failsafe failsafe(&wifiManager, &settings, BUTTON_1_PIN, BUTTON_2_PIN, STATUS_LED_PIN, BUTTONS_INVERTED, LED_INVERTED);
StatusLED statusLed;
Buttons buttons(BUTTON_1_PIN, BUTTON_2_PIN, BUTTONS_INVERTED);
SoftwareSerial dfPlayerSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFPlayer dfPlayer;
Vellez vellez;
BearSSL::WiFiClientSecure wifiClientSecure;
MQTT mqtt;
Web web;

void update_status_led() {
    if (vellez_active) {
        statusLed.toggle(true);
    } else if (start_pending) {
        statusLed.flash_fast();
    } else if (mqtt.connected()) {
        statusLed.flash(1);
    } else if (time_synced) {
        statusLed.flash(2);
    } else if (WiFi.status() == WL_CONNECTED) {
        statusLed.flash(3);
    } else {
        statusLed.flash_slowly();
    }
}

void process_audio() {
    digitalWrite(AUDIO_OUT_EN_PIN, vellez_active);
    if (stop_pending) {
        dfPlayer.stop();
        start_pending = false;
        stop_pending = false;
        playing = false;
    }
    if (start_pending) {
        vellez.set_gong(pending_gong_enabled);
        vellez.set_zones(pending_zones);
        vellez.activate();
        if (vellez_active) {
            dfPlayer.set_volume(pending_volume);
            dfPlayer.play(pending_track);
            start_pending = false;
            playing = true;
        }
    }
    if (!start_pending && !playing) {
        vellez.deactivate();
    }
}

void mqtt_play_callback(uint16_t track, bool gong, uint8_t volume, uint16_t zones) {
    pending_track = track;
    pending_gong_enabled = gong;
    pending_volume = volume;
    pending_zones = zones;
    start_pending = true;
}

void start_playback(uint16_t track) {
    pending_track = track;
    pending_gong_enabled = settings.get_vellez_gong_enabled();
    pending_volume = settings.get_volume();
    pending_zones = settings.get_vellez_zones();
    start_pending = true;
}

void btn1_callback() {
    if (!start_pending && !playing) {
        start_playback(1);
    } else {
        stop_pending = true;
    }
}

void btn2_callback() {
    if (!start_pending && !playing) {
        start_playback(2);
    } else {
        stop_pending = true;
    }
}

void setup() {
#ifdef BUTTONS_CONNECTED
    failsafe.handle_startup();
#endif
    pinMode(AUDIO_OUT_EN_PIN, OUTPUT);
    statusLed.begin(STATUS_LED_PIN, LED_INVERTED);
    settings.begin();
    update_status_led();
    wifiClientSecure.setTrustAnchors(&x509);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.autoConnect(settings.get_ap_ssid());
    configTime(settings.get_timezone(), settings.get_ntp_host());
    settimeofday_cb([] { time_synced = true; });
#ifdef BUTTONS_CONNECTED
    buttons.begin();
#endif
    buttons.set_btn1_callback(btn1_callback);
    buttons.set_btn2_callback(btn2_callback);
    Serial.begin(VELLEZ_BAUD_RATE);
    Serial.setDebugOutput(false);
    dfPlayerSerial.begin(DFPLAYER_BAUD_RATE);
    dfPlayer.begin(dfPlayerSerial);
    dfPlayer.set_callback([](bool state) { playing = state; });
    dfPlayer.set_volume(settings.get_volume());
    vellez.begin(
            Serial,
            VELLEZ_RS485_TX_EN_PIN,
            settings.get_vellez_address(),
            settings.get_vellez_gong_enabled(),
            settings.get_vellez_zones()
    );
    vellez.set_callback([](bool active) { vellez_active = active; });
    mqtt.set_address_callback([](uint8_t address) { vellez.set_address(address); });
    mqtt.set_gong_callback([](bool gong) { vellez.set_gong(gong); });
    mqtt.set_volume_callback([](uint8_t volume) { dfPlayer.set_volume(volume); });
    mqtt.set_zones_callback([](uint16_t zones) { vellez.set_zones(zones); });
    mqtt.set_play_callback(mqtt_play_callback);
    mqtt.set_stop_callback([] { stop_pending = true; });
    mqtt.set_reboot_callback([] { reboot_pending = true; });
    web.set_vellez_address_callback({[](uint8_t addr) { vellez.set_address(addr); }});
    web.set_vellez_gong_callback({[](bool gong) { vellez.set_gong(gong); }});
    web.set_vellez_zones_callback({[](uint16_t zones) { vellez.set_zones(zones); }});
    web.set_volume_callback({[](uint8_t volume) { dfPlayer.set_volume(volume); }});
    web.set_play_callback([](uint16_t track) { start_playback(track); });
    web.set_stop_callback([] { stop_pending = true; });
    web.set_reboot_callback([] { reboot_pending = true; });
    web.begin(settings);
}

void loop() {
    if (time_synced && !playing && !start_pending) {
        if (!update_checked) {
            OTA::check(
                    settings.get_ota_host(),
                    settings.get_ota_port(),
                    settings.get_ota_uri(),
                    FIRMWARE_VERSION,
                    &wifiClientSecure
            );
            update_checked = true;
        }
        if (!mqtt_initialized) {
            mqtt.begin(wifiClientSecure, settings);
            mqtt_initialized = true;
        }
    }
#ifdef BUTTONS_CONNECTED
    buttons.process();
#endif
    wifiManager.process();
    settings.process();
    vellez.process();
    if (mqtt_initialized) {
        mqtt.process();
    }
    dfPlayer.process();
    process_audio();
    update_status_led();
    if (reboot_pending && !playing && !start_pending) {
        EspClass::reset();
    }
}
