#include "MotorFunction.h"

#include <cstdio>
#include <string>

// create SpeedController and PositionController objects, default parametrization is for 78.125:1 gear box
float max_voltage = 12.0f;                  // define maximum voltage of battery packs, adjust this to 6.0f V if you only use one batterypack
float counts_per_turn = 20.0f * 78.0f;    // define counts per turn at gearbox end: counts/turn * gearratio
float kn = 78.0f / 12.0f;                  // define motor constant in rpm per V
float k_gear = 78.0f / 100.0f;            // define additional ratio in case you are using a dc motor with a different gear box, e.g. 100:1
float kp = 0.1f;                            // define custom kp, this is the default speed controller gain for gear box 78.125:1
float max_speed_rps = 0.5f;                 // define maximum speed that the position controller is changig the speed, has to be smaller or equal to kn * max_voltage
float pwm_period_s = 0.00005f;    // define pwm period time in seconds and create FastPWM objects to command dc motors

bool motorStoped;
Timer reference_filter;
Timer cycle_time;

int reference_step = 0;
float referencePos;
bool referenced = false;
float homePos;

int teach_step = 0;
Timer timeOut_teach;
float endPos;

const int move_main_direction = -1;

// Motor
PinName enable_dc_motors_pin = PB_15;
PinName pwm_M1_pin = PB_13;
PinName encoder_M1_pin1 = PA_6;
PinName encoder_M1_pin2 = PC_7;

// Reference Button
PinName reference_pin = PB_2;



// Motor initialisieren
InterruptIn reference(reference_pin);

DigitalOut enable_motors(enable_dc_motors_pin);    // create DigitalOut object to enable dc motors



FastPWM pwm_M1(pwm_M1_pin);             // motor is closed-loop position controlled (angle controlled)
EncoderCounter  encoder_M1(encoder_M1_pin1, encoder_M1_pin2); // create encoder objects to read in the encoder counter values
PositionController positionController_M1(counts_per_turn * k_gear, kn / k_gear, kp * k_gear, max_voltage, pwm_M1, encoder_M1); // parameters adjusted to 100:1 gear, we need a different speed controller gain here

void motorEnable(){
    // attach button fall and rise functions to reference button object
    reference.fall(&reference_pressed_fcn);
    reference.rise(&reference_released_fcn);
    reference_filter.start();

    //Motoren enablen
    enable_motors = 1;
    //Motoren initialisieren
    pwm_M1.period(pwm_period_s);
    pwm_M1.write(0.5f);
    positionController_M1.setDesiredRotation(positionController_M1.getRotation(),0);

    //Referenzierbutton initialisieren
    reference.mode(PullUp);

    
}

void debug(){
    float temp = positionController_M1.getRotation();
    printf("Geber: %3.33f ", temp);
    printf("Home: %3.33f ", homePos);
    printf("End: %3.33f ", endPos);
}



bool motorReference(){
    switch (reference_step) {
    case 0:
        if (reference.read() == 0){
            setRotateMotor(0.5, false);
            reference_step = 1;
        } else {
            setRotateMotor(0.1, true);
            reference_step = 2;
        }
        break;
    case 1:
        if (reference.read() == 1 and motorStoped){
            setRotateMotor(0.1, true);
            reference_step = 2;
        }
        break;
    case 2:
        if (motorStoped){
            referencePos = positionController_M1.getRotation();
            homePos = referencePos + 0.5;
            reference_step = 3;
        }
        break;
    case 3:
        referenced = true;
        break;
    default:
        setStopMotor();
        reference_step = 0;
    }
    return referenced;
}



bool motorTeach(bool button){
    if (referenced){
        int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(timeOut_teach.elapsed_time()).count();
        switch (teach_step) {
        case 0:
            if (button){
                timeOut_teach.start();
                timeOut_teach.reset();
                teach_step = 1;
            }
            break;
        case 1:
            if (button){
                setRotateMotor(1, true);
                timeOut_teach.reset();
            } else {
                setStopMotor();
            }
            
            if (time_ms > 2000) {
                teach_step = 2;
            }
            break;
        case 2:
            setStopMotor();
            endPos = positionController_M1.getRotation();
            teach_step = 3;
            break;
        case 3:
            break;
        default:
        teach_step = 0;
        }
    }
    return teach_step == 3;
}
bool motorMoveHome(){
    positionController_M1.setDesiredRotation(homePos, 1);
    return motorOnTarget(homePos);
}

int cycle_step = 0;

bool motorCycle(int time_in_ms, int time_out_ms){
    float distance = abs(homePos-endPos);
    switch (cycle_step) {
    case 0:
    cycle_time.start();
    cycle_time.reset();
    cycle_step = 1;
    break;
    case 1:
    positionController_M1.setDesiredRotation(endPos, distance/time_in_ms);
    if (motorOnTarget(endPos)) cycle_step = 2;
    break;
    case 2:
    positionController_M1.setDesiredRotation(homePos, distance/time_out_ms);
    if (motorOnTarget(homePos)) cycle_step = 1;
    break;
    default:
    cycle_step = 0;
    }
    return cycle_step != 0;
}

const float EPS = 1e-4;

bool motorOnTarget(float target){
    return abs(target - positionController_M1.getRotation())<EPS;
}


void setRotateMotor(float speed, bool direction, float step){
    float rotation;
    if (direction){
        rotation = positionController_M1.getRotation() + (step * move_main_direction);
    } else {
        rotation = positionController_M1.getRotation() + (step * move_main_direction * (-1));
    }
    positionController_M1.setDesiredRotation(rotation, speed);
    motorStoped = false;
}

void setStopMotor(){
    positionController_M1.setDesiredRotation(positionController_M1.getRotation(), 0);
    motorStoped = true;
}

void reference_pressed_fcn(){
    reference_filter.start();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(reference_filter.elapsed_time()).count();
    if (time_ms > 5) {
        reference_filter.reset();
        setStopMotor();
    }
}
void reference_released_fcn(){
    reference_filter.start();
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(reference_filter.elapsed_time()).count();
    if (time_ms > 5) {
        reference_filter.reset();
        setStopMotor();
    }
}

void resetMotorControll(){
    cycle_step = 0;
    teach_step = 0;
    reference_step = 0;
    referenced = false;
    setStopMotor();
}