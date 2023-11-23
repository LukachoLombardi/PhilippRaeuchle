#include <LAS.h>
#include <LASConfig.h>

#include "PhilippSettings.h"

#include <Stepper.h>
#include <Wire.h>
#include <Logger.h>
#include <Adafruit_TCS34725.h>

namespace Shared{
  Logger logger = Logger();
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
    char buffer [BUFFER_SIZE] = "";
    snprintf(buffer, BUFFER_SIZE, "rotating left motor by %d", steps);
    logger.printline(buffer , "debug");
  }

  void rotateRightMotorAsync(int steps){
    if(checkMotorActivity()){
      return;
    }
    LAS::scheduleRepeated(new StepperRotator(&rightMotor, MOTOR_STEPSIZE * (abs(steps)/steps)), steps/MOTOR_STEPSIZE);
    char buffer [BUFFER_SIZE] = "";
    snprintf(buffer, BUFFER_SIZE, "rotating right motor by %d", steps);
    logger.printline(buffer , "debug");
  }

  void driveStepsForward(int steps){
    rotateRightMotorAsync(steps);
    rotateRightMotorAsync(steps);
    char buffer [BUFFER_SIZE] = "";
    snprintf(buffer, BUFFER_SIZE, "driving %d steps", steps);
    logger.printline(buffer , "info");
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
    char buffer [BUFFER_SIZE] = "";
    snprintf(buffer, BUFFER_SIZE, "rotating vehicle by %d", steps);
    logger.printline(buffer, "info");
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

  void rotateVehicleToAsync(float pi_mul){
    RotateVehicleByAsync(pi_mul - currentVehicleRotation);
  }

  void driveSizeUnits(float units){
    driveStepsForward(int(units * VEHICLE_STEPS_X));
  }
}

namespace Sensors {
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

    class DistanceReader: public LAS::Callable{
      public:
        void run() override {
          pulseIn = 0;
          //error correction
          if(pulseIn - pulseOut > ULTRASOUND_ERROR_THRESHOLD){
            if(ULTRASOUND_VALIDATION_CYCLES > validationCounter) { 
              validationCounter++;
              return;
            }
            else {
              validationCounter = 0;
            }
          }
          pulseDelay = max(-1, pulseIn - pulseOut);
          pulseOut = micros();
          pulse();
          attachInterrupt(digitalPinToInterrupt(inPin), pulseInISR, CHANGE);
        }
        DistanceReader(int outPin, int inPin) {
          this->outPin = outPin;
          pinMode(outPin, OUTPUT);
          pinMode(inPin, INPUT);
        }
      private:
        int validationCounter = 0;
        int outPin;
        long pulseOut = 0;
        volatile long pulseIn = 0;
        int pulseDelay = 0;
        void pulse() {
          digitalWrite(outPin, LOW);
          digitalWrite(outPin, HIGH);
          digitalWrite(outPin, LOW);
        }
        void pulseInISR(){
          pulseIn = micros();
        }
    };
    
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

using namespace Shared;

class: public LAS::Callable{
    public:
      void run() override{
        if(Serial.available() <= 0){
          return;
        }
        delay(10);
        readSerial();
        execute();
      }

      void simulateInput(char* input) {
        strcpy(serialBuffer, input);
        execute();
      }
    private:
    void readSerial(){
        logger.printline("RECEIVING COMMAND");
        strcpy(serialBuffer, "");
        while(Serial.available() > 0) {
          char serialByte[2] = "";
          serialByte[0] = Serial.read();
          strcat(serialBuffer, serialByte);
        }
      }
      void execute(){
        if(handleBuffer()){
          logger.printline("EXECUTION SUCCESSFUL");
        } else {
          logger.printline("EXECUTION FAILED: CMD NOT FOUND");
        }
      }
      bool handleBuffer(){
        char buffer[BUFFER_SIZE] = "";
        memset(buffer, 0, BUFFER_SIZE * (sizeof(char)));
        strcat(buffer, "ATEMPTING COMMAND ");
        strcat(buffer, serialBuffer);
        logger.printline(buffer);
        if(strcmp(serialBuffer, "HALT") == 0){
          logger.printline("HALTING EXECUTION!", "severe");
          while(true);
        }
        if(strcmp(serialBuffer, "TASKS") == 0){
          LAS::printSchedule();
          return true;
        }
        if(strcmp(serialBuffer, "CLEARTASKS") == 0){
          LAS::clearSchedule();
          LAS::scheduleRepeated(this);
          return true;
        }
        if(strcmp(serialBuffer, "CLEARALLTASKS") == 0){ //also clears itself
          LAS::clearSchedule();
          return true;
        }
        if(strcmp(serialBuffer, "STOPCONSOLE") == 0){
          this->taskPtr->isActive = false;
          return true;
        }
        if(strcmp(serialBuffer, "TOGGLEINFO") == 0){
          Logger::LogConfig conf = logger.getConf();
          conf.info ^= true;
          logger.setConf(conf);
          return true;
        }
        if(strcmp(serialBuffer, "TOGGLEDEBUG") == 0){
          Logger::LogConfig conf = logger.getConf();
          conf.debug ^= true;
          logger.setConf(conf);          return;
        }
        if(strcmp(serialBuffer, "TOGGLEWARNING") == 0){
          Logger::LogConfig conf = logger.getConf();
          conf.warning ^= true;
          logger.setConf(conf);
          return true;
        }
        if(strcmp(serialBuffer, "STEPPERTEST") == 0){
          Navigation::RotateVehicleByAsync(1);
          return true;
        }
<<<<<<< Updated upstream
        if(strcmp(serialBuffer, "PHILIPP") == 0){
          //add algorithm execution here
          return true;
        }
=======
        if(strcmp(serialBuffer, "RESET") == 0){
          logger.printline("USER RESET...", "severe");
          delay(1000);
          void(* resetFunc) (void) = 0;
          resetFunc();
          return true;
        }
        return false;
>>>>>>> Stashed changes
      }
      char serialBuffer[BUFFER_SIZE] = "";
  } serialConsole;

void setup() {
  // put your setup code here, to run once:
  interrupts();
  
  Serial.begin(BAUDRATE);
  Serial.println("Welcome to");
  Serial.println("\r\n\r\n (                                  (                                         \r\n )\\ )    )     (                    )\\ )                          )  (        \r\n(()/( ( /( (   )\\ (                (()/(    )    (    (        ( /(  )\\   (   \r\n /(_)))\\()))\\ ((_))\\  `  )   `  )   /(_))( /(   ))\\  ))\\   (   )\\())((_) ))\\  \r\n(_)) ((_)\\((_) _ ((_) /(/(   /(/(  (_))  )(_)) /((_)/((_)  )\\ ((_)\\  _  /((_) \r\n| _ \\| |(_)(_)| | (_)((_)_\\ ((_)_\\ | _ \\((_)_ (_)) (_))(  ((_)| |(_)| |(_))   \r\n|  _/| \' \\ | || | | || \'_ \\)| \'_ \\)|   // _` |/ -_)| || |/ _| | \' \\ | |/ -_)  \r\n|_|  |_||_||_||_| |_|| .__/ | .__/ |_|_\\\\__,_|\\___| \\_,_|\\__| |_||_||_|\\___|  \r\n                     |_|    |_|                                             ");
    
  logger.printline("PhilippRaeuchle started");
 
  LAS::initScheduler(logger);

  LAS::scheduleRepeated(&serialConsole);
  LAS::scheduleFunction(Navigation::initSteppers);
  LAS::scheduleFunction(Sensors::initColorSensorAsync);
  LAS::startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
  }
