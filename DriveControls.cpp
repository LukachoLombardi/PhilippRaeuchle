#include "DriveControls.h"

namespace DriveControls {

bool driving = false;

bool lastMotorState = false;
bool rotationActive = false;

float currentVehicleRotation = 0.0;

void initSteppers() {
  pinMode(MOTOR_FEEDBACK_PIN, INPUT);
  logger.printline("initialized steppers");
  Serial1.begin(STEPPER_BAUDRATE);
  sendMotorSignalsAsync(0,0);
  las.scheduleFunction(driveKeepalive);
  las.scheduleRepeated(driveKeepalive,10000);
  las.scheduleRepeated(rotationCheckKeepalive);
}

bool checkMotorActivity() {
  if (digitalRead(MOTOR_FEEDBACK_PIN) == HIGH) {
    logger.printline("rotation blocked because of ongoing rotation", "warning");
    return true;
  }
  return false;
}

bool checkMotorActivitySilent() {
  if (digitalRead(MOTOR_FEEDBACK_PIN) == HIGH) {
    return true;
  }
  return false;
}

void rotateMotorsAsync(int steps1, int steps2) {
  if (checkMotorActivitySilent()) {
    return;
  }
  sendMotorSignalsAsync(steps1, steps2);
}

void sendMotorSignalsAsync(int sig1, int sig2) {
  Serial1.print("/");
  Serial1.print(sig1);
  Serial1.print("/");
  Serial1.print(sig2);

  long startTime = millis();
  while(!checkMotorActivitySilent()) {
    /* if(millis() - startTime >= MAX_STEPPER_RESPONSE_MS) {
      logger.printline("Stepper core out of sync. Halting!", Logger::LogLevel::Severe);
      while(true){millis();}
    } */
    if(millis() - startTime >= STEPPER_RECHECK_TIME) {
      startTime = millis();
      Serial1.print("/");
      Serial1.print(sig1);
      Serial1.print("/");
      Serial1.print(sig2);
    }
  }
}

void rotateLeftMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  rotateMotorsAsync(steps, -1);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating left motor by %d", steps);
  logger.printline(buffer, "debug");
}

void rotateRightMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  rotateMotorsAsync(-1, steps);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating right motor by %d", steps);
  logger.printline(buffer, "debug");
}

void drive() {
  if(!driving) {
    logger.printline("start driving");
  }
  driving = true;
  driveKeepalive();
}
void stop() {
  if(driving) {
    logger.printline("stop driving");
  }
  driving = false;
  sendMotorSignalsAsync(0,0);

  long startTime = millis();
  while(checkMotorActivitySilent()) {
    if(millis() - startTime >= MAX_STEPPER_RESPONSE_MS) {
      logger.printline("Stepper core out of sync. Halting!", Logger::LogLevel::Severe);
      while(true){millis();}
    }
  }
}
void driveKeepalive() {
  if(driving) {
    rotateMotorsAsync(MOTOR_STEPS_PER_REVOLUTION*100, MOTOR_STEPS_PER_REVOLUTION*100);
  }
}

void rotationCheckKeepalive() {
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
  logger.printline(buffer);
    rotateMotorsAsync(-steps, steps);
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
