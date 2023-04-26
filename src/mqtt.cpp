#include "mqtt.h"

void reverse(char *start, char *end) {
    while (end > start) {
        char ch = *end;
        *end-- = *start;
        *start++ = ch;
    }
}

MQTT::MQTT() {
    _settings = nullptr;
    _client_id = nullptr;
    _generic_topic = nullptr;
    _telemetry_topic = nullptr;
    _username = nullptr;
    _password = nullptr;
    pubSubClient = nullptr;
    last_reconnect_attempt = 0;
}

void MQTT::begin(Client &client, Settings &settings) {
    _settings = &settings;
    _client_id = _settings->get_mqtt_client_id();
    _generic_topic = _settings->get_mqtt_generic_topic();
    _telemetry_topic = _settings->get_mqtt_telemetry_topic();
    _username = _settings->get_mqtt_username();
    _password = _settings->get_mqtt_password();
    pubSubClient = new PubSubClient();
    pubSubClient->setServer(_settings->get_mqtt_host(), _settings->get_mqtt_port());
    pubSubClient->setClient(client);
    pubSubClient->setCallback(
            {[this](char *topic, uint8_t *payload, unsigned int length) {
                callback(topic, payload, length);
            }}
    );
    reconnect();
}

void MQTT::end() {
    if (pubSubClient != nullptr) {
        pubSubClient->disconnect();
    }
}

bool MQTT::connected() {
    return pubSubClient != nullptr && pubSubClient->connected();
}

bool MQTT::reconnect() {
    if (pubSubClient == nullptr) {
        return false;
    }
    if (pubSubClient->connect(_client_id, _username, _password)) {
        pubSubClient->publish(_telemetry_topic, "connected");
        pubSubClient->subscribe(_telemetry_topic);
        pubSubClient->subscribe(_generic_topic);
    }
    return pubSubClient->connected();
}

void MQTT::change_generic_topic(char *new_topic) {
    if (pubSubClient->connected()) {
        pubSubClient->unsubscribe(_generic_topic);
        pubSubClient->subscribe(new_topic);
    }
    _generic_topic = new_topic;
}

void MQTT::change_telemetry_topic(char *new_topic) {
    if (pubSubClient->connected()) {
        pubSubClient->unsubscribe(_telemetry_topic);
        pubSubClient->subscribe(new_topic);
    }
    _telemetry_topic = new_topic;
}

void MQTT::process() {
    if (pubSubClient == nullptr) {
        return;
    }
    if (!pubSubClient->connected()) {
        unsigned long now = millis();
        if (now - last_reconnect_attempt > 5000) {
            last_reconnect_attempt = now;
            if (reconnect()) {
                last_reconnect_attempt = 0;
            }
        }
    } else {
        pubSubClient->loop();
    }
}

void MQTT::callback(const char *topic, uint8_t *payload, unsigned int length) {
    char temp_str[LIMIT_SETTINGS_MAX_STRING_LENGTH];
    int gong_pos = -1;
    int volume_pos = -1;
    int zones_pos = -1;
    char *track_num_end = nullptr;
    char *volume_end = nullptr;
    uint16_t track_num;
    bool gong = _settings->get_vellez_gong_enabled();
    uint16_t zones = _settings->get_vellez_zones();
    uint8_t volume = _settings->get_volume();

    if (strcmp(topic, _telemetry_topic) != 0) {
        switch (payload[0]) {
            case MQTT_COMMAND_SET_ZONES:
            case MQTT_COMMAND_SET_ADDRESS:
            case MQTT_COMMAND_SET_VOLUME:
                return;
        }
    }

    switch (payload[0]) {
        case MQTT_COMMAND_DISABLE:
            if (toggle_mqtt_activation_callback != nullptr) {
                toggle_mqtt_activation_callback(false);
            }
            break;
        case MQTT_COMMAND_ENABLE:
            if (toggle_mqtt_activation_callback != nullptr) {
                toggle_mqtt_activation_callback(true);
            }
            break;
        case MQTT_COMMAND_REBOOT:
            if (reboot_callback != nullptr) {
                reboot_callback();
            }
            break;
        case MQTT_COMMAND_SET_ADDRESS:
            if (length == 1) {
                _settings->clear_vellez_address();
            } else if (length == 2) {
                if (payload[1] - MQTT_PARAM_NUMBER_BASE > 0 && payload[1] - MQTT_PARAM_NUMBER_BASE < 6) {
                    _settings->set_vellez_address(payload[1] - MQTT_PARAM_NUMBER_BASE - 1);
                }
            }
            if (address_callback != nullptr) {
                address_callback(_settings->get_vellez_address());
            }
            break;
        case MQTT_COMMAND_SET_GONG:
            if (length == 1) {
                _settings->clear_vellez_gong_enabled();
            } else if (length == 2) {
                if (payload[1] == MQTT_PARAM_TRUE || payload[1] == MQTT_PARAM_FALSE) {
                    _settings->set_vellez_gong_enabled(payload[1] == MQTT_PARAM_TRUE);
                }
            }
            if (gong_callback != nullptr) {
                gong_callback(_settings->get_vellez_gong_enabled());
            }
            break;
        case MQTT_COMMAND_SET_WEB_USERNAME:
            if (length == 1) {
                _settings->clear_web_username();
            } else if (length > 2 && length < LIMIT_SETTINGS_MAX_STRING_LENGTH - 1) {
                snprintf(temp_str, length, "%s", (char *) payload + 1);
                _settings->set_web_username(temp_str);
            }
            if (web_username_callback != nullptr) {
                web_username_callback(_settings->get_mqtt_username());
            }
            break;
        case MQTT_COMMAND_SET_WEB_PASSWORD:
            if (length == 1) {
                _settings->clear_web_password();
            } else if (length > 2 && length < LIMIT_SETTINGS_MAX_STRING_LENGTH - 1) {
                snprintf(temp_str, length, "%s", (char *) payload + 1);
                _settings->set_web_password(temp_str);
            }
            if (web_password_callback != nullptr) {
                web_password_callback(_settings->get_mqtt_password());
            }
            break;
        case MQTT_COMMAND_SET_GENERIC_TOPIC:
            if (length == 1) {
                _settings->clear_mqtt_generic_topic();
            } else if (length > 2 && length < LIMIT_SETTINGS_MAX_STRING_LENGTH - 1) {
                snprintf(temp_str, length, "%s", (char *) payload + 1);
                _settings->set_mqtt_generic_topic(temp_str);
            }
            if (generic_topic_callback != nullptr) {
                generic_topic_callback(_settings->get_mqtt_generic_topic());
            }
            _generic_topic = _settings->get_mqtt_generic_topic();
            break;
        case MQTT_COMMAND_SET_TELEMETRY_TOPIC:
            if (length == 1) {
                _settings->clear_mqtt_telemetry_topic();
            } else if (length > 2 && length < LIMIT_SETTINGS_MAX_STRING_LENGTH - 1) {
                snprintf(temp_str, length, "%s", (char *) payload + 1);
                _settings->set_mqtt_telemetry_topic(temp_str);
            }
            if (telemetry_topic_callback != nullptr) {
                telemetry_topic_callback(_settings->get_mqtt_telemetry_topic());
            }
            _telemetry_topic = _settings->get_mqtt_telemetry_topic();
            break;
        case MQTT_COMMAND_SET_VOLUME:
            if (length == 1) {
                _settings->clear_volume();
            } else if (length == 2 || length == 3) {
                snprintf(temp_str, length, "%s", (char *) payload + 1);
                _settings->set_volume(strtol(temp_str, nullptr, 10));
            }
            if (volume_callback != nullptr) {
                volume_callback(_settings->get_volume());
            }
            break;
        case MQTT_COMMAND_SET_ZONES:
            if (length == 1) {
                _settings->clear_vellez_zones();
            } else {
                snprintf(temp_str, length, "%s", (char *) payload + 1);
                if (length > 1 && length < 7) {
                    _settings->set_vellez_zones(strtol(temp_str, nullptr, 10));
                } else if (length == 17) {
                    reverse(temp_str, temp_str + 15);
                    _settings->set_vellez_zones(strtol(temp_str, nullptr, 2));
                }
            }
            if (zones_callback != nullptr) {
                zones_callback(_settings->get_vellez_zones());
            }
            break;
        case MQTT_COMMAND_START:
            if (length < 2) {
                break;
            }

            snprintf(temp_str, length, "%s", (char *) payload + 1);
            track_num = strtol(temp_str, (char **) &track_num_end, 10);

            if ((int) length > track_num_end - temp_str) {
                switch (track_num_end[0]) {
                    case 'G':
                        gong_pos = track_num_end - temp_str;
                        break;
                    case 'V':
                        volume_pos = track_num_end - temp_str;
                        break;
                    case 'Z':
                        zones_pos = track_num_end - temp_str;
                        break;
                }
            }

            if (gong_pos != -1) {
                gong = temp_str[gong_pos] == MQTT_PARAM_TRUE;
                if ((int) length > gong_pos + 2) {
                    switch (temp_str[gong_pos + 1]) {
                        case 'V':
                            volume_pos = gong_pos + 2;
                            break;
                        case 'Z':
                            zones_pos = gong_pos + 2;
                            break;
                    }
                }
            }

            if (volume_pos != -1) {
                volume = strtol(temp_str + volume_pos, (char **) &volume_end, 10);
                if ((int) length > volume_end - temp_str && volume_end[0] == 'Z') {
                    zones_pos = volume_end - temp_str;
                }
            }

            if (zones_pos != -1) {
                if ((int) length < (zones_pos + 6)) {
                    zones = strtol(temp_str + zones_pos, nullptr, 10);
                } else if ((int) length == zones_pos + 16) {
                    reverse(temp_str + zones_pos, temp_str + zones_pos + 15);
                    zones = strtol(temp_str + zones_pos, nullptr, 2);
                }
            }
            if (play_callback != nullptr) {
                play_callback(track_num, gong, volume, zones);
            }
            break;
        case MQTT_COMMAND_STOP:
            if (stop_callback != nullptr)
                stop_callback();
            break;
    }
}

void MQTT::send(const char *topic, const char *message) {
    if (pubSubClient == nullptr) {
        return;
    }
}

void MQTT::set_toggle_callback(void_bool_callback_t callback) {
    toggle_mqtt_activation_callback = std::move(callback);
}

void MQTT::set_address_callback(void_uint8_callback_t callback) {
    address_callback = std::move(callback);
}

void MQTT::set_gong_callback(void_bool_callback_t callback) {
    gong_callback = std::move(callback);
}

void MQTT::set_generic_topic_callback(void_str_callback_t callback) {
    generic_topic_callback = std::move(callback);
}

void MQTT::set_telemetry_topic_callback(void_str_callback_t callback) {
    generic_topic_callback = std::move(callback);
}

void MQTT::set_web_username_callback(void_str_callback_t callback) {
    web_username_callback = std::move(callback);
}

void MQTT::set_web_password_callback(void_str_callback_t callback) {
    web_password_callback = std::move(callback);
}

void MQTT::set_volume_callback(void_uint8_callback_t callback) {
    volume_callback = std::move(callback);
}

void MQTT::set_zones_callback(void_uint16_callback_t callback) {
    zones_callback = std::move(callback);
}

void MQTT::set_play_callback(MQTT::mqtt_play_callback_t callback) {
    play_callback = std::move(callback);
}

void MQTT::set_stop_callback(void_callback_t callback) {
    stop_callback = std::move(callback);
}

void MQTT::set_reboot_callback(void_callback_t callback) {
    reboot_callback = std::move(callback);
}
