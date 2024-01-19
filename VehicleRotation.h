#pragma once
#include <Arduino.h>
#include <Stepper.h>
#include <LAS.h>
#include "PhilippSettings.h"

class VehicleRotation : public LAS::Callable {
public:
  void run() override;
  VehicleRotation(bool directionL, Stepper* leftMotor, Stepper* rightMotor);
  static bool isRotationActive();
  static float getCurrentVehicleRotation();
private:
  void setRotationVar(float pi_mul);
  Stepper *leftMotor;
  Stepper *rightMotor;
  bool alternate = false;
  bool directionL = true;
  int stepSize;
  static float currentVehicleRotation;
  static bool rotationActive;
};
