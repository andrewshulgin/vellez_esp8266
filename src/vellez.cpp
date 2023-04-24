#include "vellez.h"

void Vellez::begin(Stream &stream, uint8_t tx_en_pin, uint8_t vellez_address, bool gong, uint16_t zones) {
    serial = &stream;
    address = vellez_address;
    tx_enable_pin = tx_en_pin;
    bytes_received = 0;
    pkt_complete = false;
    enable_processing = true;
    enable_gong = gong;
    enabled_zones = zones;
}

void Vellez::set_callback(Vellez::callback_t callback) {
    callback_function = std::move(callback);
}

void Vellez::set_address(uint8_t addr) {
    address = addr;
}

void Vellez::set_gong(bool gong) {
    enable_gong = gong;
}

void Vellez::set_zones(uint8_t zones) {
    enabled_zones = zones;
}

void Vellez::process() {
    if (!enable_processing) {
        return;
    }
    if (serial->available()) {
        uint8_t vellez_in = serial->read();
        if (vellez_in >= 0) {
            // Reset on overflow
            if (bytes_received >= VELLEZ_POLL_SIZE) {
                bytes_received = 0;
                pkt_complete = false;
            }
            if (vellez_in == VELLEZ_POLL_HEADER) {
                in_pkt[0] = vellez_in;
                bytes_received = 1;
                pkt_complete = false;
            } else {
                in_pkt[bytes_received] = vellez_in;
                bytes_received++;
            }
            if (bytes_received == VELLEZ_POLL_SIZE) {
                pkt_complete = true;
            }
        }
    }
    if (pkt_complete) {
        if (in_pkt[0] == VELLEZ_POLL_HEADER && in_pkt[1] == address && verify_checksum()) {
            delayMicroseconds(160);
            digitalWrite(tx_enable_pin, HIGH);
            write_header();
            uint8 mode = VELLEZ_MODE_IDLE;
            if (keep_active) {
                mode = enable_gong ? VELLEZ_MODE_GONG : VELLEZ_MODE_ACTIVE;
            }
            if (enabled_zones < VELLEZ_ALL_ZONES) {
                write(0x10 + address);
                write(mode);
                write(enabled_zones & 0xFF);
                write((enabled_zones >> 8) & 0xFF);
            } else {
                write(address);
                write(0x04 + mode);
            }
            write_trailer();
            serial->flush();
            digitalWrite(tx_enable_pin, LOW);
            callback_function(in_pkt[2] == true);
            pkt_complete = false;
        }
    }
}

void Vellez::activate() {
    keep_active = true;
}

void Vellez::deactivate() {
    keep_active = false;
}

bool Vellez::verify_checksum() {
    return in_pkt[3] == (in_pkt[1] ^ in_pkt[2]);
}

void Vellez::write_header() {
    serial->write(VELLEZ_RESP_HEADER);
}

void Vellez::write(uint8_t byte) {
    checksum = checksum ^ byte;
    serial->write(byte);
}

void Vellez::write_trailer() {
    serial->write(checksum);
    checksum = 0;
}

void Vellez::end() {
    digitalWrite(tx_enable_pin, LOW);
    enable_processing = false;
}
