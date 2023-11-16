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

namespace MotorControl {

Stepper leftMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, LEFT_MOTOR_PIN_0, LEFT_MOTOR_PIN_1, LEFT_MOTOR_PIN_2, LEFT_MOTOR_PIN_3);
Stepper rightMotor = Stepper(MOTOR_STEPS_PER_REVOLUTION, RIGHT_MOTOR_PIN_0, RIGHT_MOTOR_PIN_1, RIGHT_MOTOR_PIN_2, RIGHT_MOTOR_PIN_3);

void initSteppers() {
  leftMotor.setSpeed(10);
  rightMotor.setSpeed(10);
  LAS::scheduleRepeated([]() {
    leftMotor.step(MOTOR_STEPSIZE);
  },
                        ASAP, MOTOR_STEPS_PER_REVOLUTION / MOTOR_STEPSIZE);
  LAS::scheduleRepeated([]() {
    rightMotor.step(MOTOR_STEPSIZE);
  },
                        ASAP, MOTOR_STEPS_PER_REVOLUTION / MOTOR_STEPSIZE);
}
}

namespace Sensors {
  using namespace Shared;
  namespace ColorSensor {
    using namespace Shared;
    Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X); 
    float red, green, blue;

    void readColor(){
        tcs.getRGB(&red, &green, &blue);
    }
    
    void initColorSensor() {
        if (!tcs.begin()) {
          logger.printline("No TCS34725 found!", "severe");
          return;
        }
        LAS::scheduleRepeated(readColor, 50, ENDLESS_LOOP);
    }
  }
}

using namespace Shared;
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

  Serial.begin(9600);  logger.printline("Logger started");
  
  logger.printline("PhilippRaeuchle started");
 
  LAS::initScheduler(logger);
  LAS::scheduleFunction(MotorControl::initSteppers, ASAP);
  LAS::scheduleFunction(Sensors::ColorSensor::initColorSensor, ASAP);
  LAS::startScheduler();
}

void loop() {
  // put your main code here, to run repeatedly:
}
