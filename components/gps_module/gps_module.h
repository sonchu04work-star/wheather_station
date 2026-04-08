#ifndef GPS_MODULE_H
#define GPS_MODULE_H
#include <stdbool.h>

void gps_module_init(void);
void gps_module_get_latest(float *lat, float *lon, bool *is_valid);

#endif