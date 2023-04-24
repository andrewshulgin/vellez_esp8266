#include "web.h"

Web::Web() {
    this->server = new AsyncWebServer(WEB_PORT);
    this->_settings = nullptr;
    this->blank = "\0";
    this->stars = "******\0";
}

void Web::begin(Settings &settings) {
    this->_settings = &settings;
    server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handle_get_root(request);
    });
    server->on("/favicon.ico", HTTP_GET, Web::handle_get_favicon);
    server->on("/settings.json", HTTP_GET, [this](AsyncWebServerRequest *request) {
        this->handle_get_settings(request);
    });
    server->addHandler(new AsyncCallbackJsonWebHandler(
            "/settings.json",
            [this](AsyncWebServerRequest *request, JsonVariant &json) {
                this->handle_patch_settings(request, json);
            })
    );
    server->addHandler(new AsyncCallbackJsonWebHandler(
            "/play",
            [this](AsyncWebServerRequest *request, JsonVariant &json) {
                this->handle_play(request, json);
            })
    );
    server->on("/stop", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handle_stop(request);
    });
    server->on("/reboot", HTTP_POST, [this](AsyncWebServerRequest *request) {
        this->handle_reboot(request);
    });
    server->onNotFound(handle_not_found);
    server->begin();
}

void Web::set_start_callback(Web::start_callback_t callback) {
    start_callback = std::move(callback);
}

void Web::set_stop_callback(Web::callback_t callback) {
    stop_callback = std::move(callback);
}

void Web::set_reboot_callback(Web::callback_t callback) {
    reboot_callback = std::move(callback);
}

void Web::handle_not_found(AsyncWebServerRequest *request) {
    request->send(404);
}

void Web::handle_get_favicon(AsyncWebServerRequest *request) {
    request->send(204);
}

void Web::handle_get_root(AsyncWebServerRequest *request) {
    if (!request->authenticate(_settings->get_web_username(), _settings->get_web_password()))
        return request->requestAuthentication();
    request->send_P(200, "text/html", webpage);
}

void Web::handle_get_settings(AsyncWebServerRequest *request) {
    if (!request->authenticate(_settings->get_web_username(), _settings->get_web_password()))
        return request->requestAuthentication();
    auto *response = new AsyncJsonResponse();
    JsonObject root = response->getRoot();
    JsonObject defaults = root.createNestedObject("defaults");
    JsonObject overrides = root.createNestedObject("overrides");

    defaults["tz"] = _settings->get_default_timezone();
    defaults["ap_ssid"] = _settings->get_default_ap_ssid();
    defaults["web_username"] = _settings->get_default_web_username();
    defaults["web_password"] = strlen(_settings->get_default_web_password()) > 0 ? stars : blank;
    defaults["ntp_host"] = _settings->get_default_ntp_host();
    defaults["ota_host"] = _settings->get_default_ota_host();
    defaults["ota_port"] = _settings->get_default_ota_port();
    defaults["ota_uri"] = _settings->get_default_ota_uri();
    defaults["mqtt_host"] = _settings->get_default_mqtt_host();
    defaults["mqtt_port"] = _settings->get_default_mqtt_port();
    defaults["mqtt_client_id"] = _settings->get_default_mqtt_client_id();
    defaults["mqtt_username"] = _settings->get_default_mqtt_username();
    defaults["mqtt_password"] = strlen(_settings->get_default_mqtt_password()) > 0 ? stars : blank;
    defaults["mqtt_generic_topic"] = _settings->get_default_mqtt_generic_topic();
    defaults["mqtt_telemetry_topic"] = _settings->get_default_mqtt_telemetry_topic();
    defaults["vellez_address"] = _settings->get_default_vellez_address() + 1;
    defaults["gong"] = (bool) _settings->get_default_vellez_gong_enabled();
    defaults["volume"] = _settings->get_default_volume();
    defaults["vellez_zones"] = _settings->get_default_vellez_zones();

    overrides["tz"] = (
            _settings->is_timezone_overridden() ?
            _settings->get_timezone() : nullptr
    );
    overrides["ap_ssid"] = (
            _settings->is_ap_ssid_overridden() ?
            _settings->get_ap_ssid() : nullptr
    );
    overrides["web_username"] = (
            _settings->is_web_username_overridden() ?
            _settings->get_web_username() : nullptr
    );
    overrides["web_password"] = (
            _settings->is_web_password_overridden() ?
            (strlen(_settings->get_web_password()) > 0 ? stars : blank) : nullptr
    );
    overrides["ntp_host"] = (
            _settings->is_ntp_host_overridden() ?
            _settings->get_ntp_host() : nullptr
    );
    overrides["ota_host"] = (
            _settings->is_ota_host_overridden() ?
            _settings->get_ota_host() : nullptr
    );
    if (_settings->is_ota_port_overridden()) {
        overrides["ota_port"] = _settings->get_ota_port();
    } else {
        overrides["ota_port"] = nullptr;
    }
    overrides["ota_uri"] = (
            _settings->is_ota_uri_overridden() ?
            _settings->get_ota_uri() : nullptr
    );
    overrides["mqtt_host"] = (
            _settings->is_mqtt_host_overridden() ?
            _settings->get_mqtt_host() : nullptr
    );
    if (_settings->is_mqtt_port_overridden()) {
        overrides["mqtt_port"] = _settings->get_mqtt_port();
    } else {
        overrides["mqtt_port"] = nullptr;
    }
    overrides["mqtt_client_id"] = (
            _settings->is_mqtt_client_id_overridden() ?
            _settings->get_mqtt_client_id() : nullptr
    );
    overrides["mqtt_username"] = (
            _settings->is_mqtt_username_overridden() ?
            _settings->get_mqtt_username() : nullptr
    );
    overrides["mqtt_password"] = (
            _settings->is_mqtt_password_overridden() ?
            (strlen(_settings->get_mqtt_password()) > 0 ? stars : blank) : nullptr
    );
    overrides["mqtt_generic_topic"] = (
            _settings->is_mqtt_generic_topic_overridden() ?
            _settings->get_mqtt_generic_topic() : nullptr
    );
    overrides["mqtt_telemetry_topic"] = (
            _settings->is_mqtt_telemetry_topic_overridden() ?
            _settings->get_mqtt_telemetry_topic() : nullptr
    );
    if (_settings->is_vellez_address_overridden()) {
        overrides["vellez_address"] = _settings->get_vellez_address();
    } else {
        overrides["vellez_address"] = nullptr;
    }
    if (_settings->is_vellez_gong_enabled_overridden()) {
        overrides["gong"] = (bool) _settings->get_vellez_gong_enabled();
    } else {
        overrides["gong"] = nullptr;
    }
    if (_settings->is_volume_overridden()) {
        overrides["volume"] = _settings->get_volume();
    } else {
        overrides["volume"] = nullptr;
    }
    if (_settings->is_vellez_zones_overridden()) {
        overrides["vellez_zones"] = _settings->get_vellez_zones();
    } else {
        overrides["vellez_zones"] = nullptr;
    }

    response->setLength();
    request->send(response);
}

bool put_string(
        JsonObject overrides,
        const char *name,
        const std::function<void(const char *)> &set,
        const std::function<void(void)> &clear
) {
    if (overrides[name].isNull()) {
        clear();
        return true;
    } else if (overrides[name].is<JsonString>()) {
        set(overrides[name].as<JsonString>().c_str());
        return true;
    }
    return false;
}

bool put_uint8(
        JsonObject overrides,
        const char *name,
        const std::function<void(uint8_t)> &set,
        const std::function<void(void)> &clear
) {
    if (overrides[name].isNull()) {
        clear();
        return true;
    } else if (overrides[name].is<uint8_t>()) {
        set(overrides[name].as<uint8_t>());
        return true;
    }
    return false;
}

bool put_uint16(
        JsonObject overrides,
        const char *name,
        const std::function<void(uint16_t)> &set,
        const std::function<void(void)> &clear
) {
    if (overrides[name].isNull()) {
        clear();
        return true;
    } else if (overrides[name].is<uint16_t>()) {
        set(overrides[name].as<uint16_t>());
        return true;
    }
    return false;
}

bool put_bool(
        JsonObject overrides,
        const char *name,
        const std::function<void(bool)> &set,
        const std::function<void(void)> &clear
) {
    if (overrides[name].isNull()) {
        clear();
        return true;
    } else if (overrides[name].is<bool>()) {
        set(overrides[name].as<bool>());
        return true;
    }
    return false;
}

void Web::handle_patch_settings(AsyncWebServerRequest *request, JsonVariant &json) {
    if (!request->authenticate(_settings->get_web_username(), _settings->get_web_password()))
        return request->requestAuthentication();
    JsonObject root = json.as<JsonObject>();
    JsonObject overrides;
    bool valid = true;
    if (!root.containsKey("overrides")) {
        return handle_get_settings(request);
    }
    if (!root["overrides"].is<JsonObject>()) {
        return request->send(400);
    }
    overrides = root["overrides"].as<JsonObject>();
    _settings->toggle_autocommit(false);
    if (valid && overrides.containsKey("tz")) {
        valid = put_string(
                overrides,
                "tz",
                [this](const char *value) { _settings->set_timezone(value); },
                [this]() { _settings->clear_timezone(); }
        );
    }
    if (valid && overrides.containsKey("ap_ssid")) {
        valid = put_string(
                overrides,
                "ap_ssid",
                [this](const char *value) { _settings->set_ap_ssid(value); },
                [this]() { _settings->clear_ap_ssid(); }
        );
    }
    if (valid && overrides.containsKey("web_username")) {
        valid = put_string(
                overrides,
                "web_username",
                [this](const char *value) { _settings->set_web_username(value); },
                [this]() { _settings->clear_web_username(); }
        );
    }
    if (valid && overrides.containsKey("web_password")) {
        valid = put_string(
                overrides,
                "web_password",
                [this](const char *value) { _settings->set_web_password(value); },
                [this]() { _settings->clear_web_password(); }
        );
    }
    if (valid && overrides.containsKey("ntp_host")) {
        valid = put_string(
                overrides,
                "ntp_host",
                [this](const char *value) { _settings->set_ntp_host(value); },
                [this]() { _settings->clear_ntp_host(); }
        );
    }
    if (valid && overrides.containsKey("ota_host")) {
        valid = put_string(
                overrides,
                "ota_host",
                [this](const char *value) { _settings->set_ota_host(value); },
                [this]() { _settings->clear_ota_host(); }
        );
    }
    if (valid && overrides.containsKey("ota_port")) {
        valid = put_uint16(
                overrides,
                "ota_port",
                [this](uint16_t value) { _settings->set_ota_port(value); },
                [this]() { _settings->clear_ota_port(); }
        );
    }
    if (valid && overrides.containsKey("ota_uri")) {
        valid = put_string(
                overrides,
                "ota_uri",
                [this](const char *value) { _settings->set_ota_uri(value); },
                [this]() { _settings->clear_ota_uri(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_host")) {
        valid = put_string(
                overrides,
                "mqtt_host",
                [this](const char *value) { _settings->set_mqtt_host(value); },
                [this]() { _settings->clear_mqtt_host(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_port")) {
        valid = put_uint16(
                overrides,
                "mqtt_port",
                [this](uint16_t value) { _settings->set_mqtt_port(value); },
                [this]() { _settings->clear_mqtt_port(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_client_id")) {
        valid = put_string(
                overrides,
                "mqtt_client_id",
                [this](const char *value) { _settings->set_mqtt_client_id(value); },
                [this]() { _settings->clear_mqtt_client_id(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_username")) {
        valid = put_string(
                overrides,
                "mqtt_username",
                [this](const char *value) { _settings->set_mqtt_username(value); },
                [this]() { _settings->clear_mqtt_username(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_password")) {
        valid = put_string(
                overrides,
                "mqtt_password",
                [this](const char *value) { _settings->set_mqtt_password(value); },
                [this]() { _settings->clear_mqtt_password(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_generic_topic")) {
        valid = put_string(
                overrides,
                "mqtt_generic_topic",
                [this](const char *value) { _settings->set_mqtt_generic_topic(value); },
                [this]() { _settings->clear_mqtt_generic_topic(); }
        );
    }
    if (valid && overrides.containsKey("mqtt_telemetry_topic")) {
        valid = put_string(
                overrides,
                "mqtt_telemetry_topic",
                [this](const char *value) { _settings->set_mqtt_telemetry_topic(value); },
                [this]() { _settings->clear_mqtt_telemetry_topic(); }
        );
    }
    if (valid && overrides.containsKey("vellez_address")) {
        valid = put_uint8(
                overrides,
                "vellez_address",
                [this](uint8 value) { _settings->set_vellez_address(value - 1); },
                [this]() { _settings->clear_vellez_address(); }
        );
    }
    if (valid && overrides.containsKey("gong")) {
        valid = put_bool(
                overrides,
                "gong",
                [this](bool value) { _settings->set_vellez_gong_enabled(value); },
                [this]() { _settings->clear_vellez_gong_enabled(); }
        );
    }
    if (valid && overrides.containsKey("volume")) {
        valid = put_uint8(
                overrides,
                "volume",
                [this](uint8 value) { _settings->set_volume(value); },
                [this]() { _settings->clear_volume(); }
        );
    }
    if (valid && overrides.containsKey("vellez_zones")) {
        valid = put_uint16(
                overrides,
                "vellez_zones",
                [this](uint16_t value) { _settings->set_vellez_zones(value); },
                [this]() { _settings->clear_vellez_zones(); }
        );
    }
    if (!valid) {
        _settings->discard_changes();
        _settings->toggle_autocommit(true);
        return request->send(400);
    }
    _settings->save();
    _settings->toggle_autocommit(true);
    handle_get_settings(request);
}

void Web::handle_play(AsyncWebServerRequest *request, JsonVariant &json) {
    if (!request->authenticate(_settings->get_web_username(), _settings->get_web_password()))
        return request->requestAuthentication();
    if (this->start_callback == nullptr) {
        return request->send(404);
    }
    JsonObject root = json.as<JsonObject>();
    if (!root.containsKey("track_num") || !root["track_num"].is<uint8_t>()) {
        return request->send(400);
    }
    uint8_t track_num = root["track_num"].as<uint8_t>();
    if (track_num > 9) {
        return request->send(400);
    }
    this->start_callback(track_num);
    return request->send(204);
}

void Web::handle_stop(AsyncWebServerRequest *request) {
    if (!request->authenticate(_settings->get_web_username(), _settings->get_web_password()))
        return request->requestAuthentication();
    if (this->stop_callback == nullptr) {
        return request->send(404);
    }
    this->stop_callback();
    return request->send(204);
}

void Web::handle_reboot(AsyncWebServerRequest *request) {
    if (!request->authenticate(_settings->get_web_username(), _settings->get_web_password()))
        return request->requestAuthentication();
    if (this->reboot_callback == nullptr) {
        return request->send(404);
    }
    this->reboot_callback();
    return request->send(204);
}
