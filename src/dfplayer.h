#ifndef VELLEZ_ESP8266_DFPLAYER_H
#define VELLEZ_ESP8266_DFPLAYER_H

#include <Stream.h>

#define DFPLAYER_PKT_LENGTH 10

#define DFPLAYER_PKT_START 0x7E
#define DFPLAYER_VERSION 0xFF
#define DFPLAYER_LENGTH 0x06
#define DFPLAYER_PKT_END 0xEF

#define DFPLAYER_CMD_PLAY 0x03
#define DFPLAYER_CMD_RESET 0x0C
#define DFPLAYER_CMD_VOL 0x06
#define DFPLAYER_CMD_STOP 0x16
#define DFPLAYER_CMD_STATUS 0x42
#define DFPLAYER_CMD_FEEDBACK 0x0B

class DFPlayer {
public:
    typedef std::function<void(bool)> callback_t;
    void begin(Stream &stream);
    void set_callback(callback_t callback);
    void reset();
    void set_volume(uint8_t volume);
    void play(uint16_t track);
    void stop();
    void process();
    struct data {
        uint8_t cmd;
        bool ack;
        uint16_t cmd_param;
        uint16_t checksum;
    } pkt_out, pkt_in;
private:
    Stream *_serial;

    bool _initialized;
    bool _playing;
    uint8_t _volume;
    uint16_t _track;
    uint16_t _checksum;

    uint32_t last_request;
    uint32_t last_response;

    bool need_reset;
    bool need_status;
    bool need_set_volume;
    bool need_play;
    bool need_stop;

    callback_t callback_function;

    enum pkt_parse_sequence {
        start,
        version,
        length,
        cmd,
        ack,
        cmd_param_msb,
        cmd_param_lsb,
        checksum_msb,
        checksum_lsb,
        end,
        complete
    };
    pkt_parse_sequence pkt_parse_state = start;

    void request(uint8_t cmd, uint16_t param = 0);
};


#endif // VELLEZ_ESP8266_DFPLAYER_H
