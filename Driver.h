#pragma once

#include "CommonIncludes.h"
#include "StepperRotator.h"
#include "Sensors.h"
#include "DriveControls.h"

class Driver : public LAS::Callable {
public:
  Driver();
  ~Driver();
  void run() override;
  int getStateId();
  int getAvoidStage();
  void init();
  bool isLeftMotorActive();
  bool isRightMotorActive();
private:
  enum NavState { DRIVE = 0, // mostly done
                  SETBACK = 7, // done
                  AVOID_ENTRY = 1, // needs testing
                  AVOID_EXIT = 2, // ...
                  COLLECT = 3, // not started
                  TRANSFER_ENTRY = 4, // not started
                  TRANSFER_EXIT = 6, // not started
                  FINISH = 5 }; // should work
  int avoidStage = 0;
  void setState(NavState state);
  void pauseDriving();
  void resumeDriving();
  int lastLDist;
  bool directionR;
  NavState state;
  NavState lastState;
  StepperRotator *rotatorLeft;
  StepperRotator *rotatorRight;
};
