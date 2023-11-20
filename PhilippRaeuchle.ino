#include <LAS.h>
#include <LASConfig.h>

#include "PhilippSettings.h"

#include <Stepper.h>
#include <Wire.h>
#include <Logger.h>
#include <Adafruit_TCS34725.h>

namespace Shared{
  Logger logger = Logger();

  char* concat(char* string0, char* string1){
    char buffer[SMALL_CHAR_BUFFER_SIZE] = {};
    strcat(buffer, string0);
    strcat(buffer, string1);
    return buffer;
  }

  char* concat(char* string0, int num){
    char buffer[SMALL_CHAR_BUFFER_SIZE] = {};
    sprintf(buffer, "%s%d", string0, num);
    return buffer;
  }

  char* concat(char* string0, float num){
    char buffer[SMALL_CHAR_BUFFER_SIZE] = {};
    sprintf(buffer, "%s%f", string0, num);
    return buffer;
  }
}

namespace Navigation {
  using namespace Shared;

  Stepper leftMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, LEFT_MOTOR_PIN_0, LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, LEFT_MOTOR_PIN_3);
  Stepper rightMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, RIGHT_MOTOR_PIN_0, RIGHT_MOTOR_PIN_1, RIGHT_MOTOR_PIN_2, RIGHT_MOTOR_PIN_3);

  bool motorsActive = false;
  int currentVehicleRotation = 0;

  bool checkMotorActivity(){
    if(motorsActive){
      logger.printline("rotation blocked because of ongoing rotation", "warning");
      return true;
    }
    return false;
  }

  void initSteppers() {
    leftMotor.setSpeed(10);
    rightMotor.setSpeed(10);
    logger.printline("initialized steppers");
  }

  class StepperRotator: public LAS::Callable{
    public:
      void run() override{
        motorsActive = true;
        this->stepper->step(rotationAmount);
        if(taskPtr->remainingRepeats == 1){
          motorsActive = false;
        }
      }
      StepperRotator(Stepper *stepper, int rotationAmount): stepper(stepper), rotationAmount(rotationAmount) {}
    private:
      Stepper *stepper;
      int rotationAmount;
      int rotatedSteps = 0;
  };

  void rotateLeftMotorAsync(int steps){
    if(checkMotorActivity()){
      return;
    }
    LAS::scheduleRepeated(new StepperRotator(&leftMotor, MOTOR_STEPSIZE * (abs(steps)/steps)), steps/MOTOR_STEPSIZE);
    logger.printline(concat("rotating left motor by ", steps) , "debug");
  }

  void rotateRightMotorAsync(int steps){
    if(checkMotorActivity()){
      return;
    }
    LAS::scheduleRepeated(new StepperRotator(&rightMotor, MOTOR_STEPSIZE * (abs(steps)/steps)), steps/MOTOR_STEPSIZE);
    logger.printline(concat("rotating right motor by ", steps) , "debug");
  }

  void driveStepsForward(int steps){
    rotateRightMotorAsync(steps);
    rotateRightMotorAsync(steps);
    logger.printline(concat("driving forward ", steps));
  }

  void setRotationVar(float pi_mul){
    currentVehicleRotation = pi_mul;
    while(currentVehicleRotation >= 2){
      currentVehicleRotation -= 2;
    }
  }

  void RotateVehicleByAsync(float pi_mul){
    if(checkMotorActivity()){
      return;
    }
    int steps = int(ROTATION_REVOLUTIONS * (pi_mul/2) * MOTOR_STEPS_PER_REVOLUTION);
    logger.printline(concat("rotating vehicle by pi_mul ", pi_mul));
    //left motor task handles the advanced settings
    LAS::scheduleRepeated([]() {
      motorsActive = true;
      leftMotor.step(MOTOR_STEPSIZE * -1);
      setRotationVar(currentVehicleRotation + PI_MUL_PER_STEPSIZE);
      if(LAS::getActiveTask().remainingRepeats == 1){
        motorsActive = false;
      }
    },
    ASAP, abs(int(steps / MOTOR_STEPSIZE)));
    LAS::scheduleRepeated([]() {
      rightMotor.step(MOTOR_STEPSIZE);
    },
    ASAP, abs(int(steps / MOTOR_STEPSIZE)));
  }

  void rotateVehicleTo(float pi_mul){
    RotateVehicleByAsync(pi_mul - currentVehicleRotation);
  }

  void driveSizeUnits(float units){
    driveStepsForward(int(units * VEHICLE_STEPS_X));
  }
}

namespace Sensors {
  using namespace Shared;
  namespace ColorSensor {
    using namespace Shared;
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

    void readColor(float *r, float *g, float *b){
      tcs.getRGB(r, g, b);
    }

    class: public LAS::Callable{
      public:
        void run() override{
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
        LAS::scheduleRepeated(&colorReader, 50, ENDLESS_LOOP);
    }

    void initUltrasoundSensorsAsync() {

    }
  }
}

using namespace Shared;

class: public LAS::Callable{
    public:
      void run() override{
        if(Serial.available() <= 0){
          return;
        }
        readSerial();
        handleBuffer();
      }

      void readSerial(){
        if(Serial.available()>CMD_CHAR_BUFFER_SIZE){
          Serial.flush();
          logger.printline("Serial input buffer too large, deleted.", "warning");
          return;
        }
        memset(serialBuffer, 0, CMD_CHAR_BUFFER_SIZE * (sizeof(char)));
        serialBuffer[CMD_CHAR_BUFFER_SIZE-1] = "\0";
        while(Serial.available() > 0) {
          strcat(serialBuffer, Serial.read());
        }
      }
      void handleBuffer(){
        if(strcmp(serialBuffer, "HALT") == 0){
          logger.printline("HALTING EXECUTION!", "severe");
          while(true);
        }
        if(strcmp(serialBuffer, "TASKS") == 0){
          LAS::printSchedule();
          return;
        }
      }
    private:
      char serialBuffer[CMD_CHAR_BUFFER_SIZE] = "";
  } serialConsole;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Welcome to");
  Serial.println(" (                                (                                         ");
  Serial.println(" )\\ )    )     (   (              )\\ )                          )  (        ");
  Serial.println("(()/( ( /( (   )\\  )\\ (          (()/(    )    (    (        ( /(  )\\   (   ");
  Serial.println(" /(_)))\\()))\\ ((_)((_))\\  `  )    /(_))( /(   ))\\  ))\\   (   )\\())((_) ))\\  ");
  Serial.println("(_)) ((_)\\((_) _   _ ((_) /(/(   (_))  )(_)) /((_)/((_)  )\\ ((_)\\  _  /((_) ");
  Serial.println("| _ \\| |(_)(_)| | | | (_)((_)_\\  | _ \\((_)_ (_)) (_))(  ((_)| |(_)| |(_))   ");
  Serial.println("|  _/| ' \\ | || | | | | || '_ \\) |   // _` |/ -_)| || |/ _| | ' \\ | |/ -_)  ");
  Serial.println("|_|  |_||_||_||_| |_| |_|| .__/  |_|_\\\\__,_|\\___| \\_,_|\\__| |_||_||_|\\___|  ");
  Serial.println("                         |_|                                                ");
  Serial.println("");

  Serial.begin(9600);  
  logger.printline("Logger started");
  
  logger.init(&Serial);
  logger.printline("PhilippRaeuchle started");
 
  LAS::initScheduler(logger);
  LAS::scheduleRepeated(&serialConsole);
  LAS::scheduleFunction(Navigation::initSteppers, ASAP);
  LAS::scheduleFunction(Sensors::ColorSensor::initColorSensorAsync, ASAP);
  LAS::startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
