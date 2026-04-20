/*
  Morgan Williams
  CSCE 3612.001
  Lab 4: Wio Serial and Interrupts - sensor.h
  Apr 22, 2026
*/
#ifndef SENSOR_H
#define SENSOR_H

typedef struct Sensor Sensor;
typedef void (*read_t)(Sensor*);
typedef void (*init_t)(Sensor*);
struct Sensor {
  const char* name;
  init_t init;                    // Sensor-specific initialization
  read_t readSensor;              // Sensor-specific sensor-read
};

void sensor_init(Sensor* s);

void init_Temp(Sensor* s);
void init_Humid(Sensor* s);
void init_Accel(Sensor* s);
void init_Light(Sensor*);

void read_Temp(Sensor*);
void read_Humid(Sensor*);
void read_Accel(Sensor*);
void read_Light(Sensor*);
void read_NoSensor(Sensor* s);    // Fallback if any sensor is unavailable

#endif