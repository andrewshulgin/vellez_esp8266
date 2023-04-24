#include "dfplayer.h"

void DFPlayer::begin(Stream &stream) {
    this->_serial = &stream;
    this->_initialized = false;
    this->_track = 0;
    this->_volume = 30;
    this->_playing = false;
    this->last_request = 0;
    this->last_response = 0;
    this->need_reset = true;
    this->need_status = true;
    this->need_set_volume = false;
    this->need_play = false;
    this->need_stop = false;
}

void DFPlayer::set_callback(DFPlayer::callback_t callback) {
    this->callback_function = std::move(callback);
}

void DFPlayer::reset() {
    this->need_reset = true;
}

void DFPlayer::set_volume(uint8_t volume) {
    this->_volume = volume <= 30 ? volume : 30;
    this->need_set_volume = true;
}

void DFPlayer::play(uint16_t track) {
    this->_track = track;
    this->need_play = true;
}

void DFPlayer::stop() {
    this->need_play = false;
    this->need_stop = true;
}

uint16_t calc_checksum(DFPlayer::data pkt) {
    return ~(DFPLAYER_VERSION + DFPLAYER_LENGTH + pkt.cmd + pkt.ack + pkt.cmd_param) + 1;
}

void DFPlayer::request(uint8_t cmd, uint16_t param) {
    this->last_request = millis();

    pkt_out.cmd = cmd;
    pkt_out.ack = false;
    pkt_out.cmd_param = param;
    pkt_out.checksum = calc_checksum(pkt_out);

    _serial->write(DFPLAYER_PKT_START);
    _serial->write(DFPLAYER_VERSION);
    _serial->write(DFPLAYER_PKT_LENGTH);
    _serial->write(pkt_out.cmd);
    _serial->write(pkt_out.ack);
    _serial->write((pkt_out.cmd_param >> 8) & 0xFF);
    _serial->write(pkt_out.cmd_param & 0xFF);
    _serial->write((pkt_out.checksum >> 8) & 0xFF);
    _serial->write(pkt_out.checksum & 0xFF);
    _serial->write(DFPLAYER_PKT_END);
}

void DFPlayer::process() {
    if (millis() - last_request > 100 && millis() - last_response > 50) {
        if (this->need_reset) {
            this->request(DFPLAYER_CMD_RESET);
            this->need_reset = false;
        } else if (this->_initialized && this->need_set_volume) {
            this->request(DFPLAYER_CMD_VOL, this->_volume);
            this->need_set_volume = false;
        } else if (this->_initialized && this->need_stop) {
            this->request(DFPLAYER_CMD_STOP);
            this->need_stop = false;
        } else if (this->_initialized && this->need_play) {
            this->request(DFPLAYER_CMD_PLAY, this->_track);
            this->need_play = false;
        } else if (this->need_status) {
            this->request(DFPLAYER_CMD_STATUS);
            this->need_status = false;
        }
    }
    if (_serial->available()) {
        uint8_t dfplayer_in = _serial->read();
        switch (pkt_parse_state) {
            case start:
                pkt_parse_state = dfplayer_in == DFPLAYER_PKT_START ? version : start;
                break;
            case version:
                pkt_parse_state = dfplayer_in == DFPLAYER_VERSION ? length : start;
                break;
            case length:
                pkt_parse_state = dfplayer_in == DFPLAYER_PKT_LENGTH ? cmd : start;
                break;
            case cmd:
                pkt_in.cmd = dfplayer_in;
                pkt_parse_state = ack;
                break;
            case ack:
                pkt_in.ack = dfplayer_in;
                pkt_parse_state = cmd_param_msb;
                break;
            case cmd_param_msb:
                pkt_in.cmd_param = dfplayer_in << 8;
                pkt_parse_state = cmd_param_lsb;
                break;
            case cmd_param_lsb:
                pkt_in.cmd_param = pkt_in.cmd_param | dfplayer_in;
                pkt_parse_state = checksum_msb;
                break;
            case checksum_msb:
                pkt_in.checksum = dfplayer_in << 8;
                pkt_parse_state = checksum_lsb;
                break;
            case checksum_lsb:
                pkt_in.checksum = pkt_in.checksum | dfplayer_in;
                pkt_parse_state = (_checksum == calc_checksum(pkt_in)) ? end : start;
                break;
            case end:
                pkt_parse_state = dfplayer_in == DFPLAYER_PKT_END ? complete : start;
                break;
            case complete:
                break;
        }
        if (pkt_parse_state == complete && pkt_in.cmd == DFPLAYER_CMD_FEEDBACK) {
            this->_playing = pkt_in.cmd_param == 1;
            if (this->_playing) {
                need_status = true;
            }
            callback_function(this->_playing);
        }
    }
}
