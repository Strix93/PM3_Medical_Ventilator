#ifndef MBED_BUTTONFUNCTION_H
#define MBED_BUTTONFUNCTION_H

#include "mbed.h"
#include <string>
#include "StringIntervalSeperator.h"

class ButtonPressed : public InterruptIn
{
public:
    /**
    * @param pin - Pinnummer des Buttons
    */
    ButtonPressed(PinName pin, int shortTime = 100, int longTime = 1000, int timeOut = 5000);

    virtual ~ButtonPressed();

    /**
    *
    *
    * @param shortTime -  Maximale Zeit in [ms] wärend der Taster betätigt sein darf um eine kurze betätigung zu erkennen
    */
    void setShortTime(int shortTime);

    /**
    *
    *
    * @param longTime - Minimale Zeit in [ms] wärend der Taster betätigt sein muss
    */
    void setLongTime(int longTime);
    void setMaxChange(int maxChange);
    void setFiltersTime(int filtersTime_ms);

    void reset();

    bool getShortPressed(bool clear = false);
    bool getLongPressed(bool clear = false);

    /**
    *   Es wird überprüft ob die Reienfolge der Zustansänderung mit dem Interval überrein stimmt.
    *
    *   @param interval - Einen String der mit dem Trenzeichen getrente reienfolge der zustände definiert.
    *                     "*" steht für belibig
    *                     "minXX maxXX" wenn einen bereich definiert werden soll, den Wert 0 steht dabei als belibig 
    *                     "XX" bei nur einer Zahl wird der Wert mit der Tolleranz berücksichtigt.
    *   @param seperator - Das Trennzeichen zwischen den teilen
    *   @param tol - die Tolleranz falls einzelne werte vorgegeben werden
    *
    */
    bool checkInterval(string interval, char seperator, int tol = 0);

    private:

    Timer button_pressed_timer;
    Timer button_released_timer;

    struct intervalData{
        int timeOn;
        int timeOff;
        bool state;
    };

    struct intervalCheck{
        bool correct;
        int correctCounter;
    };

    //Timer function_timer;

    int button_elapsed_pressed_time_ms;
    int button_elapsed_released_time_ms;
    //bool button_state_changed;
    //int count;
    intervalData* interval = NULL;
    intervalCheck* checkData = NULL;
    int countInterval = -1;
    int writeInterval = -1;
    int counterMax;
    int shortTime;
    int longTime;
    int filtersTime_ms = 5;

    void button_pressed_fcn();
    void button_released_fcn();
    void button_dedect_fcn();
};

#endif