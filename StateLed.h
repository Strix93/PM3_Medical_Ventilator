#ifndef MBED_STATELED_H
#define MBED_STATELED_H

#include "mbed.h"
#include <string>
#include "ThreadFlag.h"
#include "StringIntervalSeperator.h"

class StateLed : public DigitalOut
{
public:
    /**
    * Der Ausgang schaltet im vorgegebenen Intervall
    *
    * @param pin - PWM pin to connect to
    * @param interval - Zeiten in ms, positiv = ein, negativ = aus
    * @param seperator - Trennzeichen zwischen den Zeiten im Interval
    */
    StateLed(PinName pin, string interval = "100, -100", char seperator = ',');

    virtual ~StateLed();

    /**
    * Anpassung des Interval
    *
    * @param interval - Zeiten in ms, positiv = ein, negativ = aus
    * @param seperator - Trennzeichen zwischen den Zeiten im Interval
    */
    void setInterval(string interval = "100, -100", char seperator = ',');

private:

    static const float TS;

    StringIntervalSeperator interval;

    int                task_time_ms;
    int                actual_interval;

    ThreadFlag         threadFlag;
    Thread             thread;
    Ticker             ticker;
    Timer              task_timer; 

    void               run();
    void               sendThreadFlag();

};


#endif