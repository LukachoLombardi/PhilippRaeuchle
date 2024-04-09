#pragma once

//comms
#define BUFFER_SIZE 64

#define BAUDRATE 115200

// still gotta define these
#define WHEEL_RADIUS 25 // actually like 35mm
#define CENTER_RADIUS 110
#define REVOLUTION_LENGTH int(2*WHEEL_RADIUS*3.14)
#define ROTATION_REVOLUTIONS CENTER_RADIUS/WHEEL_RADIUS
#define ROT_MUL_PER_STEPSIZE MOTOR_STEPSIZE/(ROTATION_REVOLUTIONS*MOTOR_STEPS_PER_REVOLUTION)

// drive settings
#define VEHICLE_X 250
#define VEHICLE_Y 200
#define VEHICLE_STEPS_X VEHICLE_X/REVOLUTION_LENGTH*MOTOR_STEPS_PER_REVOLUTION
#define VEHICLE_STEPS_Y VEHICLE_Y/REVOLUTION_LENGTH*MOTOR_STEPS_PER_REVOLUTION
#define SAFETY_DISTANCE 250
#define SETBACK_MUL 1
#define LANE_CHANGE_MUL 0.5
#define UNITY_DISTANCE 10
#define MAX_TABLE_DISTANCE 500

//pins
#define COLOR_SENSOR_PIN 15

#define TOF_XSHUT_FW_LOW 22
#define TOF_XSHUT_FW_HIGH 26
#define TOF_XSHUT_LEFT 23
#define TOF_XSHUT_RIGHT 24
#define TOF_XSHUT_DOWN 25

#define MOTOR_FEEDBACK_PIN 10
#define STEPPER_BAUDRATE 9600

//stepper settings
#define MOTOR_STEPS_PER_REVOLUTION 200

//legacy settings
#define MOTOR_STEPSIZE 1
