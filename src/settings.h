#ifndef VELLEZ_ESP8266_SETTINGS_H
#define VELLEZ_ESP8266_SETTINGS_H

#include <cstdint>
#include <cstdio>
#include <Esp.h>
#include <EEPROM.h>

#define EEPROM_START_ADDRESS 0
#define SETTINGS_STRING_SIZE 32
#define DEFAULT_VELLEZ_ADDRESS 0
#define DEFAULT_VOLUME 15
#define DEFAULT_GONG_ENABLED 0
#define DEFAULT_OTA_PORT 443
#define DEFAULT_MQTT_PORT 8883
#define DEFAULT_VELLEZ_ZONES 0xFFFF
#define DEFAULT_TIMEZONE "EET-2EEST,M3.5.0/3,M10.5.0/4"
#define DEFAULT_AP_SSID "Vellez Alert %06X"
#define DEFAULT_NTP_HOST "pool.ntp.org"
#define DEFAULT_OTA_HOST "iot.rmrf.co"
#define DEFAULT_OTA_URI "/.ota/firmware/vellez.bin"
#define DEFAULT_MQTT_HOST "iot.rmrf.co"
#define DEFAULT_MQTT_USERNAME nullptr
#define DEFAULT_MQTT_PASSWORD nullptr
#define DEFAULT_MQTT_GENERIC_TOPIC "vellez_test"
#define DEFAULT_MQTT_TELEMETRY_TOPIC "vellez_%06X"
#define DEFAULT_WEB_USERNAME "admin"
#define DEFAULT_WEB_PASSWORD "vellez_admin"

class Settings {
public:
    Settings();
    void begin();
    void save();
    void read();
    void reset();
    void process();

    void toggle_autocommit(bool enabled);
    void discard_changes();

    [[nodiscard]] uint8_t get_default_vellez_address() const;
    [[nodiscard]] uint8_t get_default_vellez_gong_enabled() const;
    [[nodiscard]] uint8_t get_default_volume() const;
    [[nodiscard]] uint16_t get_default_ota_port() const;
    [[nodiscard]] uint16_t get_default_mqtt_port() const;
    [[nodiscard]] uint16_t get_default_vellez_zones() const;
    const char *get_default_timezone();
    const char *get_default_ap_ssid();
    const char *get_default_ntp_host();
    const char *get_default_ota_host();
    const char *get_default_ota_uri();
    const char *get_default_mqtt_host();
    const char *get_default_mqtt_client_id();
    const char *get_default_mqtt_username();
    const char *get_default_mqtt_password();
    const char *get_default_mqtt_generic_topic();
    const char *get_default_mqtt_telemetry_topic();
    const char *get_default_web_username();
    const char *get_default_web_password();

    uint8_t get_vellez_address();
    [[nodiscard]] uint8_t get_vellez_gong_enabled() const;
    uint8_t get_volume();
    uint16_t get_ota_port();
    uint16_t get_mqtt_port();
    [[nodiscard]] uint16_t get_vellez_zones() const;
    const char *get_timezone();
    const char *get_ap_ssid();
    const char *get_ntp_host();
    const char *get_ota_host();
    const char *get_ota_uri();
    const char *get_mqtt_host();
    const char *get_mqtt_client_id();
    const char *get_mqtt_username();
    const char *get_mqtt_password();
    const char *get_mqtt_generic_topic();
    const char *get_mqtt_telemetry_topic();
    const char *get_web_username();
    const char *get_web_password();

    [[nodiscard]] bool is_vellez_address_overridden() const;
    [[nodiscard]] bool is_vellez_gong_enabled_overridden() const;
    [[nodiscard]] bool is_volume_overridden() const;
    [[nodiscard]] bool is_ota_port_overridden() const;
    [[nodiscard]] bool is_mqtt_port_overridden() const;
    [[nodiscard]] bool is_vellez_zones_overridden() const;
    [[nodiscard]] bool is_timezone_overridden();
    [[nodiscard]] bool is_ap_ssid_overridden();
    [[nodiscard]] bool is_ntp_host_overridden();
    [[nodiscard]] bool is_ota_host_overridden();
    [[nodiscard]] bool is_ota_uri_overridden();
    [[nodiscard]] bool is_mqtt_host_overridden();
    [[nodiscard]] bool is_mqtt_client_id_overridden();
    [[nodiscard]] bool is_mqtt_username_overridden();
    [[nodiscard]] bool is_mqtt_password_overridden();
    [[nodiscard]] bool is_mqtt_generic_topic_overridden();
    [[nodiscard]] bool is_mqtt_telemetry_topic_overridden();
    [[nodiscard]] bool is_web_username_overridden();
    [[nodiscard]] bool is_web_password_overridden();

    bool set_vellez_address(uint8_t);
    bool set_vellez_gong_enabled(uint8_t);
    bool set_volume(uint8_t);
    bool set_ota_port(uint16_t);
    bool set_mqtt_port(uint16_t);
    bool set_vellez_zones(uint16_t);
    bool set_timezone(const char *);
    bool set_ap_ssid(const char *);
    bool set_ntp_host(const char *);
    bool set_ota_host(const char *);
    bool set_ota_uri(const char *);
    bool set_mqtt_host(const char *);
    bool set_mqtt_client_id(const char *);
    bool set_mqtt_username(const char *);
    bool set_mqtt_password(const char *);
    bool set_mqtt_generic_topic(const char *);
    bool set_mqtt_telemetry_topic(const char *);
    bool set_web_username(const char *);
    bool set_web_password(const char *);

    void clear_vellez_address();
    void clear_vellez_gong_enabled();
    void clear_volume();
    void clear_ota_port();
    void clear_mqtt_port();
    void clear_vellez_zones();
    void clear_timezone();
    void clear_ap_ssid();
    void clear_ntp_host();
    void clear_ota_host();
    void clear_ota_uri();
    void clear_mqtt_host();
    void clear_mqtt_client_id();
    void clear_mqtt_username();
    void clear_mqtt_password();
    void clear_mqtt_generic_topic();
    void clear_mqtt_telemetry_topic();
    void clear_web_username();
    void clear_web_password();
private:
    EEPROMClass *_eeprom;
    bool autocommit;
    bool need_save;
    struct settings {
        uint8_t initialized;
        uint8_t vellez_address;
        uint8_t vellez_gong_enabled;
        uint8_t volume;
        uint64_t ota_port;
        uint64_t mqtt_port;
        uint16_t vellez_zones;
        uint8_t timezone[SETTINGS_STRING_SIZE];
        uint8_t ap_ssid[SETTINGS_STRING_SIZE];
        uint8_t ntp_host[SETTINGS_STRING_SIZE];
        uint8_t ota_host[SETTINGS_STRING_SIZE];
        uint8_t ota_uri[SETTINGS_STRING_SIZE];
        uint8_t mqtt_host[SETTINGS_STRING_SIZE];
        uint8_t mqtt_client_id[SETTINGS_STRING_SIZE];
        uint8_t mqtt_username[SETTINGS_STRING_SIZE];
        uint8_t mqtt_password[SETTINGS_STRING_SIZE];
        uint8_t mqtt_generic_topic[SETTINGS_STRING_SIZE];
        uint8_t mqtt_telemetry_topic[SETTINGS_STRING_SIZE];
        uint8_t web_username[SETTINGS_STRING_SIZE];
        uint8_t web_password[SETTINGS_STRING_SIZE];
    };
    settings default_settings;
    settings override_settings;
};


#endif // VELLEZ_ESP8266_SETTINGS_H
