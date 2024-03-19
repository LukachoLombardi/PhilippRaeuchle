#pragma once

#include "CommonIncludes.h"

namespace DriveControls {
using namespace Shared;

extern bool driving;

extern bool lastMotorState;
extern bool rotationActive;

extern float currentVehicleRotation;

void initSteppers();

bool checkMotorActivity();
bool checkMotorActivitySilent();
bool isRotationActive();
void setRotationVar(float pi_mul);

void drive();
void stop();
void driveKeepalive();
void rotationCheckKeepalive();

void rotateLeftMotorAsync(int steps);

void rotateRightMotorAsync(int steps);

void rotateMotorsAsync(int steps1, int steps2);

void driveStepsForward(int steps);

void rotateVehicleByAsync(float rot_mul);

void rotateVehicleToAsync(float rot_mul);

void driveSizeUnits(float units);
}
