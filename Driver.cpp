#include "Driver.h"

void Driver::run() {
  if (state == SETBACK) {
    if (StepperRotator::areActive()) {
      return;
    } else {
      setState(lastState);
      if (state == DRIVE) {
        setState(TRANSFER_ENTRY);
      } else if (state == AVOID_ENTRY || state == AVOID_EXIT) {
        DriveControls::rotateVehicleByAsync(0.5);
        setState(DRIVE);
        directionR = !directionR;
      } else if (state == TRANSFER_EXIT) {
        Shared::logger.printline("Table Finished!");
        finish();
      }
      return;
    }
  }
  if (Sensors::tof_measure_down.RangeMilliMeter >= MAX_TABLE_DISTANCE) {
    Shared::logger.printline("end of table reached.");
    pauseDriving();
    setState(SETBACK);
    DriveControls::driveSizeUnits(-0.25);
    return;
  }
  if (VehicleRotation::isRotationActive()) {
    return;
  }
  // switch those fucking states
  switch (state) {
    case DRIVE:
      if ((Sensors::tof_measure_fw_low.RangeMilliMeter >= SAFETY_DISTANCE)) {
        resumeDriving();
      } else {
        setState(AVOID_ENTRY);
        pauseDriving();
        DriveControls::rotateVehicleByAsync(0.75);
      }
      break;
    case AVOID_ENTRY:
      if (Sensors::tof_measure_left.RangeMilliMeter >= SAFETY_DISTANCE) {
        resumeDriving();
      } else {
        pauseDriving();
        lastLDist = -1;
        if (avoidStage == 2) {
          DriveControls::rotateVehicleByAsync(-0.25);
          avoidStage = 0;
          setState(DRIVE);
        } else {
          setState(AVOID_EXIT);
        }
      }
      break;
    case AVOID_EXIT:
      if (lastLDist == -1 || (abs(Sensors::tof_measure_left.RangeMilliMeter - lastLDist) <= UNITY_DISTANCE)) {
        resumeDriving();
      } else {
        pauseDriving();
        DriveControls::rotateVehicleByAsync(0.25);
        avoidStage++;
        if (avoidStage <= 2) {
          setState(AVOID_ENTRY);
        } else {
          avoidStage = 0;
          if (directionR) {
            DriveControls::rotateVehicleToAsync(0);
          } else {
            DriveControls::rotateVehicleToAsync(0.5);
          }
          setState(DRIVE);
        }
      }
      lastLDist = Sensors::tof_measure_left.RangeMilliMeter;
      break;
    case TRANSFER_ENTRY:
      if(avoidStage <)
      DriveControls::rotateVehicleByAsync(0.25);
      avoidStage++;
      break;
  }
}
int Driver::getStateId() {
  return int(state);
}
int Driver::getAvoidStage() {
  return avoidStage;
}
void Driver::init() {
  delete rotatorLeft;
  delete rotatorRight;
  rotatorLeft = DriveControls::scheduleConstantLeftRotatorAsync();
  rotatorRight = DriveControls::scheduleConstantRightRotatorAsync();
  pauseDriving();
}
bool Driver::isLeftMotorActive() {
  return rotatorLeft->isActive();
}
bool Driver::isRightMotorActive() {
  return rotatorRight->isActive();
}

void Driver::setState(NavState state) {
  this->lastState = this->state;
  this->state = state;
  char buffer[BUFFER_SIZE];
  char stateChar[BUFFER_SIZE];
  switch (state) {
    case 0:
      strcpy(stateChar, "DRIVE");
      break;
    case 7:
      strcpy(stateChar, "SETBACK");
      break;
    case 1:
      strcpy(stateChar, "AVOID_ENTRY");
      break;
    case 2:
      strcpy(stateChar, "AVOID_EXIT");
      break;
    case 3:
      strcpy(stateChar, "COLLECT");
      break;
    case 4:
      strcpy(stateChar, "TRANSFER_ENTRY");
      break;
    case 6:
      strcpy(stateChar, "TRANSFER_EXIT");
      break;
    case 5:
      strcpy(stateChar, "FINISH");
      break;
  }
  strcpy(buffer, "switched state to: ");
  strcat(buffer, stateChar);
  Shared::logger.printline(buffer);
}
void Driver::pauseDriving() {
  rotatorLeft->pause();
  rotatorRight->pause();
}
void Driver::resumeDriving() {
  rotatorLeft->resume();
  rotatorRight->resume();
}

Driver::Driver() {
  lastLDist = 0;
  directionR = true;
  state = DRIVE;
  lastState = DRIVE;
  rotatorLeft = nullptr;
  rotatorRight = nullptr;
}
Driver::~Driver() {
  rotatorLeft->finish();
  rotatorRight->finish();
}
