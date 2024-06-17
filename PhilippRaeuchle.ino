#include <LAS.h>
#include <LASConfig.h>
#include <Callable.h>
#include <LASMacros.h>

#include <Logger.h>

#include "PhilippSettings.h"
#include "Shared.h"
#include "Sensors.h"
#include "Navigation.h"

using namespace Shared;

bool diagActive = false;
void printDiag() {
  if (!diagActive) {
    return;
  }
  Serial.println(">--- DEBUG PRINT ---");

  char buffer[BUFFER_SIZE] = "";
  Sensors::readTOFMMs();

  snprintf(buffer, BUFFER_SIZE, "navState: %d \n avoidStage: %d", Navigation::driver.getStateId(), Navigation::driver.getAvoidStage());
  logger.printline(buffer, "debug");
  Serial.print("currentRot:");
  Serial.println(DriveControls::currentVehicleRotation);

  strcpy(buffer, "");
  snprintf(buffer, BUFFER_SIZE, "tofs are: %d;%d;%d;%d;%d", Sensors::tof_measure_fw_low.RangeMilliMeter,
           Sensors::tof_measure_fw_high.RangeMilliMeter,
           Sensors::tof_measure_left.RangeMilliMeter,
           Sensors::tof_measure_right.RangeMilliMeter,
           Sensors::tof_measure_down.RangeMilliMeter);
  logger.printline(buffer, "debug");

  Serial.println("--- DEBUG PRINT --->");
}

class : public Callable {
public:
  void run() override {
    if (Serial.available() <= 0) {
      return;
    }
    delay(10);
    readSerial();
    execute();
  }

  void simulateInput(char *input) {
    strcpy(serialBuffer, input);
    execute();
  }
private:
  void readSerial() {
    logger.printline("RECEIVING COMMAND");
    strcpy(serialBuffer, "");
    while (Serial.available() > 0) {
      char serialByte[2] = "";
      serialByte[0] = Serial.read();
      if (strcmp(serialByte, "\n") == 0) {
        break;
      }
      strcat(serialBuffer, serialByte);
    }
  }
  void execute() {
    if (handleBuffer()) {
      logger.printline("EXECUTION SUCCESSFUL");
    } else {
      logger.printline("EXECUTION FAILED: CMD NOT FOUND");
    }
  }
  bool handleBuffer() {
    char buffer[BUFFER_SIZE] = "";
    memset(buffer, 0, BUFFER_SIZE * (sizeof(char)));
    strcat(buffer, "ATTEMPTING COMMAND \"");
    strcat(buffer, serialBuffer);
    strcat(buffer, "\"");
    logger.printline(buffer);
    if (strcmp(serialBuffer, "HALT") == 0) {
      logger.printline("HALTING EXECUTION!", "severe");
      while (true)
        ;
    }
    if (strcmp(serialBuffer, "TASKS") == 0) {
      las.printSchedule();
      return true;
    }
    if (strcmp(serialBuffer, "CLEARTASKS") == 0) {
      las.clearSchedule();
      las.scheduleRepeated(this);
      return true;
    }
    if (strcmp(serialBuffer, "CLEARALLTASKS") == 0) {  //also clears itself
      las.clearSchedule();
      return true;
    }
    if (strcmp(serialBuffer, "STOPCONSOLE") == 0) {
      finish();
      return true;
    }
    if (strcmp(serialBuffer, "TOGGLEDIAG") == 0) {
      diagActive = !diagActive;
      return true;
    }
    if (strcmp(serialBuffer, "TOGGLEINFO") == 0) {
      Logger::LogConfig conf = logger.getConf();
      conf.info ^= true;
      logger.setConf(conf);
      return true;
    }
    if (strcmp(serialBuffer, "TOGGLEDEBUG") == 0) {
      Logger::LogConfig conf = logger.getConf();
      conf.debug ^= true;
      logger.setConf(conf);
      return true;
    }
    if (strcmp(serialBuffer, "TOGGLEWARNING") == 0) {
      Logger::LogConfig conf = logger.getConf();
      conf.warning ^= true;
      logger.setConf(conf);
      return true;
    }
    if (strcmp(serialBuffer, "STEPPERTEST") == 0) {
      Navigation::rotateVehicleByAsync(0.25);
      return true;
    }
    if (strcmp(serialBuffer, "DRIVETEST") == 0) {
      Navigation::driveSizeUnits(5);
      return true;
    }
    if (strcmp(serialBuffer, "PHILIPP") == 0) {
      printPhilipp();
      Navigation::driver = Driver();
      las.scheduleRepeated(&Navigation::driver, ASAP, ENDLESS_LOOP, false);
      return true;
    }
    if (strcmp(serialBuffer, "RESET") == 0) {
      logger.printline("USER RESET...", "severe");
      delay(10);
      void (*resetFunc)(void) = 0;
      resetFunc();
      return true;
    }
    return false;
  }
  char serialBuffer[BUFFER_SIZE] = "";
} serialConsole;

void setup() {
  // put your setup code here, to run once:
  interrupts();

  Serial.begin(BAUDRATE);
  Serial.println("Welcome to");
  printPhilipp();
  logger.printline("PhilippRaeuchle started");

  LASConfig conf = LASConfig(
    16,
    false,
    50,
    128
  );
  las.initScheduler(conf, logger);

  las.scheduleRepeated(&serialConsole, ASAP, ENDLESS_LOOP, false);
  // TODO: Add other annoying debug messages about driving to the diag, add command to toggle it
  las.scheduleRepeated(printDiag, 5000, ENDLESS_LOOP);
  las.scheduleFunction(Navigation::initSteppers);
  las.scheduleFunction(Sensors::initColorSensorAsync);
  las.scheduleFunction(Sensors::initTOFSensorsAsync);

  las.scheduleRepeated(Sensors::readTOFMMs, 200, ENDLESS_LOOP);
  las.scheduleFunction(Sensors::readTOFMMs);
  las.scheduleRepeated(&Navigation::driver, 500);

  //Navigation::driver.init();

  las.startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
