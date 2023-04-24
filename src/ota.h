#ifndef VELLEZ_ESP8266_OTA_H
#define VELLEZ_ESP8266_OTA_H

#include <Client.h>
#include <ESP8266httpUpdate.h>

class OTA {
public:
    static void check(const char *, uint16_t, const char *, const char *, WiFiClient *client);
};


#endif // VELLEZ_ESP8266_OTA_H
