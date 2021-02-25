#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct {
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
    float temp, pres;
    float alt;
}Imu;

typedef struct {
    float lat, lon;
    float speed;
    float course;
    float alt;
}Gps;

typedef struct {
    Imu imu;
    Gps gps;
}Telemetry;

#endif