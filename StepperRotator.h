#pragma once
#include "CommonIncludes.h"
#include "Stepper.h"

class StepperRotator : public LAS::Callable {
public:
  void run() override;
  void pause();
  void resume();
  bool isActive();
  static bool areActive();
  static void unblock();
  StepperRotator(Stepper *stepper, int rotationAmount);
private:
  Stepper *stepper;
  bool isPaused;
  int rotationAmount;
  int rotatedSteps = 0;
  static bool motorsActive;
};
