#include "mqtt.h"

MQTT::MQTT() {
    this->_client_id = nullptr;
    this->_generic_topic = nullptr;
    this->_telemetry_topic = nullptr;
    this->_username = nullptr;
    this->_password = nullptr;
    this->pubSubClient = nullptr;
    this->last_reconnect_attempt = 0;
}

void MQTT::begin(
        Client *client,
        const char *host,
        uint16_t port,
        const char *client_id,
        const char *generic_topic,
        const char *telemetry_topic,
        const char *username,
        const char *password
) {
    this->_client_id = client_id;
    this->_generic_topic = generic_topic;
    this->_telemetry_topic = telemetry_topic;
    this->_username = username;
    this->_password = password;
    this->pubSubClient = new PubSubClient();
    this->pubSubClient->setServer(host, port);
    this->pubSubClient->setClient(*client);
    this->pubSubClient->setCallback(
            {[this](char *topic, uint8_t *payload, unsigned int length) {
                callback(topic, payload, length);
            }}
    );
    reconnect();
}

void MQTT::end() {
    if (this->pubSubClient != nullptr) {
        this->pubSubClient->disconnect();
    }
}

bool MQTT::connected() {
    return this->pubSubClient != nullptr && this->pubSubClient->connected();
}

bool MQTT::reconnect() {
    if (this->pubSubClient == nullptr) {
        return false;
    }
    if (this->pubSubClient->connect(this->_client_id, this->_username, this->_password)) {
        this->pubSubClient->publish(this->_telemetry_topic, "connected");
        this->pubSubClient->subscribe(_telemetry_topic);
        this->pubSubClient->subscribe(_generic_topic);
    }
    return this->pubSubClient->connected();
}

void MQTT::change_generic_topic(char *new_topic) {
    if (this->pubSubClient->connected()) {
        this->pubSubClient->unsubscribe(_generic_topic);
        this->pubSubClient->subscribe(new_topic);
    }
    this->_generic_topic = new_topic;
}

void MQTT::change_telemetry_topic(char *new_topic) {
    if (this->pubSubClient->connected()) {
        this->pubSubClient->unsubscribe(_telemetry_topic);
        this->pubSubClient->subscribe(new_topic);
    }
    this->_telemetry_topic = new_topic;
}

void MQTT::process() {
    if (this->pubSubClient == nullptr) {
        return;
    }
    if (!this->pubSubClient->connected()) {
        unsigned long now = millis();
        if (now - last_reconnect_attempt > 5000) {
            last_reconnect_attempt = now;
            if (reconnect()) {
                last_reconnect_attempt = 0;
            }
        }
    } else {
        this->pubSubClient->loop();
    }
}

void MQTT::callback(const char *topic, uint8_t *payload, unsigned int length) {
    int gong_pos;
    int zones_pos;
    bool override_gong = false;
    bool override_zones = false;
    bool gong_enabled = false;
    uint16_t enabled_zones = 0xFFFF;

    if (topic != this->_telemetry_topic) {
        switch (payload[0]) {
            case MQTT_COMMAND_SET_ZONES:
            case MQTT_COMMAND_SET_ADDRESS:
            case MQTT_COMMAND_SET_VOLUME:
                return;
        }
    }

    switch (payload[0]) {
        case MQTT_COMMAND_DISABLE:
            if (this->toggle_callback != nullptr)
                this->toggle_callback(false);
            break;
        case MQTT_COMMAND_ENABLE:
            if (this->toggle_callback != nullptr)
                this->toggle_callback(true);
            break;
        case MQTT_COMMAND_REBOOT:
            if (this->reboot_callback != nullptr)
                this->reboot_callback();
            break;
        case MQTT_COMMAND_SET_ADDRESS:
            if (length == 2 && this->address_callback != nullptr) {
                this->address_callback(payload[1] - MQTT_COMMAND_NUMBER_BASE - 1);
            }
            break;
        case MQTT_COMMAND_SET_GONG:
            if (length == 2 && (payload[1] == MQTT_COMMAND_FALSE || payload[1] == MQTT_COMMAND_TRUE)) {
                if (this->gong_callback != nullptr)
                    this->gong_callback(payload[1] == MQTT_COMMAND_TRUE);
            }
            break;
        case MQTT_COMMAND_SET_WEB_USERNAME:
            if (length > 2 && this->web_username_callback != nullptr) {
                this->web_username_callback((char *) payload + 1, length);
            }
            break;
        case MQTT_COMMAND_SET_WEB_PASSWORD:
            if (length > 2 && this->web_password_callback != nullptr) {
                this->web_password_callback((char *) payload + 1, length);
            }
            break;
        case MQTT_COMMAND_SET_GENERIC_TOPIC:
            if (length > 2 && this->generic_topic_callback != nullptr) {
                this->generic_topic_callback((char *) payload + 1, length);
            }
            break;
        case MQTT_COMMAND_SET_TELEMETRY_TOPIC:
            if (length > 2 && this->telemetry_topic_callback != nullptr) {
                this->telemetry_topic_callback((char *) payload + 1, length);
            }
            break;
        case MQTT_COMMAND_SET_VOLUME:
            if (length == 2 || length == 3) {
                if (this->volume_callback != nullptr)
                    this->volume_callback(strtol((char *) payload + 1, nullptr, 10));
            }
            break;
        case MQTT_COMMAND_SET_ZONES:
            if (length == 17 && this->zones_callback != nullptr) {
                this->zones_callback(strtol((char *) payload + 1, nullptr, 2));
            }
            break;
        case MQTT_COMMAND_START:
            if (length < 2 || payload[1] < MQTT_COMMAND_NUMBER_BASE || payload[1] > MQTT_COMMAND_MSG_MAX) {
                break;
            }
            if (this->start_callback == nullptr) {
                break;
            }
            gong_pos = (length == 3 || length == 19) ? 2 : -1;
            zones_pos = length >= 18 ? (gong_pos < 0 ? 2 : 3) : -1;
            if (gong_pos != -1 && (payload[gong_pos] == MQTT_COMMAND_FALSE || payload[gong_pos] == MQTT_COMMAND_TRUE)) {
                override_gong = true;
                gong_enabled = payload[gong_pos] == MQTT_COMMAND_TRUE;
            }
            if (zones_pos != -1) {
                override_zones = true;
                enabled_zones = strtol((char *) payload + zones_pos, nullptr, 2);
            }
            this->start_callback(
                    payload[1] - MQTT_COMMAND_NUMBER_BASE,
                    override_gong,
                    override_zones,
                    gong_enabled,
                    enabled_zones
            );
            break;
        case MQTT_COMMAND_STOP:
            if (this->stop_callback != nullptr)
                this->stop_callback();
            break;
    }
}

void MQTT::send(const char *topic, const char *message) {
    if (this->pubSubClient == nullptr) {
        return;
    }
}

void MQTT::set_toggle_callback(MQTT::bool_callback_t callback) {
    toggle_callback = std::move(callback);
}

void MQTT::set_address_callback(MQTT::uint8_callback_t callback) {
    address_callback = std::move(callback);
}

void MQTT::set_gong_callback(MQTT::bool_callback_t callback) {
    gong_callback = std::move(callback);
}

void MQTT::set_generic_topic_callback(MQTT::str_callback_t callback) {
    generic_topic_callback = std::move(callback);
}

void MQTT::set_telemetry_topic_callback(MQTT::str_callback_t callback) {
    generic_topic_callback = std::move(callback);
}

void MQTT::set_web_username_callback(MQTT::str_callback_t callback) {
    web_username_callback = std::move(callback);
}

void MQTT::set_web_password_callback(MQTT::str_callback_t callback) {
    web_password_callback = std::move(callback);
}

void MQTT::set_volume_callback(MQTT::uint8_callback_t callback) {
    volume_callback = std::move(callback);
}

void MQTT::set_zones_callback(MQTT::uint16_callback_t callback) {
    zones_callback = std::move(callback);
}

void MQTT::set_start_callback(MQTT::start_callback_t callback) {
    start_callback = std::move(callback);
}

void MQTT::set_stop_callback(MQTT::callback_t callback) {
    stop_callback = std::move(callback);
}

void MQTT::set_reboot_callback(MQTT::callback_t callback) {
    reboot_callback = std::move(callback);
}
