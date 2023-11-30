#ifndef MOTORFUNCTION_H
#define MOTORFUNCTION_H

#include "mbed.h"
#include "PM2_Libary.h"

void motorEnable();
bool motorReference();
bool motorTeach(bool button);
bool motorMoveHome();
bool motorCycle(int time_in_ms, int time_out_ms);

void setRotateMotor(float speed, bool direction, float step = 10);
void setStopMotor();

void reference_pressed_fcn();
void reference_released_fcn();

bool motorOnTarget(float target);
void resetMotorControll();

void debug();



    



#endif