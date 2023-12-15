#include "MotorFunction.h"

#include <cstdio>
#include <string>

// create SpeedController and PositionController objects, default parametrization is for 78.125:1 gear box
float max_voltage = 12.0f;                  // define maximum voltage of battery packs, adjust this to 6.0f V if you only use one batterypack
float counts_per_turn = 20.0f * 78.0f;    // define counts per turn at gearbox end: counts/turn * gearratio
float kn = 78.0f / 12.0f;                  // define motor constant in rpm per V
float k_gear = 78.0f / 100.0f;            // define additional ratio in case you are using a dc motor with a different gear box, e.g. 100:1
float kp = 0.1f;                            // define custom kp, this is the default speed controller gain for gear box 78.125:1
float max_speed_rps = 78.0f;                 // define maximum speed that the position controller is changig the speed, has to be smaller or equal to kn * max_voltage
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

int cycle_step = 0;
int cycle_count = 0;
int cycle_count_2 = 0;

const int move_main_direction = -1;
const float EPS = 1e-2;

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
    //positionController_M1.setDesiredRotation(positionController_M1.getRotation(),0);
    setStopMotor();

    //Referenzierbutton initialisieren
    reference.mode(PullUp);

    
}

void debug(){
    float temp = positionController_M1.getRotation();
    //printf("Geber: %3.3f ", temp);
    printf("Home: %3.3f ", homePos);
    printf("End: %3.3f ", endPos);
    printf("Cycle Count: %i ", cycle_count);
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
        if (motorStoped){
            if (reference.read() == 1){
                setRotateMotor(0.1, true);
                reference_step = 2;
            } else {
                reference_step = 2;
            }
        }
        break;
    case 2:
        if (motorStoped){
            referencePos = positionController_M1.getRotation();
            homePos = referencePos - 0.05;
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
    printf("reference step: %d ", reference_step);
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
                setRotateMotor(generateHigerMoment(0.5), true);
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
            resetHigerMoment();
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
    positionController_M1.setDesiredRotation(homePos, 0.5);
    cycle_time.stop();
    cycle_step = 0;
    return motorOnTarget(homePos);
}

bool motorCycle(int time_in_ms, int time_out_ms, int cycle_time_ms){
    float distance = abs(homePos-endPos);
    float speed_in = distance/(float)time_in_ms*1000;
    if (speed_in <= 0.09) speed_in = 0.09;
    float speed_out = distance/(float)time_out_ms*1000;
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(cycle_time.elapsed_time()).count();

    if (speed_out <= 0.09) speed_out = 0.09;
    switch (cycle_step) {
    case 0:
    cycle_time.start();
    cycle_time.reset();
    cycle_step = 1;
    break;
    case 1:
    cycle_count_2 = cycle_count + 1;
    positionController_M1.setDesiredRotation(endPos, generateHigerMoment(speed_in*1));
    //positionController_M1.setDesiredRotation(endPos, generateHigerMoment(max_speed_rps));
    if (motorOnTarget(endPos) or (time_ms > time_in_ms)) cycle_step = 2;
    //if (motorOnTarget(endPos)) cycle_step = 2;
    break;
    case 2:
    cycle_count = cycle_count_2;
    resetHigerMoment();
    if (time_ms >= time_in_ms) cycle_step = 3;
    break;
    case 3:
    positionController_M1.setDesiredRotation(homePos, generateHigerMoment(speed_out));
    if (motorOnTarget(homePos)) cycle_step = 4;
    break;
    case 4:
    resetHigerMoment();
    if (time_ms >= cycle_time_ms){
        cycle_time.reset();
        cycle_step = 1;
    }
    break;
    default:
    cycle_step = 0;
    }
    //printf("Zykle state %i ", cycle_step);
    //printf("speed_in %3.3f ", speed_in);
    //printf("speed_out %3.3f ", speed_out);
    printf("Motor Voltage %3.3f ", positionController_M1.getVoltage());
    return cycle_step != 0;
}

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

float add_speed_higer_moment = 0;
Timer time_higer_moment;
int step_higer_moment = 0;

float generateHigerMoment(float soll_speed){
    int time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_higer_moment.elapsed_time()).count();
    switch (step_higer_moment) {
    case 0:
    time_higer_moment.start();
    time_higer_moment.reset();
    add_speed_higer_moment = 0;
    step_higer_moment = 1;
    break;
    case 1:
    if (time_ms >= 100){
        time_higer_moment.reset();
        if (abs(abs(positionController_M1.getSpeedRPS()) - abs(soll_speed))>=1) add_speed_higer_moment += soll_speed*1;
    }
    }
    printf("hige moment state: %d speed: %3.3f ", step_higer_moment, soll_speed + add_speed_higer_moment);
    //printf("dif: %3.3f", (abs(abs(positionController_M1.getSpeedRPS()) - abs(soll_speed))));
    return soll_speed + add_speed_higer_moment;
}

void resetHigerMoment(){
    time_higer_moment.stop();
    step_higer_moment = 0;
}