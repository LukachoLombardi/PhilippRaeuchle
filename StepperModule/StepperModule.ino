#include <AccelStepper.h>
#include <MultiStepper.h>

AccelStepper stepperLeft = AccelStepper(AccelStepper::FULL4WIRE, 2, 3, 4, 5);
AccelStepper stepperRight = AccelStepper(AccelStepper::FULL4WIRE, 6, 7, 8, 9);

int dataIn = 0;
int dataIn2 = 0;

constexpr int FEEDBACK_PIN = 10;

constexpr float STEPPER_MAX_SPEED = 150;
constexpr float STEPPER_ACCEL = 10;
constexpr int STEP_AMOUNT = 200;

constexpr int BAUDRATE = 115200;

void setup() {
  pinMode(FEEDBACK_PIN, OUTPUT);
  pinMode(TOGGLE_PIN, INPUT_PULLUP);

  stepperLeft.setMaxSpeed(STEPPER_MAX_SPEED);
  stepperRight.setMaxSpeed(STEPPER_MAX_SPEED);

  stepperLeft.setAcceleration(STEPPER_ACCEL);
  stepperRight.setAcceleration(STEPPER_ACCEL);

  Serial.begin(115200);
}

void serialRead() {
  if (Serial.available() >= 0) {
    dataIn1 = Serial.parseInt();
    dataIn2 = Serial.parseInt();
    if(dataIn == 0) {
      stepperLeft.stop();
      return;
    }
    if(dataIn2 == 0) {
      stepperRight.stop();
      return;
    }
    stepperLeft.move(dataIn);
    stepperRight.move(dataIn2);
  }
}

void loop() {
  serialRead();

  if(stepperLeft.isRunning() || stepperRight.isRunning()) {
    digitalWrite(FEEDBACK_PIN, HIGH);
  } else {
    digitalWrite(FEEDBACK_PIN, LOW);
  }

  stepperLeft.run();
  stepperRight.run();
}
