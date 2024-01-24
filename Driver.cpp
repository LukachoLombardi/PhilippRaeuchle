#include "Driver.h"

void Driver::run() {
  if (VehicleRotation::isRotationActive()) {
    return;
  }
  Sensors::readTOFMMs();
  if (Sensors::tof_measure_down.RangeMilliMeter >= MAX_TABLE_DISTANCE && Sensors::tof_measure_down.RangeStatus != 4) {
    Shared::logger.printline("end of table reached.");
    pauseDriving();
    if (state == DRIVE) {
      setState(TRANSFER);
    } else if (state == AVOID_ENTRY || state == AVOID_EXIT) {
      DriveControls::rotateVehicleByAsync(1);
      setState(DRIVE);
      directionR = !directionR;
    } else if (state == TRANSFER) {
      Shared::logger.printline("Table Finished!");
      taskPtr->isActive = false;
    }
  }
  if (state == DRIVE) {
    if ((Sensors::tof_measure_fw_low.RangeStatus != 4 && Sensors::tof_measure_fw_low.RangeMilliMeter >= SAFETY_DISTANCE)) {
      resumeDriving();
    } else {
      setState(AVOID_ENTRY);
      pauseDriving();
      DriveControls::rotateVehicleByAsync(3 / 2);
    }
  } else if (state == AVOID_ENTRY) {
    if (Sensors::tof_measure_left.RangeStatus != 4 && Sensors::tof_measure_left.RangeMilliMeter >= UNITY_DISTANCE) {
      resumeDriving();
    } else {
      pauseDriving();
      lastLDist = -1;
      if (avoidStage == 2) {
        DriveControls::rotateVehicleByAsync(-1 / 4);
        avoidStage = 0;
        setState(DRIVE);
      } else {
        setState(AVOID_EXIT);
      }
    }
  } else if (state == AVOID_EXIT) {
    if (lastLDist == -1 || (Sensors::tof_measure_left.RangeStatus != 4 && abs(Sensors::tof_measure_left.RangeMilliMeter - lastLDist) <= UNITY_DISTANCE)) {
      resumeDriving();
    } else {
      pauseDriving();
      DriveControls::rotateVehicleByAsync(1 / 4);
      avoidStage++;
      if (avoidStage < 3) {
        setState(AVOID_ENTRY);
      } else {
        avoidStage = 0;
        if (directionR) {
          DriveControls::rotateVehicleToAsync(0);
        } else {
          DriveControls::rotateVehicleToAsync(1);
        }
        setState(DRIVE);
      }
      return;
    }
    lastLDist = 1000;  // might be fucky, test!
    if (Sensors::tof_measure_left.RangeStatus != 4) {
      lastLDist = Sensors::tof_measure_left.RangeMilliMeter;
    }
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
}
bool Driver::isLeftMotorActive() {
  return rotatorLeft->isActive();
}
bool Driver::isRightMotorActive() {
  return rotatorRight->isActive();
}

void Driver::setState(NavState state) {
  this->state = state;
  char buffer[BUFFER_SIZE];
  const char stateChar = char(int(state) + 48);
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
  rotatorLeft = nullptr;
  rotatorRight = nullptr;
}
