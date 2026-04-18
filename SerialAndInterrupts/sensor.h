#ifndef SENSOR_H
#define SENSOR_H

typedef struct Sensor Sensor;
typedef void (*read_t)(Sensor*);
typedef void (*init_t)(Sensor*);
struct Sensor {
  const char* name;
  init_t init;
  read_t readSensor;
};

void sensor_init(Sensor* s);

void init_Temp(Sensor* s);
void init_Humid(Sensor* s);
void init_Accel(Sensor* s);
void init_Light(Sensor*);

void read_Idle(Sensor*);
void read_Temp(Sensor*);
void read_Humid(Sensor*);
void read_Accel(Sensor*);
void read_Light(Sensor*);
void read_NoSensor(Sensor* s);

#endif