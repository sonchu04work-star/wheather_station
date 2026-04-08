idf_component_register(SRCS "wheather_station.c"
                       INCLUDE_DIRS "."
                       REQUIRES display_module gps_module network_module sensor_module)