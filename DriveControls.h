#pragma once

#include "CommonIncludes.h"

#include <Stepper.h>
#include "StepperRotator.h"
#include "VehicleRotation.h"

namespace DriveControls {
using namespace Shared;

extern Stepper leftMotor;
extern Stepper rightMotor;

void initSteppers();

bool checkMotorActivity();

void rotateLeftMotorAsync(int steps);

void rotateRightMotorAsync(int steps);

StepperRotator *scheduleConstantRightRotatorAsync();

StepperRotator *scheduleConstantLeftRotatorAsync();

void driveStepsForward(int steps);

void rotateVehicleByAsync(float pi_mul);

void rotateVehicleToAsync(float pi_mul);

void driveSizeUnits(float units);
}
