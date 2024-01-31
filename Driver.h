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
  enum NavState { DRIVE = 0,
                  AVOID_ENTRY = 1,
                  AVOID_EXIT = 2,
                  COLLECT = 3,
                  TRANSFER = 4,
                  FINISH = 5 };
  int avoidStage = 0;
  void setState(NavState state);
  void pauseDriving();
  void resumeDriving();
  int lastLDist;
  bool directionR;
  NavState state;
  StepperRotator *rotatorLeft;
  StepperRotator *rotatorRight;
};
