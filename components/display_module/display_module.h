#ifndef DISPLAY_MODULE_H
#define DISPLAY_MODULE_H

#include "wheather_station.h"

void display_module_init(void);
void display_module_update(weather_data_t *data);

#endif