#include <LAS.h>
#include <LASConfig.h>

#include <Stepper.h>
using namespace LAS;

#define LEFT_MOTOR_PIN_0 8
#define LEFT_MOTOR_PIN_1 9
#define LEFT_MOTOR_PIN_2 10
#define LEFT_MOTOR_PIN_3 11


#define RIGHT_MOTOR_PIN_0 2
#define RIGHT_MOTOR_PIN_1 4
#define RIGHT_MOTOR_PIN_2 3
#define RIGHT_MOTOR_PIN_3 5

#define MOTOR_STEPS_PER_REVOLUTION 2048
#define MOTOR_STEPSIZE 5

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

  scheduleFunction(MotorControl::initSteppers, ASAP);
  schedulerInit();
}

void loop() {
  // put your main code here, to run repeatedly:
}
