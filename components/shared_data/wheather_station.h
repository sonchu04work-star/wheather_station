#ifndef WHEARTHER_STATION_H
#define WHEARTHER_STATION_H

#include <stdbool.h>

typedef struct {
    float temperature;
    float humidity;
    float latitude;
    float longitude;
    bool gps_valid;
} weather_data_t;

#endif // WHEARTHER_STATION_H