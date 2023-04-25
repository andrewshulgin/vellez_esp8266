#ifndef VELLEZ_ESP8266_VELLEZ_H
#define VELLEZ_ESP8266_VELLEZ_H

#include <Arduino.h>
#include <HardwareSerial.h>

#include "callbacks.h"

#define VELLEZ_BAUD_RATE 115200

#define VELLEZ_POLL_SIZE 4

#define VELLEZ_POLL_HEADER 0x52
#define VELLEZ_RESP_HEADER 0x41

#define VELLEZ_MODE_IDLE 0x00
#define VELLEZ_MODE_ACTIVE 0x01
#define VELLEZ_MODE_GONG 0x03

#define VELLEZ_ALL_ZONES 0xFFFF

class Vellez {
public:
    void begin(Stream &stream, uint8_t tx_en_pin, uint8_t addr, bool gong, uint16_t zones);
    void set_callback(void_bool_callback_t callback);
    void set_address(uint8_t addr);
    void set_gong(bool gong);
    void set_zones(uint16_t zones);
    void activate();
    void deactivate();
    void process();
    void end();
private:
    Stream *serial;
    bool enable_processing;
    bool keep_active;
    bool pkt_complete;
    bool enable_gong;
    unsigned long last_pkt_at;
    uint8_t address;
    uint16_t enabled_zones;
    uint8_t tx_enable_pin;
    uint8_t in_pkt[VELLEZ_POLL_SIZE];
    uint8_t bytes_received;
    uint8_t checksum;
    void_bool_callback_t callback_function;
    bool verify_checksum();
    void write_header();
    void write(uint8_t byte);
    void write_trailer();
};

#endif // VELLEZ_ESP8266_VELLEZ_H
