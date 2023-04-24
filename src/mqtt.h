#ifndef VELLEZ_ESP8266_MQTT_H
#define VELLEZ_ESP8266_MQTT_H

#include <string>
#include <PubSubClient.h>

// 0x23
#define MQTT_COMMAND_DISABLE '#'
// 0x2A
#define MQTT_COMMAND_ENABLE '*'
// 0x2B
#define MQTT_COMMAND_START '+'
// 0x2E
#define MQTT_COMMAND_STOP '.'
// 0x30
#define MQTT_COMMAND_NUMBER_BASE '0'
// 0x30
#define MQTT_COMMAND_FALSE '0'
// 0x31
#define MQTT_COMMAND_TRUE '1'
// 0x39
#define MQTT_COMMAND_MSG_MAX '9'
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

class MQTT {
public:
    MQTT();
    typedef std::function<void(void)> callback_t;
    typedef std::function<void(bool)> bool_callback_t;
    typedef std::function<void(uint8_t)> uint8_callback_t;
    typedef std::function<void(char *, uint)> str_callback_t;
    typedef std::function<void(uint16_t)> uint16_callback_t;
    typedef std::function<void(uint8_t, bool, bool, uint16_t, uint8_t)> start_callback_t;
    void begin(
            Client *client,
            const char *host,
            uint16_t port,
            const char *client_id,
            const char *generic_topic,
            const char *telemetry_topic,
            const char *username,
            const char *password
    );
    bool connected();
    void change_generic_topic(char *new_topic);
    void change_telemetry_topic(char *new_topic);
    void process();
    void callback(const char *topic, uint8_t *payload, unsigned int length);
    void send(const char *topic, const char *message);
    void set_toggle_callback(bool_callback_t callback);
    void set_address_callback(uint8_callback_t callback);
    void set_gong_callback(bool_callback_t callback);
    void set_generic_topic_callback(str_callback_t callback);
    void set_telemetry_topic_callback(str_callback_t callback);
    void set_web_username_callback(str_callback_t callback);
    void set_web_password_callback(str_callback_t callback);
    void set_volume_callback(uint8_callback_t callback);
    void set_zones_callback(uint16_callback_t callback);
    void set_start_callback(start_callback_t callback);
    void set_stop_callback(callback_t callback);
    void set_reboot_callback(callback_t callback);
    void end();
private:
    PubSubClient *pubSubClient;
    const char *_client_id;
    const char *_generic_topic;
    const char *_telemetry_topic;
    const char *_username;
    const char *_password;
    unsigned long last_reconnect_attempt;
    bool_callback_t toggle_callback;
    uint8_callback_t address_callback;
    bool_callback_t gong_callback;
    str_callback_t generic_topic_callback;
    str_callback_t telemetry_topic_callback;
    str_callback_t web_username_callback;
    str_callback_t web_password_callback;
    uint8_callback_t volume_callback;
    uint16_callback_t zones_callback;
    start_callback_t start_callback;
    callback_t stop_callback;
    callback_t reboot_callback;
    bool reconnect();
};


#endif // VELLEZ_ESP8266_MQTT_H
