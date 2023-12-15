#include "PositionController.h"

const float PositionController::TS = 0.001f;                       // period of 1 ms
const float PositionController::LOWPASS_FILTER_FREQUENCY = 100.0f; // given in [rad/s]
const float PositionController::MIN_DUTY_CYCLE = 0.01f;            // minimum duty-cycle
const float PositionController::MAX_DUTY_CYCLE = 0.99f;            // maximum duty-cycle

PositionController::PositionController(float counts_per_turn, float kn, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter) : pwm(pwm), encoderCounter(encoderCounter), thread(osPriorityHigh, 4096)
{
    this->counts_per_turn = counts_per_turn;
    setFeedForwardGain(kn);
    setSpeedCntrlGain(0.1f);
    this->max_voltage = max_voltage;
    this->max_speed   = kn*max_voltage;
    setPositionCntrlGain(1300.0f);

    // initialise pwm
    pwm.period(0.00005); // pwm period of 50 us
    pwm.write(0.5);      // duty-cycle of 50%

    // initialise
    previousValueCounter = encoderCounter.read();
    speedFilter.setPeriod(TS);
    speedFilter.setFrequency(LOWPASS_FILTER_FREQUENCY);
    desiredSpeed = 0.0f;
    actualSpeed = 0.0f;
    float initialRotation = (float)encoderCounter.read()/counts_per_turn;
    this->initialRotation = initialRotation;
    actualRotation  = initialRotation;
    desiredRotation = initialRotation;

    // set up thread
    thread.start(callback(this, &PositionController::run));
    ticker.attach(callback(this, &PositionController::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});
}

PositionController::PositionController(float counts_per_turn, float kn, float kp, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter) : pwm(pwm), encoderCounter(encoderCounter), thread(osPriorityHigh, 4096)
{
    this->counts_per_turn = counts_per_turn;
    setFeedForwardGain(kn);
    setSpeedCntrlGain(kp);
    this->max_voltage = max_voltage;
    this->max_speed   = kn*max_voltage;
    setPositionCntrlGain(1300.0f);

    // initialise pwm
    pwm.period(0.00005); // pwm period of 50 us
    pwm.write(0.5);      // duty-cycle of 50%

    // initialise
    previousValueCounter = encoderCounter.read();
    speedFilter.setPeriod(TS);
    speedFilter.setFrequency(LOWPASS_FILTER_FREQUENCY);
    desiredSpeed = 0.0f;
    actualSpeed = 0.0f;
    float initialRotation = (float)encoderCounter.read()/counts_per_turn;
    this->initialRotation = initialRotation;
    actualRotation  = initialRotation;
    desiredRotation = initialRotation;

    // set up thread
    thread.start(callback(this, &PositionController::run));
    ticker.attach(callback(this, &PositionController::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});
}

PositionController::PositionController(float counts_per_turn, float kn, float kp, float p, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter) : pwm(pwm), encoderCounter(encoderCounter), thread(osPriorityHigh, 4096)
{
    this->counts_per_turn = counts_per_turn;
    setFeedForwardGain(kn);
    setSpeedCntrlGain(kp);
    this->max_voltage = max_voltage;
    this->max_speed   = kn*max_voltage;
    setPositionCntrlGain(p);

    // initialise pwm
    pwm.period(0.00005); // pwm period of 50 us
    pwm.write(0.5);      // duty-cycle of 50%

    // initialise
    previousValueCounter = encoderCounter.read();
    speedFilter.setPeriod(TS);
    speedFilter.setFrequency(LOWPASS_FILTER_FREQUENCY);
    desiredSpeed = 0.0f;
    actualSpeed = 0.0f;
    float initialRotation = (float)encoderCounter.read()/counts_per_turn;
    this->initialRotation = initialRotation;
    actualRotation  = initialRotation;
    desiredRotation = initialRotation;

    // set up thread
    thread.start(callback(this, &PositionController::run));
    ticker.attach(callback(this, &PositionController::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});
}

PositionController::~PositionController()
{
    ticker.detach();
}

/**
 * Reads the speed in RPM (rotations per minute).
 * @return actual speed in RPM.
 */
float PositionController::getSpeedRPM()
{
    return actualSpeed;
}

/**
 * Reads the speed in RPS (rotations per second).
 * @return actual speed in RPS.
 */
float PositionController::getSpeedRPS()
{
    return actualSpeed/60.0f;
}

/**
 * Sets desired rotation (1 corresponds to 360 deg).
 */
void PositionController::setDesiredRotation(float desiredRotation)
{
    this->desiredRotation = initialRotation + desiredRotation;
}

/**
 * Sets desired rotation (1 corresponds to 360 deg) and max. rotational speed in RPS (rotations per second).
 */
void PositionController::setDesiredRotation(float desiredRotation, float maxSpeedRPS)
{
    float maxSpeedRPM = fabs(maxSpeedRPS * 60.0f);
    if(maxSpeedRPM > kn * max_voltage) {
        maxSpeedRPM = kn * max_voltage;
    }
    this->max_speed = maxSpeedRPM;
    this->desiredRotation = initialRotation + desiredRotation;
}

/**
 * Reads the number of rotations (1 corresponds to 360 deg).
 * @return actual rotations.
 */
float PositionController::getRotation()
{
    return actualRotation - initialRotation;
}

float PositionController::getVoltage()
{
    return voltage_out;
}

/**
 * Sets the feed-forward gain.
 */
void PositionController::setFeedForwardGain(float kn)
{
    this->kn = kn;
}

/**
 * Sets the gain of the speed controller (p-controller).
 */
void PositionController::setSpeedCntrlGain(float kp)
{
    this->kp = kp;
}

/**
 * Sets the gain of the position controller (p-controller).
 */
void PositionController::setPositionCntrlGain(float p)
{
    this->p = p;
}

void PositionController::run()
{
    while(true) {
        // wait for the periodic signal
        ThisThread::flags_wait_any(threadFlag);

        // calculate actual speed of motors in [rpm]
        short valueCounter = encoderCounter.read();
        short countsInPastPeriod = valueCounter - previousValueCounter;
        previousValueCounter = valueCounter;
        actualSpeed = speedFilter.filter((float)countsInPastPeriod/counts_per_turn/TS*60.0f);
        actualRotation = actualRotation + actualSpeed/60.0f*TS;

        // calculate motor phase voltages
        desiredSpeed  = p*(desiredRotation - actualRotation);
        if (desiredSpeed < -max_speed) desiredSpeed = -max_speed;
        else if (desiredSpeed > max_speed) desiredSpeed = max_speed;
        float voltage = kp*(desiredSpeed - actualSpeed) + desiredSpeed/kn;
        voltage_out = voltage;
        // calculate, limit and set duty cycles
        float dutyCycle = 0.5f + 0.5f*voltage/max_voltage;
        if (dutyCycle < MIN_DUTY_CYCLE) dutyCycle = MIN_DUTY_CYCLE;
        else if (dutyCycle > MAX_DUTY_CYCLE) dutyCycle = MAX_DUTY_CYCLE;
        pwm.write(static_cast<double>(dutyCycle));
        //printf("Motot Volt: %3.3f, DutyCycle: %3.3f", voltage, dutyCycle);
    }
}

void PositionController::sendThreadFlag()
{
    thread.flags_set(threadFlag);
}