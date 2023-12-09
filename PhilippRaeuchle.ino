#include <LAS.h>
#include <LASConfig.h>

#include "PhilippSettings.h"

#include <Stepper.h>
#include <Wire.h>
#include <Logger.h>
#include <Adafruit_TCS34725.h>

namespace Shared{
  Logger logger = Logger();
  void printPhilipp(){
    Serial.println("\r\n\r\n (                                  (                                         \r\n )\\ )    )     (                    )\\ )                          )  (        \r\n(()/( ( /( (   )\\ (                (()/(    )    (    (        ( /(  )\\   (   \r\n /(_)))\\()))\\ ((_))\\  `  )   `  )   /(_))( /(   ))\\  ))\\   (   )\\())((_) ))\\  \r\n(_)) ((_)\\((_) _ ((_) /(/(   /(/(  (_))  )(_)) /((_)/((_)  )\\ ((_)\\  _  /((_) \r\n| _ \\| |(_)(_)| | (_)((_)_\\ ((_)_\\ | _ \\((_)_ (_)) (_))(  ((_)| |(_)| |(_))   \r\n|  _/| \' \\ | || | | || \'_ \\)| \'_ \\)|   // _` |/ -_)| || |/ _| | \' \\ | |/ -_)  \r\n|_|  |_||_||_||_| |_|| .__/ | .__/ |_|_\\\\__,_|\\___| \\_,_|\\__| |_||_||_|\\___|  \r\n                     |_|    |_|                                             ");
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

    struct USReader{
      volatile long pulseDelay = -1;
      volatile int validationCounter = 0;
    };
    USReader usFwLow = USReader();
    USReader usFwHigh = USReader();
    USReader usLeft = USReader();
    USReader usRight = USReader();
    USReader usDown = USReader();

    long pulseOut = 0;
    void sendPulse(int pin){
      digitalWrite(pin, LOW);
      digitalWrite(pin, HIGH);
      digitalWrite(pin, LOW);
    }

    void pulse(){
      pulseOut = micros();
      sendPulse(ULTRASONIC_OUT);
    }

    void calcPulseDelay(USReader *reader, long pulseIn){
          //error correction
          if(pulseIn - pulseOut > ULTRASONIC_VALIDATION_THRESHOLD){
            if(ULTRASONIC_VALIDATION_CYCLES > reader->validationCounter) { 
              reader->validationCounter++;
              return;
            }
            else {
              reader->validationCounter = 0;
            }
          }
          reader->pulseDelay = max(-1, pulseIn - pulseOut);
    }
    void usFwLowISR(){
      calcPulseDelay(&usFwLow, micros());
    }
    void usFwHighISR(){
      calcPulseDelay(&usFwHigh, micros());
    }
    void usLeftISR(){
      calcPulseDelay(&usLeft, micros());
    }
    void usRightISR(){
      calcPulseDelay(&usRight, micros());
    }
    void usDownISR(){
      calcPulseDelay(&usDown, micros());
    }

void registerUsISRs() {
  pinMode(ULTRASONIC_FORWARD_LOW_IN, INPUT);
  pinMode(ULTRASONIC_FORWARD_HIGH_IN, INPUT);
  pinMode(ULTRASONIC_LEFT_IN, INPUT);
  pinMode(ULTRASONIC_RIGHT_IN, INPUT);
  pinMode(ULTRASONIC_DOWN_IN, INPUT);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_FORWARD_LOW_IN), usFwLowISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_FORWARD_HIGH_IN), usFwHighISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_LEFT_IN), usLeftISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_RIGHT_IN), usRightISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ULTRASONIC_DOWN_IN), usDownISR, CHANGE);
}

    void initUltrasonicAsync(){
      registerUsISRs();
      LAS::scheduleRepeated(pulse, ULTRASONIC_DELAY, ENDLESS_LOOP);
      logger.printline("initialized Ultrasonic");
    }
    
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

  class VehicleRotation: public LAS::Callable {
    public:
      void run() override {
        motorsActive = true;
        if(alternate){
          leftMotor.step(stepSize * -1);
        } else {
          rightMotor.step(stepSize);
        }
        setRotationVar(currentVehicleRotation + (PI_MUL_PER_STEPSIZE/2));
          if(taskPtr->remainingRepeats == 1){
            motorsActive = false;
          }
        alternate = !alternate;
      }
      VehicleRotation(bool directionL): directionL(directionL){
        if(directionL){
          stepSize = MOTOR_STEPSIZE;
        } else {
          stepSize = MOTOR_STEPSIZE * -1;
        }
      };
    private:
      bool alternate = false;
      bool directionL = true;
      int stepSize;
  };

  void rotateVehicleByAsync(float pi_mul){
    if(checkMotorActivity()){
      return;
    }
    int steps = int(ROTATION_REVOLUTIONS * (pi_mul/2) * MOTOR_STEPS_PER_REVOLUTION);
    char buffer [BUFFER_SIZE] = "";
    snprintf(buffer, BUFFER_SIZE, "rotating vehicle by %d", steps);
    logger.printline(buffer, "info");
    bool l = true;
    if(steps<0){
      l = false;
    }
    LAS::scheduleRepeated(new VehicleRotation(l), ASAP, abs(int(steps / MOTOR_STEPSIZE) * 2));
  }

  void rotateVehicleToAsync(float pi_mul){
    rotateVehicleByAsync(pi_mul - currentVehicleRotation);
  }

  void driveSizeUnits(float units){
    driveStepsForward(int(units * VEHICLE_STEPS_X));
  }

  class: public LAS::Callable {
    void run() override {
      if(!Navigation::motorsActive) {
        char buffer[BUFFER_SIZE] = "";
        snprintf(buffer, sizeof(buffer), "UsFwLow: %d", Sensors::usFwLow.pulseDelay);
        logger.printline(buffer);
        if(Sensors::usFwLow.pulseDelay > 100 || Sensors::usFwLow.pulseDelay != -1) {
          driveStepsForward(100);
        }
      }
    }
  } driver;

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
          if(strcmp(serialByte, "\n") == 0){
            break;
          } 
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
        strcat(buffer, "ATEMPTING COMMAND \"");
        strcat(buffer, serialBuffer);
        strcat(buffer, "\""); 
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
        if(strcmp(serialBuffer, "UNBLOCKMOTOR") == 0){
          Navigation::motorsActive = false;
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
          logger.setConf(conf);          
          return true;
        }
        if(strcmp(serialBuffer, "TOGGLEWARNING") == 0){
          Logger::LogConfig conf = logger.getConf();
          conf.warning ^= true;
          logger.setConf(conf);
          return true;
        }
        if(strcmp(serialBuffer, "STEPPERTEST") == 0){
          Navigation::rotateVehicleByAsync(1);
          return true;
        }
        if(strcmp(serialBuffer, "PHILIPP") == 0){
          printPhilipp();
          LAS::scheduleRepeated(&Navigation::driver, ASAP, ENDLESS_LOOP, false);
          return true;
        }
        if(strcmp(serialBuffer, "RESET") == 0){
          logger.printline("USER RESET...", "severe");
          delay(10);
          void(* resetFunc) (void) = 0;
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

  LAS::scheduleRepeated(&serialConsole, ASAP, ENDLESS_LOOP, false);
  LAS::scheduleFunction(Navigation::initSteppers);
  LAS::scheduleFunction(Sensors::initColorSensorAsync);
  LAS::scheduleFunction(Sensors::initUltrasonicAsync);
  LAS::startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
  }
