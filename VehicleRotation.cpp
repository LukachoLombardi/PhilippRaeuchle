#include "VehicleRotation.h"
void VehicleRotation::run() {
  rotationActive = true;
  if (alternate) {
    leftMotor->step(stepSize * -1);
  } else {
    rightMotor->step(stepSize);
  }
  setRotationVar(currentVehicleRotation + (ROT_MUL_PER_STEPSIZE / 2));
  if (taskPtr->remainingRepeats == 1) {
    rotationActive = false;
  }
  alternate = !alternate;
}
VehicleRotation::VehicleRotation(bool directionL, Stepper* leftMotor, Stepper* rightMotor)
  : directionL(directionL), leftMotor(leftMotor), rightMotor(rightMotor) {
  if (directionL) {
    stepSize = MOTOR_STEPSIZE;
  } else {
    stepSize = MOTOR_STEPSIZE * -1;
  }
};
bool VehicleRotation::isRotationActive() {
  return rotationActive;
}
float VehicleRotation::getCurrentVehicleRotation() {
  return currentVehicleRotation;
}

void VehicleRotation::setRotationVar(float pi_mul) {
  currentVehicleRotation = pi_mul;
  while (currentVehicleRotation >= 2) {
    currentVehicleRotation -= 2;
  }
  while (currentVehicleRotation < 0) {
    currentVehicleRotation += 2;
  }
}

bool VehicleRotation::rotationActive = false;
float VehicleRotation::currentVehicleRotation = 0;
