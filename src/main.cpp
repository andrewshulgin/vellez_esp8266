#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <SoftwareSerial.h>

#define WEBSERVER_H

#include "status_led.h"
#include "vellez.h"
#include "tls.h"
#include "ota.h"
#include "mqtt.h"
#include "dfplayer.h"
#include "failsafe.h"
#include "settings.h"
#include "web.h"

#define STATUS_LED_PIN LED_BUILTIN
//#define STATUS_LED_PIN 2
#define VELLEZ_RS485_TX_EN_PIN 4
#define AUDIO_OUT_EN_PIN 5
#define DFPLAYER_TX_PIN 12
#define DFPLAYER_RX_PIN 13
#define BUTTON_1_PIN 14
#define BUTTON_2_PIN 16

#define DFPLAYER_BAUD_RATE 9600

#define FIRMWARE_VERSION "0.1"

bool time_synced = false;
bool update_checked = false;

bool mqtt_initialized = false;
bool mqtt_activation_enabled = true;

bool reboot_pending = false;
bool stop_pending = false;
bool start_pending = false;

bool activated_locally = false;
bool vellez_active = false;
bool playing = false;

uint8_t message;

BearSSL::X509List x509(trusted_tls_ca);

WiFiManager wifiManager;
Settings settings;
Failsafe failsafe(&wifiManager, &settings, BUTTON_1_PIN, BUTTON_2_PIN, STATUS_LED_PIN, true);
StatusLED statusLed;
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
        if (vellez_active) {
            dfPlayer.play(message);
            start_pending = false;
            playing = true;
        } else {
            vellez.activate();
        }
    }
    if (!start_pending && !playing) {
        vellez.deactivate();
    }
}

void time_sync_callback() {
    time_synced = true;
}

void vellez_callback(bool active) {
    vellez_active = active;
    if (!active) {
        vellez.set_gong(settings.get_vellez_gong_enabled());
        vellez.set_zones(settings.get_vellez_zones());
    }
}

void dfplayer_callback(bool state) {
    playing = state;
}

void mqtt_toggle_callback(bool enabled) {
    mqtt_activation_enabled = enabled;
}

void mqtt_address_callback(uint8_t address) {
    if (address > 4) {
        return;
    }
    settings.set_vellez_address(address);
    vellez.set_address(address);
}

void mqtt_gong_callback(bool gong) {
    settings.set_vellez_gong_enabled(gong);
    vellez.set_gong(gong);
}

void mqtt_generic_topic_callback(char *topic, uint length) {
    if (topic != nullptr && length == 0) {
        return;
    }
    settings.set_mqtt_generic_topic(topic);
    mqtt.change_generic_topic(topic);
}

void mqtt_telemetry_topic_callback(char *topic, uint length) {
    if (topic != nullptr && length == 0) {
        return;
    }
    settings.set_mqtt_telemetry_topic(topic);
    mqtt.change_telemetry_topic(topic);
}

void mqtt_web_username_callback(char *username, uint length) {
    if (username != nullptr && length == 0) {
        return;
    }
    settings.set_web_username(username);
}

void mqtt_web_password_callback(char *password, uint length) {
    if (password != nullptr && length == 0) {
        return;
    }
    settings.set_web_password(password);
}

void mqtt_volume_callback(uint8_t volume) {
    if (volume > 30) {
        volume = 30;
    }
    settings.set_volume(volume);
    dfPlayer.set_volume(volume);
}

void mqtt_zones_callback(uint16_t zones) {
    settings.set_vellez_zones(zones);
    vellez.set_zones(zones);
}

void mqtt_start_callback(uint8_t msg, bool override_gong, bool override_zones, bool gong, uint16_t zones) {
    if (mqtt_activation_enabled) {
        message = msg;
        start_pending = true;
        if (override_gong) {
            vellez.set_gong(gong);
        }
        if (override_zones) {
            vellez.set_zones(zones);
        }
        activated_locally = false;
        vellez.activate();
    }
}

void mqtt_stop_callback() {
    if (!activated_locally) {
        stop_pending = true;
    }
}

void reboot_callback() {
    reboot_pending = true;
}

void web_start_callback(uint8_t msg) {
    message = msg;
    start_pending = true;
    activated_locally = false;
    vellez.activate();
}

void web_stop_callback() {
    stop_pending = true;
}

void setup() {
    failsafe.handle_startup();
    statusLed.begin(STATUS_LED_PIN, true);
    settings.begin();
    update_status_led();
    wifiClientSecure.setTrustAnchors(&x509);
    wifiManager.setConfigPortalBlocking(false);
    wifiManager.autoConnect(settings.get_ap_ssid());
    configTime(settings.get_timezone(), settings.get_ntp_host());
    settimeofday_cb(time_sync_callback);
    Serial.begin(VELLEZ_BAUD_RATE);
    Serial.setDebugOutput(false);
    dfPlayerSerial.begin(DFPLAYER_BAUD_RATE);
    dfPlayer.begin(dfPlayerSerial);
    dfPlayer.set_callback(dfplayer_callback);
    dfPlayer.set_volume(settings.get_volume());
    vellez.begin(
            Serial,
            VELLEZ_RS485_TX_EN_PIN,
            settings.get_vellez_address(),
            settings.get_vellez_gong_enabled(),
            settings.get_vellez_zones()
    );
    vellez.set_callback(vellez_callback);
    mqtt.set_toggle_callback(mqtt_toggle_callback);
    mqtt.set_address_callback(mqtt_address_callback);
    mqtt.set_gong_callback(mqtt_gong_callback);
    mqtt.set_generic_topic_callback(mqtt_generic_topic_callback);
    mqtt.set_telemetry_topic_callback(mqtt_telemetry_topic_callback);
    mqtt.set_web_username_callback(mqtt_web_username_callback);
    mqtt.set_web_password_callback(mqtt_web_password_callback);
    mqtt.set_volume_callback(mqtt_volume_callback);
    mqtt.set_zones_callback(mqtt_zones_callback);
    mqtt.set_start_callback(mqtt_start_callback);
    mqtt.set_stop_callback(mqtt_stop_callback);
    mqtt.set_reboot_callback(reboot_callback);
    web.set_start_callback(web_start_callback);
    web.set_stop_callback(web_stop_callback);
    web.set_reboot_callback(reboot_callback);
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
            mqtt.begin(
                    &wifiClientSecure,
                    settings.get_mqtt_host(),
                    settings.get_mqtt_port(),
                    settings.get_mqtt_client_id(),
                    settings.get_mqtt_generic_topic(),
                    settings.get_mqtt_telemetry_topic(),
                    settings.get_mqtt_username(),
                    settings.get_mqtt_password()
            );
            mqtt_initialized = true;
        }
    }
    wifiManager.process();
    settings.process();
    vellez.process();
    if (mqtt_initialized) {
        mqtt.process();
    }
    dfPlayer.process();
    process_audio();
    update_status_led();
    if (reboot_pending && !start_pending && !playing) {
        EspClass::reset();
    }
}
