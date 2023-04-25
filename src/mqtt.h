#ifndef VELLEZ_ESP8266_MQTT_H
#define VELLEZ_ESP8266_MQTT_H

#include <string>
#include <PubSubClient.h>

#include "callbacks.h"
#include "settings.h"

// 0x23
#define MQTT_COMMAND_DISABLE '#'
// 0x2A
#define MQTT_COMMAND_ENABLE '*'
// 0x2B
#define MQTT_COMMAND_START '+'
// 0x2E
#define MQTT_COMMAND_STOP '.'
// 0x40
#define MQTT_COMMAND_REBOOT '@'
// 0x41
#define MQTT_COMMAND_SET_ADDRESS 'A'
// 0x47
#define MQTT_COMMAND_SET_GONG 'G'
// 0x50
#define MQTT_COMMAND_SET_WEB_PASSWORD 'P'
// 0x54
#define MQTT_COMMAND_SET_GENERIC_TOPIC 'T'
// 0x55
#define MQTT_COMMAND_SET_WEB_USERNAME 'U'
// 0x56
#define MQTT_COMMAND_SET_VOLUME 'V'
// 0x59
#define MQTT_COMMAND_SET_TELEMETRY_TOPIC 'Y'
// 0x5A
#define MQTT_COMMAND_SET_ZONES 'Z'

// 0x30
#define MQTT_PARAM_NUMBER_BASE '0'
// 0x30
#define MQTT_PARAM_FALSE '0'
// 0x31
#define MQTT_PARAM_TRUE '1'

class MQTT {
public:
    MQTT();
    typedef std::function<void(uint8_t, bool, uint8_t, uint16_t)> mqtt_play_callback_t;
    void begin(Client &client, Settings &settings);
    bool connected();
    void change_generic_topic(char *new_topic);
    void change_telemetry_topic(char *new_topic);
    void process();
    void callback(const char *topic, uint8_t *payload, unsigned int length);
    void send(const char *topic, const char *message);
    void set_toggle_callback(void_bool_callback_t callback);
    void set_address_callback(void_uint8_callback_t callback);
    void set_gong_callback(void_bool_callback_t callback);
    void set_generic_topic_callback(void_str_callback_t callback);
    void set_telemetry_topic_callback(void_str_callback_t callback);
    void set_web_username_callback(void_str_callback_t callback);
    void set_web_password_callback(void_str_callback_t callback);
    void set_volume_callback(void_uint8_callback_t callback);
    void set_zones_callback(void_uint16_callback_t callback);
    void set_play_callback(mqtt_play_callback_t callback);
    void set_stop_callback(void_callback_t callback);
    void set_reboot_callback(void_callback_t callback);
    void end();
private:
    PubSubClient *pubSubClient;
    Settings *_settings;
    const char *_client_id;
    const char *_generic_topic;
    const char *_telemetry_topic;
    const char *_username;
    const char *_password;
    unsigned long last_reconnect_attempt;
    void_bool_callback_t toggle_mqtt_activation_callback;
    void_uint8_callback_t address_callback;
    void_bool_callback_t gong_callback;
    void_str_callback_t generic_topic_callback;
    void_str_callback_t telemetry_topic_callback;
    void_str_callback_t web_username_callback;
    void_str_callback_t web_password_callback;
    void_uint8_callback_t volume_callback;
    void_uint16_callback_t zones_callback;
    mqtt_play_callback_t play_callback;
    void_callback_t stop_callback;
    void_callback_t reboot_callback;
    bool reconnect();
};


#endif // VELLEZ_ESP8266_MQTT_H
