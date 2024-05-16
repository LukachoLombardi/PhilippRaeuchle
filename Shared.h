#pragma once
#include <Logger.h>
#include <LAS.h>
#include "PhilippSettings.h"
#include <Arduino.h>

namespace Shared {
extern Logger logger;
extern LAS las;
void printPhilipp();
}
