#include <Arduino.h>
#include "sensor.h"
#include "LIS3DHTR.h" // Accelerometer
#include "AHT20.h"    // Temp/humidity sensor
#include <Wire.h>
#include <stdbool.h>

void sensor_init(Sensor* s) { s->init(s); }

/******************************************************************************
 *                                    IDLE                                    * 
 ******************************************************************************/
void read_Idle(Sensor*) {
  Serial.printf("Waiting for input\n");
}

/******************************************************************************
 *                            TEMPERATURE/HUMIDITY                            * 
 ******************************************************************************/
AHT20 aht;

static void init_TempHumid(Sensor* s) {
  static bool began = false;
  if (!began) { aht.begin(); began = true; }

  // Continue without input if initial read fails
  float _h, _t;
  if (!aht.getSensor(&_h, &_t)) s->readSensor = read_NoSensor;
}

void init_Temp(Sensor* s) { init_TempHumid(s); }
void init_Humid(Sensor* s) { init_TempHumid(s); }

void read_Temp(Sensor*) {
  float temp, _h;
  if (!aht.getSensor(&_h, &temp))
    Serial.printf("AHT20 (temperature) read failed\n");
  else
    Serial.printf("Temperature: %.2fC\n", temp);
}

void read_Humid(Sensor*) {
  float _t, humid;
  if (!aht.getSensor(&humid, &_t))
    Serial.printf("AHT20 (humidity) read failed\n");
  else
    Serial.printf("Humidity: %.2f%%\n", humid);
}

/******************************************************************************
 *                               ACCELEROMETER                                * 
 ******************************************************************************/
LIS3DHTR<TwoWire> lis;

void init_Accel(Sensor* s) {
  lis.begin(Wire1);

  // Continue without input if accelerometer not found
  if (!lis) s->readSensor = read_NoSensor;
  else
  {
    // Configure output rate and acceleration range
    lis.setOutputDataRate(LIS3DHTR_DATARATE_25HZ);
    lis.setFullScaleRange(LIS3DHTR_RANGE_2G);
  }
}

void read_Accel(Sensor*) {
  float x, y, z;
  x = lis.getAccelerationX();
  y = lis.getAccelerationY();
  z = lis.getAccelerationZ();

  Serial.printf(
    "Accelerometer values X Angle: %.2f, Y Angle: %.2f, Z Angle: %.2f\n", x,y,z
  );
}

/******************************************************************************
 *                                LIGHT SENSOR                                * 
 ******************************************************************************/
#define LIGHT_SENSOR WIO_LIGHT

void init_Light(Sensor*) {
  pinMode(LIGHT_SENSOR, INPUT);
}

void read_Light(Sensor*) {
  int light = analogRead(LIGHT_SENSOR);
  Serial.printf("Light value: %4d\n", light);
}

/******************************************************************************
 *                              NO SENSOR FOUND                               * 
 ******************************************************************************/
void read_NoSensor(Sensor* s) {
  Serial.printf("%s not found\n", s->name);
}