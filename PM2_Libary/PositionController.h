#ifndef PositionController_H_
#define PositionController_H_
#include <cstdlib>
#include <mbed.h>
#include "EncoderCounter.h"
#include "LowpassFilter.h"
#include "ThreadFlag.h"
#include "FastPWM.h"

class PositionController
{
public:

    PositionController(float counts_per_turn, float kn, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter);
    PositionController(float counts_per_turn, float kn, float kp, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter);
    PositionController(float counts_per_turn, float kn, float kp, float p, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter);

    virtual ~PositionController();

    float    getSpeedRPM();
    float    getSpeedRPS();
    float    getVoltage();
    void     setDesiredRotation(float desiredRotation);
    void     setDesiredRotation(float desiredRotation, float maxSpeedRPS);
    float    getRotation();
    void     setFeedForwardGain(float kn);
    void     setSpeedCntrlGain(float kp);
    void     setPositionCntrlGain(float p);

private:

    static const float TS;
    static const float LOWPASS_FILTER_FREQUENCY;
    static const float MIN_DUTY_CYCLE;
    static const float MAX_DUTY_CYCLE;

    float counts_per_turn;
    float kn;
    float kp;
    float max_voltage;
    float max_speed;
    float p;
    float voltage_out;

    FastPWM&           pwm;
    EncoderCounter&    encoderCounter;
    short              previousValueCounter;
    LowpassFilter      speedFilter;
    float              desiredSpeed;
    float              actualSpeed;
    float              initialRotation;
    float              actualRotation;
    float              desiredRotation;

    ThreadFlag         threadFlag;
    Thread             thread;
    Ticker             ticker;

    void               run();
    void               sendThreadFlag();
};

#endif /* PositionController_H_ */