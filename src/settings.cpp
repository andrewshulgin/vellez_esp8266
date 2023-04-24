#include "settings.h"

static void clear(uint8_t *dest) {
    memset(dest, 0xFF, SETTINGS_STRING_SIZE);
}

char *get_or_default(uint8_t *saved, uint8_t *def) {
    return (char *) (saved[0] != 0xFF ? saved : def);
}

bool is_overridden(const uint8_t *saved) {
    return saved[0] != 0xFF;
}

bool set_string(uint8_t *dest, const char *value) {
    if (value != nullptr && strlen(value) < SETTINGS_STRING_SIZE) {
        snprintf((char *) dest, SETTINGS_STRING_SIZE, "%s", value);
        return true;
    }
    return false;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-extra-args"

Settings::Settings() : default_settings(), override_settings() {
    this->_eeprom = new EEPROMClass();
    this->autocommit = true;
    this->need_save = false;

    default_settings.initialized = 1;
    default_settings.vellez_address = DEFAULT_VELLEZ_ADDRESS;
    default_settings.vellez_gong_enabled = DEFAULT_GONG_ENABLED;
    default_settings.volume = DEFAULT_VOLUME;
    default_settings.ota_port = DEFAULT_OTA_PORT;
    default_settings.mqtt_port = DEFAULT_MQTT_PORT;
    default_settings.vellez_zones = DEFAULT_VELLEZ_ZONES;
    sprintf((char *) default_settings.timezone, "%s", DEFAULT_TIMEZONE);
    sprintf((char *) default_settings.ap_ssid, DEFAULT_AP_SSID, EspClass::getChipId());
    sprintf((char *) default_settings.ntp_host, "%s", DEFAULT_NTP_HOST);
    sprintf((char *) default_settings.ota_host, "%s", DEFAULT_OTA_HOST);
    sprintf((char *) default_settings.ota_uri, "%s", DEFAULT_OTA_URI);
    sprintf((char *) default_settings.mqtt_host, "%s", DEFAULT_MQTT_HOST);
    sprintf((char *) default_settings.mqtt_client_id, "%06X", EspClass::getChipId());
    sprintf((char *) default_settings.mqtt_username, "%s",
            DEFAULT_MQTT_USERNAME == nullptr ? "" : DEFAULT_MQTT_USERNAME);
    sprintf((char *) default_settings.mqtt_password, "%s",
            DEFAULT_MQTT_PASSWORD == nullptr ? "" : DEFAULT_MQTT_PASSWORD);
    sprintf((char *) default_settings.mqtt_generic_topic, DEFAULT_MQTT_GENERIC_TOPIC, EspClass::getChipId());
    sprintf((char *) default_settings.mqtt_telemetry_topic, DEFAULT_MQTT_TELEMETRY_TOPIC, EspClass::getChipId());
    sprintf((char *) default_settings.web_username, DEFAULT_WEB_USERNAME);
    sprintf((char *) default_settings.web_password, DEFAULT_WEB_PASSWORD);
}

#pragma clang diagnostic pop

void Settings::begin() {
    _eeprom->begin(sizeof(override_settings));
    this->read();
    if (override_settings.initialized != 1) {
        this->reset();
        this->read();
    }
}

void Settings::toggle_autocommit(bool enabled) {
    this->autocommit = enabled;
}

void Settings::discard_changes() {
    this->need_save = false;
    this->read();
}

void Settings::save() {
    if (this->need_save) {
        _eeprom->put(EEPROM_START_ADDRESS, override_settings);
        _eeprom->commit();
    }
    this->need_save = false;
}

void Settings::read() {
    _eeprom->get(EEPROM_START_ADDRESS, override_settings);
}

void Settings::reset() {
    override_settings.initialized = 1;
    clear_vellez_address();
    clear_vellez_gong_enabled();
    clear_volume();
    clear_ota_port();
    clear_mqtt_port();
    clear_vellez_zones();
    clear_timezone();
    clear_ap_ssid();
    clear_ntp_host();
    clear_ota_host();
    clear_ota_uri();
    clear_mqtt_host();
    clear_mqtt_client_id();
    clear_mqtt_username();
    clear_mqtt_password();
    clear_mqtt_generic_topic();
    clear_mqtt_telemetry_topic();
    clear_web_username();
    clear_web_password();
    this->save();
}

void Settings::process() {
    if (autocommit) {
        this->save();
    }
}

uint8_t Settings::get_default_vellez_address() const {
    return default_settings.vellez_address;
}

uint8_t Settings::get_default_vellez_gong_enabled() const {
    return default_settings.vellez_gong_enabled;
}

uint8_t Settings::get_default_volume() const {
    return default_settings.volume;
}

uint16_t Settings::get_default_ota_port() const {
    return default_settings.ota_port;
}

uint16_t Settings::get_default_mqtt_port() const {
    return default_settings.mqtt_port;
}

uint16_t Settings::get_default_vellez_zones() const {
    return default_settings.vellez_zones;
}

const char *Settings::get_default_timezone() {
    return (char *) default_settings.timezone;
}

const char *Settings::get_default_ap_ssid() {
    return (char *) default_settings.ap_ssid;
}

const char *Settings::get_default_ntp_host() {
    return (char *) default_settings.ntp_host;
}

const char *Settings::get_default_ota_host() {
    return (char *) default_settings.ota_host;
}

const char *Settings::get_default_ota_uri() {
    return (char *) default_settings.ota_uri;
}

const char *Settings::get_default_mqtt_host() {
    return (char *) default_settings.mqtt_host;
}

const char *Settings::get_default_mqtt_client_id() {
    return (char *) default_settings.mqtt_client_id;
}

const char *Settings::get_default_mqtt_username() {
    return (char *) default_settings.mqtt_username;
}

const char *Settings::get_default_mqtt_password() {
    return (char *) default_settings.mqtt_password;
}

const char *Settings::get_default_mqtt_generic_topic() {
    return (char *) default_settings.mqtt_generic_topic;
}

const char *Settings::get_default_mqtt_telemetry_topic() {
    return (char *) default_settings.mqtt_telemetry_topic;
}

const char *Settings::get_default_web_username() {
    return (char *) default_settings.web_username;
}

const char *Settings::get_default_web_password() {
    return (char *) default_settings.web_password;
}

uint8_t Settings::get_vellez_address() {
    return override_settings.vellez_address != 0xFF ?
           override_settings.vellez_address : default_settings.vellez_address;
}

uint8_t Settings::get_vellez_gong_enabled() const {
    return override_settings.vellez_gong_enabled == 1;
}

uint8_t Settings::get_volume() {
    return override_settings.volume <= 30 ?
           override_settings.volume : default_settings.volume;
}

uint16_t Settings::get_ota_port() {
    return override_settings.ota_port != 0x0000 ?
           override_settings.ota_port : default_settings.ota_port;
}

uint16_t Settings::get_mqtt_port() {
    return override_settings.mqtt_port != 0x0000 ?
           override_settings.mqtt_port : default_settings.mqtt_port;
}

uint16_t Settings::get_vellez_zones() const {
    return override_settings.vellez_zones;
}

const char *Settings::get_timezone() {
    return get_or_default(override_settings.timezone, default_settings.timezone);
}

const char *Settings::get_ap_ssid() {
    return get_or_default(override_settings.ap_ssid, default_settings.ap_ssid);
}

const char *Settings::get_ntp_host() {
    return get_or_default(override_settings.ntp_host, default_settings.ntp_host);
}

const char *Settings::get_ota_host() {
    return get_or_default(override_settings.ota_host, default_settings.ota_host);
}

const char *Settings::get_ota_uri() {
    return get_or_default(override_settings.ota_uri, default_settings.ota_uri);
}

const char *Settings::get_mqtt_host() {
    return get_or_default(override_settings.mqtt_host, default_settings.mqtt_host);
}

const char *Settings::get_mqtt_client_id() {
    return get_or_default(override_settings.mqtt_client_id, default_settings.mqtt_client_id);
}

const char *Settings::get_mqtt_username() {
    return get_or_default(override_settings.mqtt_username, DEFAULT_MQTT_USERNAME);
}

const char *Settings::get_mqtt_password() {
    return get_or_default(override_settings.mqtt_password, DEFAULT_MQTT_PASSWORD);
}

const char *Settings::get_mqtt_generic_topic() {
    return get_or_default(override_settings.mqtt_generic_topic, default_settings.mqtt_generic_topic);
}

const char *Settings::get_mqtt_telemetry_topic() {
    return get_or_default(override_settings.mqtt_telemetry_topic, default_settings.mqtt_telemetry_topic);
}

const char *Settings::get_web_username() {
    return get_or_default(override_settings.web_username, default_settings.web_username);
}

const char *Settings::get_web_password() {
    return get_or_default(override_settings.web_password, default_settings.web_password);
}

bool Settings::is_vellez_address_overridden() const {
    return override_settings.vellez_address != 0xFF;
}

bool Settings::is_vellez_gong_enabled_overridden() const {
    return override_settings.vellez_gong_enabled != 0xFF;
}

bool Settings::is_volume_overridden() const {
    return override_settings.volume != 0xFF;
}

bool Settings::is_ota_port_overridden() const {
    return override_settings.ota_port != 0x0000;
}

bool Settings::is_mqtt_port_overridden() const {
    return override_settings.mqtt_port != 0x0000;
}

bool Settings::is_vellez_zones_overridden() const {
    return override_settings.vellez_zones != DEFAULT_VELLEZ_ZONES;
}

bool Settings::is_timezone_overridden() {
    return is_overridden(override_settings.timezone);
}

bool Settings::is_ap_ssid_overridden() {
    return is_overridden(override_settings.ap_ssid);
}

bool Settings::is_ntp_host_overridden() {
    return is_overridden(override_settings.ntp_host);
}

bool Settings::is_ota_host_overridden() {
    return is_overridden(override_settings.ota_host);
}

bool Settings::is_ota_uri_overridden() {
    return is_overridden(override_settings.ota_uri);
}

bool Settings::is_mqtt_host_overridden() {
    return is_overridden(override_settings.mqtt_host);
}

bool Settings::is_mqtt_client_id_overridden() {
    return is_overridden(override_settings.mqtt_client_id);
}

bool Settings::is_mqtt_username_overridden() {
    return is_overridden(override_settings.mqtt_username);
}

bool Settings::is_mqtt_password_overridden() {
    return is_overridden(override_settings.mqtt_password);
}

bool Settings::is_mqtt_generic_topic_overridden() {
    return is_overridden(override_settings.mqtt_generic_topic);
}

bool Settings::is_mqtt_telemetry_topic_overridden() {
    return is_overridden(override_settings.mqtt_telemetry_topic);
}

bool Settings::is_web_username_overridden() {
    return is_overridden(override_settings.web_username);
}

bool Settings::is_web_password_overridden() {
    return is_overridden(override_settings.web_password);
}

bool Settings::set_vellez_address(uint8_t addr) {
    if (addr > 4) {
        clear_vellez_address();
        return false;
    }
    override_settings.vellez_address = addr;
    need_save = true;
    return true;
}

bool Settings::set_vellez_gong_enabled(uint8_t gong) {
    if (gong > 1) {
        clear_vellez_gong_enabled();
        return false;
    }
    override_settings.vellez_gong_enabled = gong;
    need_save = true;
    return true;
}

bool Settings::set_volume(uint8_t volume) {
    if (volume > 30) {
        clear_volume();
        return false;
    }
    override_settings.volume = volume;
    need_save = true;
    return true;
}

bool Settings::set_ota_port(uint16_t port) {
    if (port == 0x0000) {
        clear_ota_port();
        return false;
    }
    override_settings.ota_port = port;
    need_save = true;
    return true;
}

bool Settings::set_mqtt_port(uint16_t port) {
    if (port == 0x0000) {
        clear_mqtt_port();
        return false;
    }
    override_settings.mqtt_port = port;
    need_save = true;
    return true;
}

bool Settings::set_vellez_zones(uint16_t zones) {
    override_settings.vellez_zones = zones;
    need_save = true;
    return true;
}

bool Settings::set_timezone(const char *tz) {
    if (tz == nullptr || !strlen(tz)) {
        clear_timezone();
    } else if (set_string(override_settings.timezone, tz)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_ap_ssid(const char *ssid) {
    if (ssid == nullptr || !strlen(ssid)) {
        clear_ap_ssid();
    } else if (set_string(override_settings.ap_ssid, ssid)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_ntp_host(const char *host) {
    if (host == nullptr || !strlen(host)) {
        clear_ntp_host();
    } else if (set_string(override_settings.ntp_host, host)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_ota_host(const char *host) {
    if (host == nullptr || !strlen(host)) {
        clear_ota_host();
    } else if (set_string(override_settings.ota_host, host)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_ota_uri(const char *uri) {
    if (uri == nullptr || !strlen(uri)) {
        clear_ota_uri();
    } else if (set_string(override_settings.ota_uri, uri)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_mqtt_host(const char *host) {
    if (host == nullptr || !strlen(host)) {
        clear_mqtt_host();
    } else if (set_string(override_settings.mqtt_host, host)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_mqtt_client_id(const char *client_id) {
    if (client_id == nullptr || !strlen(client_id)) {
        clear_mqtt_client_id();
    } else if (set_string(override_settings.mqtt_client_id, client_id)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_mqtt_username(const char *username) {
    if (username == nullptr || !strlen(username)) {
        clear_mqtt_username();
    } else if (set_string(override_settings.mqtt_username, username)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_mqtt_password(const char *password) {
    if (password == nullptr || !strlen(password)) {
        clear_mqtt_password();
    } else if (set_string(override_settings.mqtt_password, password)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_mqtt_generic_topic(const char *topic) {
    if (topic == nullptr || !strlen(topic)) {
        clear_mqtt_generic_topic();
    } else if (set_string(override_settings.mqtt_generic_topic, topic)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_mqtt_telemetry_topic(const char *topic) {
    if (topic == nullptr || !strlen(topic)) {
        clear_mqtt_telemetry_topic();
    } else if (set_string(override_settings.mqtt_telemetry_topic, topic)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_web_username(const char *username) {
    if (username == nullptr || !strlen(username)) {
        clear_web_username();
    } else if (set_string(override_settings.web_username, username)) {
        need_save = true;
        return true;
    }
    return false;
}

bool Settings::set_web_password(const char *password) {
    if (password == nullptr || !strlen(password)) {
        clear_web_password();
    } else if (set_string(override_settings.web_password, password)) {
        need_save = true;
        return true;
    }
    return false;
}

void Settings::clear_vellez_address() {
    override_settings.vellez_address = 0xFF;
    override_settings.vellez_address = 0xFF;
    need_save = true;
}

void Settings::clear_vellez_gong_enabled() {
    override_settings.vellez_gong_enabled = 0xFF;
    need_save = true;
}

void Settings::clear_volume() {
    override_settings.volume = 0xFF;
    need_save = true;
}

void Settings::clear_ota_port() {
    override_settings.ota_port = 0x0000;
    need_save = true;
}

void Settings::clear_mqtt_port() {
    override_settings.mqtt_port = 0x0000;
    need_save = true;
}

void Settings::clear_vellez_zones() {
    override_settings.vellez_zones = DEFAULT_VELLEZ_ZONES;
    need_save = true;
}

void Settings::clear_timezone() {
    clear(override_settings.timezone);
    need_save = true;
}

void Settings::clear_ap_ssid() {
    clear(override_settings.ap_ssid);
    need_save = true;
}

void Settings::clear_ntp_host() {
    clear(override_settings.ntp_host);
    need_save = true;
}

void Settings::clear_ota_host() {
    clear(override_settings.ota_host);
    need_save = true;
}

void Settings::clear_ota_uri() {
    clear(override_settings.ota_uri);
    need_save = true;
}

void Settings::clear_mqtt_host() {
    clear(override_settings.mqtt_host);
    need_save = true;
}

void Settings::clear_mqtt_client_id() {
    clear(override_settings.mqtt_client_id);
    need_save = true;
}

void Settings::clear_mqtt_username() {
    clear(override_settings.mqtt_username);
    need_save = true;
}

void Settings::clear_mqtt_password() {
    clear(override_settings.mqtt_password);
    need_save = true;
}

void Settings::clear_mqtt_generic_topic() {
    clear(override_settings.mqtt_generic_topic);
    need_save = true;
}

void Settings::clear_mqtt_telemetry_topic() {
    clear(override_settings.mqtt_telemetry_topic);
    need_save = true;
}

void Settings::clear_web_password() {
    clear(override_settings.web_password);
    need_save = true;
}

void Settings::clear_web_username() {
    clear(override_settings.web_username);
    need_save = true;
}
