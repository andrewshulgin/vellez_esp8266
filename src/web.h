#ifndef VELLEZ_ESP8266_WEB_H
#define VELLEZ_ESP8266_WEB_H

#include <Esp.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

#include "webpage.h"
#include "callbacks.h"
#include "def_limits.h"
#include "settings.h"

#define WEB_PORT 80

const uint8_t webpage[] PROGMEM = GZIPPED_WEBPAGE;

class Web {
public:
    Web();
    void begin(Settings &settings);
    void set_vellez_address_callback(void_uint8_callback_t callback);
    void set_vellez_gong_callback(void_bool_callback_t callback);
    void set_vellez_zones_callback(void_uint16_callback_t callback);
    void set_volume_callback(void_uint8_callback_t callback);
    void set_play_callback(void_uint8_callback_t callback);
    void set_stop_callback(void_callback_t callback);
    void set_reboot_callback(void_callback_t callback);
private:
    AsyncWebServer *server;
    Settings *_settings;
    const char *blank;
    const char *stars;
    void_uint8_callback_t vellez_address_callback;
    void_bool_callback_t vellez_gong_callback;
    void_uint16_callback_t vellez_zones_callback;
    void_uint8_callback_t volume_callback;
    void_uint8_callback_t play_callback;
    void_callback_t stop_callback;
    void_callback_t reboot_callback;
    static void handle_get_favicon(AsyncWebServerRequest *request);
    void handle_get_root(AsyncWebServerRequest *request);
    void handle_get_settings(AsyncWebServerRequest *request);
    void handle_patch_settings(AsyncWebServerRequest *request, JsonVariant &json);
    void handle_play(AsyncWebServerRequest *request, JsonVariant &json);
    void handle_stop(AsyncWebServerRequest *request);
    void handle_reboot(AsyncWebServerRequest *request);
    static void handle_not_found(AsyncWebServerRequest *request);
};

#endif // VELLEZ_ESP8266_WEB_H
