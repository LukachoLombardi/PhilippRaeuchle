#pragma once

#include <Adafruit_TCS34725.h>
#include <Adafruit_VL53L0X.h>

#include "CommonIncludes.h"


namespace Sensors {
using namespace Shared;
extern Adafruit_TCS34725 tcs;
extern Adafruit_VL53L0X tof_fw_low;
extern Adafruit_VL53L0X tof_fw_high;
extern Adafruit_VL53L0X tof_left;
extern Adafruit_VL53L0X tof_right;
extern Adafruit_VL53L0X tof_down;

extern VL53L0X_RangingMeasurementData_t tof_measure_fw_low;
extern VL53L0X_RangingMeasurementData_t tof_measure_fw_high;
extern VL53L0X_RangingMeasurementData_t tof_measure_left;
extern VL53L0X_RangingMeasurementData_t tof_measure_right;
extern VL53L0X_RangingMeasurementData_t tof_measure_down;

void TOFAddrInit(Adafruit_VL53L0X &tof, int xshutPin, int addr);

void readTOFMMs();

void initTOFSensorsAsync();

void readColor(float *r, float *g, float *b);

class : public Callable {
public:
  void run() override {
    readColor(&red, &green, &blue);
  }
private:
  float red, green, blue;
} colorReader;

void initColorSensorAsync();
}
