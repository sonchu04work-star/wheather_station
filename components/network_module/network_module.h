#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H
#include "wheather_station.h"

void network_module_init(const char* ssid, const char* pass);
void network_module_send_data(const char* server_url, weather_data_t *data);

#endif