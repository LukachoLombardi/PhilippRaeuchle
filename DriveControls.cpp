#include "DriveControls.h"

namespace DriveControls {

bool driving = false;

bool lastMotorState = false;
bool rotationActive = false;

float currentVehicleRotation = 0.0;

void initSteppers() {
  pinMode(MOTOR_FEEDBACK_PIN, INPUT_PULLUP);
  logger.printline("initialized steppers");
  LAS::scheduleRepeated(driveKeepalive,500);
}

bool checkMotorActivity() {
  if (digitalRead(MOTOR_FEEDBACK_PIN) == LOW) {
    logger.printline("rotation blocked because of ongoing rotation", "warning");
    return true;
  }
  return false;
}

bool checkMotorActivitySilent() {
  if (digitalRead(MOTOR_FEEDBACK_PIN) == LOW) {
    return true;
  }
  return false;
}

void rotateMotorsAsync(int steps1, int steps2) {
    if (checkMotorActivitySilent()) {
    return;
  }
  Serial.write("/");
  Serial.write(steps1);
  Serial.write("/");
  Serial.write(steps2);
}

void rotateLeftMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  rotateMotorsAsync(steps, 0);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating left motor by %d", steps);
  logger.printline(buffer, "debug");
}

void rotateRightMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  rotateMotorsAsync(0, steps);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating right motor by %d", steps);
  logger.printline(buffer, "debug");
}

void drive() {
  driving = true;
}
void stop() {
  driving = false;
  rotateMotorsAsync(0,0);
}
void driveKeepalive() {
  if(driving) {
    rotateMotorsAsync(MOTOR_STEPS_PER_REVOLUTION, MOTOR_STEPS_PER_REVOLUTION);
  }
  if(lastMotorState != checkMotorActivitySilent() && lastMotorState && rotationActive){
    rotationActive = false;
  }
  lastMotorState = checkMotorActivitySilent();
}

void driveStepsForward(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  rotateMotorsAsync(steps,steps);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "driving %d steps", steps);
  logger.printline(buffer, "info");
}

bool isRotationActive() {
  return rotationActive;
}

void setRotationVar(float pi_mul) {
  currentVehicleRotation = pi_mul;
  while (currentVehicleRotation >= 2) {
    currentVehicleRotation -= 2;
  }
  while (currentVehicleRotation < 0) {
    currentVehicleRotation += 2;
  }
}

void rotateVehicleByAsync(float rot_mul) {
  if (isRotationActive()) {
    Shared::logger.printline("vehicle rot blocked because of existing rotation", "warning");
    return;
  }
  int steps = int(ROTATION_REVOLUTIONS * (rot_mul)*MOTOR_STEPS_PER_REVOLUTION);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating vehicle by %d", steps);
  if (steps < 0) {
    rotateMotorsAsync(steps, -steps);
  } else {
    rotateMotorsAsync(-steps, steps);
  }
  rotationActive = true;
  setRotationVar(currentVehicleRotation + rot_mul);
}

void rotateVehicleToAsync(float rot_mul) {
  rotateVehicleByAsync(rot_mul - currentVehicleRotation);
}

void driveSizeUnits(float units) {
  driveStepsForward(units * VEHICLE_STEPS_X);
}

}
