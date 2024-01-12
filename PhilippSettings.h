#pragma once

//comms
#define BUFFER_SIZE 64

#define BAUDRATE 115200


#define WHEEL_RADIUS 50
#define CENTER_RADIUS 100
#define REVOLUTION_LENGTH 2*WHEEL_RADIUS*3.14
#define ROTATION_REVOLUTIONS CENTER_RADIUS/WHEEL_RADIUS
#define PI_MUL_PER_STEPSIZE MOTOR_STEPSIZE/(ROTATION_REVOLUTIONS)

//drive settings
#define VEHICLE_X 100
#define VEHICLE_Y 50
#define VEHICLE_STEPS_X VEHICLE_X/REVOLUTION_LENGTH
#define VEHICLE_STEPS_Y VEHICLE_Y/REVOLUTION_LENGTH
#define SAFETY_DISTANCE 5


#define ULTRASONIC_VALIDATION_THRESHOLD 500
#define ULTRASONIC_VALIDATION_CYCLES 8


//pins
#define COLOR_SENSOR_PIN 12
#define ULTRASONIC_FORWARD_LOW_IN 2
#define ULTRASONIC_FORWARD_HIGH_IN 3
#define ULTRASONIC_LEFT_IN 18
#define ULTRASONIC_RIGHT_IN 19
#define ULTRASONIC_DOWN_IN 20
#define ULTRASONIC_OUT 13
#define ULTRASONIC_DELAY 100

#define TOF_XSHUT_FW_LOW 2
#define TOF_XSHUT_FW_HIGH 3
#define TOF_XSHUT_LEFT 18
#define TOF_XSHUT_RIGHT 19
#define TOF_XSHUT_DOWN 20

#define LEFT_MOTOR_PIN_0 8
#define LEFT_MOTOR_PIN_1 10
#define LEFT_MOTOR_PIN_2 9
#define LEFT_MOTOR_PIN_3 11

#define RIGHT_MOTOR_PIN_0 4
#define RIGHT_MOTOR_PIN_1 6
#define RIGHT_MOTOR_PIN_2 5
#define RIGHT_MOTOR_PIN_3 7


//stepper settings
#define MOTOR_STEPS_PER_REVOLUTION 2048
#define MOTOR_STEPSIZE 5
