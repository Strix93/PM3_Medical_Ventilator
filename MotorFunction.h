#ifndef MOTORFUNCTION_H
#define MOTORFUNCTION_H

#include "mbed.h"
#include "PM2_Libary.h"

// Initialise and enable motor
void motorEnable();
// Motor referencing
bool motorReference();
// Teach required position
bool motorTeach(bool button);
// Move to home position
bool motorMoveHome();
// Execute cycle from home position to demand position and back again
bool motorCycle(int time_in_ms, int time_out_ms, int cycle_time_ms);
// Rotate motor continuously
void setRotateMotor(float speed, bool direction, float step = 10);
// Stop motor
void setStopMotor();
// Reference button pressed
void reference_pressed_fcn();
// Reference button released
void reference_released_fcn();
// Check if motor is in position
bool motorOnTarget(float target);
// Reset learnt motor configurations
void resetMotorControll();
// Motor debug information
void debug();
// Calculate a higher speed if the required speed is not reached (higher torque)
float generateHigerMoment(float soll_speed);
// Reset the parameters for the higher speed (higher torque)
void resetHigerMoment();


    



#endif