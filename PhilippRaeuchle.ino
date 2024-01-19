#include <LAS.h>
#include <LASConfig.h>

#include "PhilippSettings.h"
#include "StepperRotator.h"
#include "VehicleRotation.h"

#include <Stepper.h>
#include <Wire.h>
#include <Logger.h>
#include <Adafruit_TCS34725.h>
#include <Adafruit_VL53L0X.h>

namespace Shared {
Logger logger = Logger();
void printPhilipp() {
  Serial.println("\r\n\r\n (                                  (                                         \r\n )\\ )    )     (                    )\\ )                          )  (        \r\n(()/( ( /( (   )\\ (                (()/(    )    (    (        ( /(  )\\   (   \r\n /(_)))\\()))\\ ((_))\\  `  )   `  )   /(_))( /(   ))\\  ))\\   (   )\\())((_) ))\\  \r\n(_)) ((_)\\((_) _ ((_) /(/(   /(/(  (_))  )(_)) /((_)/((_)  )\\ ((_)\\  _  /((_) \r\n| _ \\| |(_)(_)| | (_)((_)_\\ ((_)_\\ | _ \\((_)_ (_)) (_))(  ((_)| |(_)| |(_))   \r\n|  _/| \' \\ | || | | || \'_ \\)| \'_ \\)|   // _` |/ -_)| || |/ _| | \' \\ | |/ -_)  \r\n|_|  |_||_||_||_| |_|| .__/ | .__/ |_|_\\\\__,_|\\___| \\_,_|\\__| |_||_||_|\\___|  \r\n                     |_|    |_|                                             ");
}
}

namespace Sensors {
using namespace Shared;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_VL53L0X tof_fw_low = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_fw_high = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_left = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_right = Adafruit_VL53L0X();
Adafruit_VL53L0X tof_down = Adafruit_VL53L0X();

VL53L0X_RangingMeasurementData_t tof_measure_fw_low;
VL53L0X_RangingMeasurementData_t tof_measure_fw_high;
VL53L0X_RangingMeasurementData_t tof_measure_left;
VL53L0X_RangingMeasurementData_t tof_measure_right;
VL53L0X_RangingMeasurementData_t tof_measure_down;

void TOFAddrInit(Adafruit_VL53L0X &tof, int xshutPin, int addr) {
  digitalWrite(xshutPin, HIGH);
  tof.begin();
  tof.setAddress(addr);
}

void readTOFMMs() {
  logger.printline("measuring TOFs...", "debug");
  tof_fw_low.rangingTest(&tof_measure_fw_low, false);
  tof_fw_high.rangingTest(&tof_measure_fw_high, false);
  tof_left.rangingTest(&tof_measure_left, false);
  tof_right.rangingTest(&tof_measure_right, false);
  tof_down.rangingTest(&tof_measure_left, false);
}

void initTOFSensorsAsync() {
  pinMode(TOF_XSHUT_FW_LOW, OUTPUT);
  pinMode(TOF_XSHUT_FW_HIGH, OUTPUT);
  pinMode(TOF_XSHUT_LEFT, OUTPUT);
  pinMode(TOF_XSHUT_RIGHT, OUTPUT);
  pinMode(TOF_XSHUT_DOWN, OUTPUT);
  pinMode(TOF_XSHUT_FW_LOW, OUTPUT);

  digitalWrite(TOF_XSHUT_FW_HIGH, LOW);
  digitalWrite(TOF_XSHUT_FW_LOW, LOW);
  digitalWrite(TOF_XSHUT_LEFT, LOW);
  digitalWrite(TOF_XSHUT_RIGHT, LOW);
  digitalWrite(TOF_XSHUT_DOWN, LOW);

  TOFAddrInit(tof_fw_low, TOF_XSHUT_FW_LOW, 0x2A);
  TOFAddrInit(tof_fw_high, TOF_XSHUT_FW_HIGH, 0x2B);
  TOFAddrInit(tof_left, TOF_XSHUT_LEFT, 0x2C);
  TOFAddrInit(tof_right, TOF_XSHUT_RIGHT, 0x2D);
  TOFAddrInit(tof_down, TOF_XSHUT_DOWN, 0x2E);
}

void readColor(float *r, float *g, float *b) {
  tcs.getRGB(r, g, b);
}

class : public LAS::Callable {
public:
  void run() override {
    readColor(&red, &green, &blue);
  }
private:
  float red, green, blue;
} colorReader;

void initColorSensorAsync() {
  if (!tcs.begin()) {
    logger.printline("No TCS34725 found!", "severe");
    return;
  }
  LAS::scheduleRepeated(&colorReader, 50, ENDLESS_LOOP, false);
  logger.printline("initialized TCS");
}
}

namespace Navigation {
using namespace Shared;

Stepper leftMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, LEFT_MOTOR_PIN_0, LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, LEFT_MOTOR_PIN_3);
Stepper rightMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, RIGHT_MOTOR_PIN_0, RIGHT_MOTOR_PIN_1, RIGHT_MOTOR_PIN_2, RIGHT_MOTOR_PIN_3);

void initSteppers() {
  leftMotor.setSpeed(10);
  rightMotor.setSpeed(10);
  logger.printline("initialized steppers");
}

bool checkMotorActivity() {
  if (StepperRotator::areActive()) {
    logger.printline("rotation blocked because of ongoing rotation", "warning");
    return true;
  }
  return false;
}

void rotateLeftMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  int repeats = steps / MOTOR_STEPSIZE;
  LAS::scheduleRepeated(new StepperRotator(&leftMotor, MOTOR_STEPSIZE * (abs(steps) / steps)), repeats);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating left motor by %d", steps);
  logger.printline(buffer, "debug");
}

void rotateRightMotorAsync(int steps) {
  if (checkMotorActivity()) {
    return;
  }
  int repeats = steps / MOTOR_STEPSIZE;
  LAS::scheduleRepeated(new StepperRotator(&rightMotor, MOTOR_STEPSIZE * (abs(steps) / steps)), repeats);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating right motor by %d", steps);
  logger.printline(buffer, "debug");
}

StepperRotator *scheduleConstantRightRotatorAsync() {
  StepperRotator *rotator = new StepperRotator(&rightMotor, MOTOR_STEPSIZE);
  LAS::scheduleRepeated(rotator);
  if (checkMotorActivity()) {
    rotator->pause();
  }
  logger.printline("start const driving right motor", "debug");
  return rotator;
}

StepperRotator *scheduleConstantLeftRotatorAsync() {
  StepperRotator *rotator = new StepperRotator(&rightMotor, MOTOR_STEPSIZE);
  LAS::scheduleRepeated(rotator);
  if (checkMotorActivity()) {
    rotator->pause();
  }
  logger.printline("start const driving left motor", "debug");
  return rotator;
}

void driveStepsForward(int steps) {
  rotateRightMotorAsync(steps);
  rotateRightMotorAsync(steps);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "driving %d steps", steps);
  logger.printline(buffer, "info");
}

void rotateVehicleByAsync(float pi_mul) {
  if (VehicleRotation::isRotationActive()) {
    return;
  }
  int steps = int(ROTATION_REVOLUTIONS * (pi_mul / 2) * MOTOR_STEPS_PER_REVOLUTION);
  char buffer[BUFFER_SIZE] = "";
  snprintf(buffer, BUFFER_SIZE, "rotating vehicle by %d", steps);
  logger.printline(buffer, "info");
  bool l = true;
  if (steps < 0) {
    l = false;
  }
  LAS::scheduleRepeated(new VehicleRotation(l, &leftMotor, &rightMotor), ASAP, abs(int(steps / MOTOR_STEPSIZE) * 2));
}

void rotateVehicleToAsync(float pi_mul) {
  rotateVehicleByAsync(pi_mul - VehicleRotation::getCurrentVehicleRotation());
}

void driveSizeUnits(float units) {
  driveStepsForward(int(units * VEHICLE_STEPS_X));
}

class : public LAS::Callable {
public:
  void run() override {
    if (VehicleRotation::isRotationActive()) {
      return;
    }
    Sensors::readTOFMMs();
    if (Sensors::tof_measure_down.RangeMilliMeter >= MAX_TABLE_DISTANCE && Sensors::tof_measure_down.RangeStatus != 4) {
      logger.printline("end of table reached.");
      pauseDriving();
      if (state == DRIVE) {
        setState(TRANSFER);
      }
      if (state == AVOID_ENTRY || state == AVOID_EXIT) {
        rotateVehicleByAsync(1);
        setState(DRIVE);
        directionR = !directionR;
      }
      if (state == TRANSFER) {
        logger.printline("Table Finished!");
        taskPtr->isActive = false;
      }
      return;
    }
    if (state == DRIVE) {
      if ((Sensors::tof_measure_fw_low.RangeStatus != 4 && Sensors::tof_measure_fw_low.RangeMilliMeter >= SAFETY_DISTANCE)) {
        logger.printline("no obstacle detected", "debug");
        resumeDriving();
      } else {
        logger.printline("obstacle detected! Starting AVOID behaviour");
        setState(AVOID_ENTRY);
        pauseDriving();
        rotateVehicleByAsync(3 / 2);
      }
    } else if (state == AVOID_ENTRY) {
      if (Sensors::tof_measure_left.RangeStatus != 4 && Sensors::tof_measure_left.RangeMilliMeter >= UNITY_DISTANCE) {
        resumeDriving();
      } else {
        pauseDriving();
        lastLDist = -1;
        setState(AVOID_EXIT);
      }
    } else if (state == AVOID_EXIT) {
      if (lastLDist == -1 || (Sensors::tof_measure_left.RangeStatus != 4 && abs(Sensors::tof_measure_left.RangeMilliMeter - lastLDist) <= UNITY_DISTANCE)) {
        resumeDriving();
      } else {
        pauseDriving();
        rotateVehicleByAsync(1 / 4);
        avoidStage++;
        if (avoidStage <= 3) {
          setState(AVOID_ENTRY);
        } else {
          avoidStage = 0;
          if (directionR) {
            rotateVehicleToAsync(0);
          } else {
            rotateVehicleToAsync(1);
          }
          setState(DRIVE);
        }
        return;
      }
      lastLDist = -1;  // might be fucky, test!
      if (Sensors::tof_measure_left.RangeStatus != 4) {
        lastLDist = Sensors::tof_measure_left.RangeMilliMeter;
      }
    }
  }
  int getStateId() {
    return int(state);
  }
  int getAvoidStage() {
    return avoidStage;
  }
  void init() {
    delete rotatorLeft;
    delete rotatorRight;
    rotatorLeft = Navigation::scheduleConstantLeftRotatorAsync();
    rotatorRight = Navigation::scheduleConstantRightRotatorAsync();
  }
private:
  enum NavState { DRIVE = 0,
                  AVOID_ENTRY = 1,
                  AVOID_EXIT = 2,
                  COLLECT = 3,
                  TRANSFER = 4,
                  FINISH = 5 };
  int avoidStage = 0;
  void setState(NavState state) {
    this->state = state;
    char buffer[BUFFER_SIZE];
    strcpy(buffer, "switched state to: ");
    strcat("buffer", int(state));
    logger.printline(buffer);
  }
  void pauseDriving() {
    logger.printline("setting steppers inactive", "debug"); // might wanna get rid of these
    rotatorLeft->pause();
    rotatorRight->pause();
  }
  void resumeDriving() {
    logger.printline("setting steppers active", "debug");
    rotatorLeft->resume();
    rotatorRight->resume();
  }
  int lastLDist = 0;
  bool directionR = true;
  NavState state = DRIVE;
  StepperRotator *rotatorLeft = nullptr;
  StepperRotator *rotatorRight = nullptr;
} driver;

}

using namespace Shared;

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
  LAS::scheduleFunction(Navigation::initSteppers);
  LAS::scheduleFunction(Sensors::initColorSensorAsync);
  LAS::scheduleFunction(Sensors::initTOFSensorsAsync);
  //add tof and driver

  LAS::startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
