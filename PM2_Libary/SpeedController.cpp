#include "SpeedController.h"

const float SpeedController::TS = 0.001f;                       // period of 1 ms
const float SpeedController::LOWPASS_FILTER_FREQUENCY = 100.0f; // given in [rad/s]
const float SpeedController::MIN_DUTY_CYCLE = 0.01f;            // minimum duty-cycle
const float SpeedController::MAX_DUTY_CYCLE = 0.99f;            // maximum duty-cycle

SpeedController::SpeedController(float counts_per_turn, float kn, float max_voltage, FastPWM& pwm, EncoderCounter& encoderCounter) : pwm(pwm), encoderCounter(encoderCounter), thread(osPriorityHigh, 4096)
{
    this->counts_per_turn = counts_per_turn;
    setFeedForwardGain(kn);
    setSpeedCntrlGain(0.1f);
    this->max_voltage = max_voltage;

    // initialise pwm
    pwm.period(0.00005); // pwm period of 50 us
    pwm.write(0.5);      // duty-cycle of 50%

    // initialise
    previousValueCounter = encoderCounter.read();
    speedFilter.setPeriod(TS);
    speedFilter.setFrequency(LOWPASS_FILTER_FREQUENCY);
    desiredSpeed = 0.0f;
    actualSpeed = 0.0f;
    // actualAngle = 0.0f;

    // set up thread
    thread.start(callback(this, &SpeedController::run));
    ticker.attach(callback(this, &SpeedController::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});
}

SpeedController::~SpeedController()
{
    ticker.detach();
}

/**
 * Sets the desired speed in RPM (rotations per minute).
 */
void SpeedController::setDesiredSpeedRPM(float desiredSpeed)
{
    this->desiredSpeed = desiredSpeed;
}

/**
 * Reads the speed in RPM (rotations per minute).
 * @return actual speed in RPM.
 */
float SpeedController::getSpeedRPM()
{
    return actualSpeed;
}

/**
 * Sets the desired speed in RPS (rotations per second).
 */
void SpeedController::setDesiredSpeedRPS(float desiredSpeed)
{
    this->desiredSpeed = desiredSpeed*60.0f;
}

/**
 * Reads the speed in RPS (rotations per second).
 * @return actual speed in RPS.
 */
float SpeedController::getSpeedRPS()
{
    return actualSpeed/60.0f;
}

/**
 * Sets the feed-forward gain.
 */
void SpeedController::setFeedForwardGain(float kn)
{
    this->kn = kn;
}

/**
 * Sets the gain of the speed controller (p-controller).
 */
void SpeedController::setSpeedCntrlGain(float kp)
{
    this->kp = kp;
}

void SpeedController::run()
{
    while(true) {
        // wait for the periodic signal
        ThisThread::flags_wait_any(threadFlag);

        // calculate actual speed of motors in [rpm]
        short valueCounter = encoderCounter.read();
        short countsInPastPeriod = valueCounter - previousValueCounter;
        previousValueCounter = valueCounter;
        actualSpeed = speedFilter.filter((float)countsInPastPeriod/counts_per_turn/TS*60.0f);
        // actualAngle = actualAngle + actualSpeed/60.0f*TS;

        // calculate motor phase voltages
        float voltage = kp*(desiredSpeed - actualSpeed) + desiredSpeed/kn;

        // calculate, limit and set duty cycles
        float dutyCycle = 0.5f + 0.5f*voltage/max_voltage;
        if (dutyCycle < MIN_DUTY_CYCLE) dutyCycle = MIN_DUTY_CYCLE;
        else if (dutyCycle > MAX_DUTY_CYCLE) dutyCycle = MAX_DUTY_CYCLE;
        pwm.write(static_cast<double>(dutyCycle));
    }
}

void SpeedController::sendThreadFlag()
{
    thread.flags_set(threadFlag);
}