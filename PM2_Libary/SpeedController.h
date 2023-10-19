#ifndef SpeedController_H_
#define SpeedController_H_
#include <cstdlib>
#include <mbed.h>
#include "EncoderCounter.h"
#include "LowpassFilter.h"
#include "ThreadFlag.h"
#include "FastPWM.h"

class SpeedController
{
public:

    SpeedController(float counts_per_turn, float kn, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter);

    virtual ~SpeedController();

    void     setDesiredSpeedRPS(float desiredSpeed);
    float    getSpeedRPS();
    float    getSpeedRPM();
    void     setFeedForwardGain(float kn);
    void     setSpeedCntrlGain(float kp);

private:

    void     setDesiredSpeedRPM(float desiredSpeed);

    static const float TS;
    static const float LOWPASS_FILTER_FREQUENCY;
    static const float MIN_DUTY_CYCLE;
    static const float MAX_DUTY_CYCLE;

    float counts_per_turn;
    float kn;
    float kp;
    float max_voltage;

    FastPWM&           pwm;
    EncoderCounter&    encoderCounter;
    short              previousValueCounter;
    LowpassFilter      speedFilter;
    float              desiredSpeed;
    float              actualSpeed;
    // float              actualAngle;

    ThreadFlag         threadFlag;
    Thread             thread;
    Ticker             ticker;

    void               run();
    void               sendThreadFlag();
};

#endif /* SpeedController_H_ */