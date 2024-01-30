#include "DriveControls.h"

namespace DriveControls {

Stepper leftMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, LEFT_MOTOR_PIN_0, LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, LEFT_MOTOR_PIN_3);
Stepper rightMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, RIGHT_MOTOR_PIN_0, RIGHT_MOTOR_PIN_1, RIGHT_MOTOR_PIN_2, RIGHT_MOTOR_PIN_3);

void initSteppers() {
  leftMotor.setSpeed(MOTOR_SPEED);
  rightMotor.setSpeed(MOTOR_SPEED);
  logger.printline("initialized steppers");
}

bool checkMotorActivity() {
  if (StepperRotator::areActive()) {
    logger.printline("rotation blocked because of ongoing rotation", "warning");
    return true;
  }
  return false;
}

void rotateLeftMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  int repeats = steps / MOTOR_STEPSIZE;
  LAS::scheduleRepeated(new StepperRotator(&leftMotor, MOTOR_STEPSIZE * (abs(steps) / steps)), repeats);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating left motor by %d", steps);
  logger.printline(buffer, "debug");
}

void rotateRightMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  int repeats = steps / MOTOR_STEPSIZE;
  LAS::scheduleRepeated(new StepperRotator(&rightMotor, MOTOR_STEPSIZE * (abs(steps) / steps)), repeats);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating right motor by %d", steps);
  logger.printline(buffer, "debug");
}

StepperRotator *scheduleConstantRightRotatorAsync() {
  StepperRotator *rotator = new StepperRotator(&rightMotor, MOTOR_STEPSIZE);
  LAS::scheduleRepeated(rotator);
  if (checkMotorActivity()) {
    rotator->pause();
  }
  logger.printline("start const driving right motor", "debug");
  return rotator;
}

StepperRotator *scheduleConstantLeftRotatorAsync() {
  StepperRotator *rotator = new StepperRotator(&rightMotor, MOTOR_STEPSIZE);
  LAS::scheduleRepeated(rotator);
  if (checkMotorActivity()) {
    rotator->pause();
  }
  logger.printline("start const driving left motor", "debug");
  return rotator;
}

void driveStepsForward(int steps) {
  rotateRightMotorAsync(steps);
  rotateRightMotorAsync(steps);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "driving %d steps", steps);
  logger.printline(buffer, "info");
}

void rotateVehicleByAsync(float pi_mul) {
  if (VehicleRotation::isRotationActive()) {
    return;
  }
  int steps = int(ROTATION_REVOLUTIONS * (pi_mul / 2) * MOTOR_STEPS_PER_REVOLUTION);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating vehicle by %d", steps);
  logger.printline(buffer, "info");
  bool l = true;
  if (steps < 0) {
    l = false;
  }
  LAS::scheduleRepeated(new VehicleRotation(l, &leftMotor, &rightMotor), ASAP, abs(int(steps / MOTOR_STEPSIZE) * 2));
}

void rotateVehicleToAsync(float pi_mul) {
  rotateVehicleByAsync(pi_mul - VehicleRotation::getCurrentVehicleRotation());
}

void driveSizeUnits(float units) {
  driveStepsForward(int(units * VEHICLE_STEPS_X));
}

}
