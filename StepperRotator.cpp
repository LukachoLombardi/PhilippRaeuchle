#include "StepperRotator.h"
void StepperRotator::run(){
  if (!isPaused) {
    motorsActive = true;
    this->stepper->step(rotationAmount);
    rotatedSteps += rotationAmount;
    if (taskPtr->remainingRepeats == 1) {
      motorsActive = false;
      return;
    }
  }
}
void StepperRotator::pause() {
  motorsActive = false;
  isPaused = true;
}
void StepperRotator::resume() {
  motorsActive = true;
  isPaused = false;
}
bool StepperRotator::isActive() {
  return !isPaused;
}
bool StepperRotator::areActive() {
  return motorsActive;
}
void StepperRotator::unblock() {
  motorsActive = false;
}
StepperRotator::StepperRotator(Stepper *stepper, int rotationAmount)
  : stepper(stepper), rotationAmount(rotationAmount) {}

bool StepperRotator::motorsActive = false;
