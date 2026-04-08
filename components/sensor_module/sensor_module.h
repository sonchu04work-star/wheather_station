#ifndef SENSOR_MODULE_H
#define SENSOR_MODULE_H

void sensor_module_init(void);
void sensor_module_read(float *temperature, float *humidity);

#endif
