#include "Sensors.h"

namespace Sensors {
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_VL53L0X tof_fw_low = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_fw_high = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_left = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_right = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_down = Adafruit_VL53L0X();

VL53L0X_RangingMeasurementData_t tof_measure_fw_low;
VL53L0X_RangingMeasurementData_t tof_measure_fw_high;
VL53L0X_RangingMeasurementData_t tof_measure_left;
VL53L0X_RangingMeasurementData_t tof_measure_right;
VL53L0X_RangingMeasurementData_t tof_measure_down;

void TOFAddrInit(Adafruit_VL53L0X &tof, int xshutPin, int addr) {
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "Initalizing tof %x on pin %d", addr, xshutPin);
  logger.printline(buffer);
  digitalWrite(xshutPin, HIGH);
  tof.begin(addr, true);
}

void readTOFMMs() {
  tof_fw_low.rangingTest(&tof_measure_fw_low, false);
  //tof_fw_high.rangingTest(&tof_measure_fw_high, false);
  tof_left.rangingTest(&tof_measure_left, false);
  //tof_right.rangingTest(&tof_measure_right, false);
  tof_down.rangingTest(&tof_measure_down, false);
}

void initTOFSensorsAsync() {
  logger.printline("initalizing tofs...");
  pinMode(TOF_XSHUT_FW_LOW, OUTPUT);
  pinMode(TOF_XSHUT_FW_HIGH, OUTPUT);
  pinMode(TOF_XSHUT_LEFT, OUTPUT);
  pinMode(TOF_XSHUT_RIGHT, OUTPUT);
  pinMode(TOF_XSHUT_DOWN, OUTPUT);

  digitalWrite(TOF_XSHUT_FW_HIGH, LOW);
  digitalWrite(TOF_XSHUT_FW_LOW, LOW);
  digitalWrite(TOF_XSHUT_LEFT, LOW);
  digitalWrite(TOF_XSHUT_RIGHT, LOW);
  digitalWrite(TOF_XSHUT_DOWN, LOW);

  delay(10);
  logger.printline("starting tof init stage...");

  TOFAddrInit(tof_fw_low, TOF_XSHUT_FW_LOW, 0x2A);
  //TOFAddrInit(tof_fw_high, TOF_XSHUT_FW_HIGH, 0x2B);
  TOFAddrInit(tof_left, TOF_XSHUT_LEFT, 0x2C);
  //TOFAddrInit(tof_right, TOF_XSHUT_RIGHT, 0x2D);
  TOFAddrInit(tof_down, TOF_XSHUT_DOWN, 0x2E);
}

void readColor(float *r, float *g, float *b) {
  tcs.getRGB(r, g, b);
}

void initColorSensorAsync() {
  if (!tcs.begin()) {
    logger.printline("No TCS34725 found!", "severe");
    return;
  }
  las.scheduleRepeated(&colorReader, 50, ENDLESS_LOOP, false);
  logger.printline("initialized TCS");
}
}
