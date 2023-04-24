#include "ota.h"

void OTA::check(const char *host, uint16_t port, const char *url, const char *current_version, WiFiClient *client) {
    (new ESP8266HTTPUpdate())->update(*client, host, port, url, current_version);
}
