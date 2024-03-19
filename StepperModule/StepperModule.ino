#include <AccelStepper.h>
#include <MultiStepper.h>

AccelStepper stepperLeft = AccelStepper(AccelStepper::FULL4WIRE, 2, 3, 4, 5);
AccelStepper stepperRight = AccelStepper(AccelStepper::FULL4WIRE, 6, 7, 8, 9);

int dataIn1 = 0;
int dataIn2 = 0;

constexpr int FEEDBACK_PIN = 10;

constexpr float STEPPER_MAX_SPEED = 150;
constexpr float STEPPER_ACCEL = 50;
constexpr float STEPPER_STOP_DECCEL = 250;
constexpr int STEP_AMOUNT = 200;

constexpr int BAUDRATE = 9600;

void setup() {
  pinMode(FEEDBACK_PIN, OUTPUT);

  stepperLeft.setMaxSpeed(STEPPER_MAX_SPEED);
  stepperRight.setMaxSpeed(STEPPER_MAX_SPEED);


  Serial.begin(BAUDRATE);
  Serial.println("Serial init");
}

void serialRead() {
  if (Serial.available() > 0) {
    Serial.println("receiving");
    Serial.println(Serial.peek());
    dataIn1 = Serial.parseInt();
    dataIn2 = Serial.parseInt();
    Serial.println(dataIn1); Serial.println(dataIn2);
    if(dataIn1 == 0) {
      stepperLeft.setAcceleration(STEPPER_STOP_DECCEL);
      stepperLeft.stop();
    }else {
      stepperLeft.setAcceleration(STEPPER_ACCEL);
      stepperLeft.move(dataIn1);
    }
    if(dataIn2 == 0) {
      stepperRight.setAcceleration(STEPPER_STOP_DECCEL);
      stepperRight.stop();
    } else {
      stepperRight.setAcceleration(STEPPER_ACCEL);
      stepperRight.move(dataIn2);
    }
    dataIn1 = -1;
    dataIn2 = -1;
    Serial.flush();
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
