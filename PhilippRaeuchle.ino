#include <LAS.h>
#include <LASConfig.h>
#include <Logger.h>

#include "PhilippSettings.h"
#include "Shared.h"
#include "Sensors.h"
#include "Navigation.h"

using namespace Shared;

bool diagActive = false;
void printDiag() {
  if(!diagActive){
    return;
  }
  char buffer[BUFFER_SIZE] = "";
  Sensors::readTOFMMs();
  snprintf(buffer, BUFFER_SIZE, "motorStateLeft is %d motorStateRight is %d", int(Navigation::driver.isLeftMotorActive()), int(Navigation::driver.isRightMotorActive()));
  logger.printline(buffer, "debug");
  strcpy(buffer, "");
  snprintf(buffer, BUFFER_SIZE, "tofs are: %d;%d;%d;%d;%d", Sensors::tof_measure_fw_low.RangeMilliMeter, 
  Sensors::tof_measure_fw_high.RangeMilliMeter,
  Sensors::tof_measure_left.RangeMilliMeter,
  Sensors::tof_measure_right.RangeMilliMeter,
  Sensors::tof_measure_down.RangeMilliMeter);
  logger.printline(buffer, "debug");
}

class : public LAS::Callable {
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
      LAS::printSchedule();
      return true;
    }
    if (strcmp(serialBuffer, "CLEARTASKS") == 0) {
      LAS::clearSchedule();
      LAS::scheduleRepeated(this);
      return true;
    }
    if (strcmp(serialBuffer, "CLEARALLTASKS") == 0) {  //also clears itself
      LAS::clearSchedule();
      return true;
    }
    if (strcmp(serialBuffer, "STOPCONSOLE") == 0) {
      this->taskPtr->isActive = false;
      return true;
    }
    if (strcmp(serialBuffer, "UNBLOCKMOTOR") == 0) {
      StepperRotator::unblock();
      return true;
    }
    if(strcmp(serialBufferm "TOGGLEDIAG")){
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
      Navigation::rotateVehicleByAsync(1);
      return true;
    }
    if (strcmp(serialBuffer, "DRIVETEST") == 0) {
      Navigation::driveSizeUnits(5);
      Navigation::driveSizeUnits(-5);
      return true;
    }
    if (strcmp(serialBuffer, "NAVINFO") == 0) {
      char buffer[BUFFER_SIZE];
      strcpy(buffer, "");
      snprintf(buffer, BUFFER_SIZE, "current rot: %d \n navState: %d \n avoidStage: %d", VehicleRotation::getCurrentVehicleRotation(), Navigation::driver.getStateId(), Navigation::driver.getAvoidStage());
      logger.printline(buffer);
      return true;
    }
    if (strcmp(serialBuffer, "PHILIPP") == 0) {
      printPhilipp();
      LAS::scheduleRepeated(&Navigation::driver, ASAP, ENDLESS_LOOP, false);
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

  LAS::initScheduler(logger);
  Navigation::driver.init();

  LAS::scheduleRepeated(&serialConsole, ASAP, ENDLESS_LOOP, false);
  // TODO: Add other annoying debug messages about driving to the diag, add command to toggle it
  LAS::scheduleRepeated(printDiag, 5000, ENDLESS_LOOP);
  LAS::scheduleFunction(Navigation::initSteppers);
  LAS::scheduleFunction(Sensors::initColorSensorAsync);
  LAS::scheduleFunction(Sensors::initTOFSensorsAsync);
  //add tof and driver

  LAS::startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
