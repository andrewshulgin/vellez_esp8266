#ifndef VELLEZ_ESP8266_CALLBACKS_H
#define VELLEZ_ESP8266_CALLBACKS_H

typedef std::function<void(void)> void_callback_t;
typedef std::function<void(bool)> void_bool_callback_t;
typedef std::function<void(uint8_t)> void_uint8_callback_t;
typedef std::function<void(const char *)> void_str_callback_t;
typedef std::function<void(uint16_t)> void_uint16_callback_t;

#endif // VELLEZ_ESP8266_CALLBACKS_H
