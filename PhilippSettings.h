#pragma once


#define CMD_CHAR_BUFFER_SIZE 16
#define SMALL_CHAR_BUFFER_SIZE 64


#define VEHICLE_STEPS_X 100
#define VEHICLE_STEPS_Y 100
#define SAFETY_DISTANCE_MUL 0.2

#define WHEEL_RADIUS 50
#define CENTER_RADIUS 100
#define ROTATION_REVOLUTIONS CENTER_RADIUS/WHEEL_RADIUS
#define PI_MUL_PER_STEPSIZE MOTOR_STEPSIZE/(ROTATION_REVOLUTIONS)


#define COLOR_SENSOR_PIN 10
#define ULTRASOUND_FORWARD_LOW_PIN 11
#define ULTRASOUND_FORWARD_HIGH_PIN 12d
#define ULTRASOUND_LEFT_PIN 13
#define ULTRASOUND_RIGHT_PIN 14
#define ULTRASOUND_DOWN_PIN 15


#define LEFT_MOTOR_PIN_0 6
#define LEFT_MOTOR_PIN_1 8
#define LEFT_MOTOR_PIN_2 7
#define LEFT_MOTOR_PIN_3 9

#define RIGHT_MOTOR_PIN_0 2
#define RIGHT_MOTOR_PIN_1 4
#define RIGHT_MOTOR_PIN_2 3
#define RIGHT_MOTOR_PIN_3 5

#define MOTOR_STEPS_PER_REVOLUTION 2048
#define MOTOR_STEPSIZE 5
