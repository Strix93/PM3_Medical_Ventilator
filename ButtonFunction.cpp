#include "ButtonFunction.h"
#include <cstdlib>
#include <string>

ButtonPressed::ButtonPressed(PinName pin, int shortTime, int longTime, int timeOut) : InterruptIn(pin){
    // attach increment function of this Button instance
    rise(callback(this, &ButtonPressed::button_pressed_fcn));
    fall(callback(this, &ButtonPressed::button_released_fcn));

    setShortTime(shortTime);
    setLongTime(longTime);
    setMaxChange(20);

    if (read() == 0){
        button_released_timer.start();
        button_released_timer.reset();
    } else {
        button_pressed_timer.start();
        button_pressed_timer.reset();        
    }
}

ButtonPressed::~ButtonPressed(){
    free(interval);
    free(checkData);
}

void ButtonPressed::button_pressed_fcn() {
    //button_state_changed = true;

    button_pressed_timer.start();
    button_pressed_timer.reset();
    
    // read timer
    button_elapsed_released_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(button_released_timer.elapsed_time()).count();
    button_elapsed_pressed_time_ms = 0;
    button_released_timer.stop();

    // Auswerten
    button_dedect_fcn();
}

void ButtonPressed::button_released_fcn() {
    //button_state_changed = true;

    button_released_timer.start();
    button_released_timer.reset();
    
    // read timer
    button_elapsed_released_time_ms = 0;
    button_elapsed_pressed_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(button_pressed_timer.elapsed_time()).count();
    button_pressed_timer.stop();

    // Auswerten
    button_dedect_fcn();
}

void ButtonPressed::button_dedect_fcn() {
    if (button_elapsed_released_time_ms > filtersTime_ms or button_elapsed_pressed_time_ms > filtersTime_ms){
            countInterval++;
            writeInterval++;
        if (0 > countInterval >= counterMax) countInterval = 0;
        if (writeInterval >= counterMax) writeInterval = counterMax;

        if (interval!=NULL){
            interval[countInterval].state = read();
            interval[countInterval].timeOn = button_elapsed_pressed_time_ms;
            interval[countInterval].timeOff = button_elapsed_released_time_ms;
        }
    }
}

void ButtonPressed::setShortTime(int shortTime){
    this->shortTime = shortTime;
}

void ButtonPressed::setLongTime(int longTime){
    this->longTime = longTime;
}

void ButtonPressed::setMaxChange(int maxChange){
    this->counterMax = maxChange;

    intervalData* new_interval;
    new_interval = (intervalData*) realloc(interval, counterMax * sizeof(intervalData));

    if (new_interval!=NULL){
        interval = new_interval;
    }

    intervalCheck* new_checkData;
    new_checkData = (intervalCheck*) realloc(checkData, counterMax * sizeof(intervalCheck));

    if (new_checkData!=NULL){
        checkData = new_checkData;
    }
}

void ButtonPressed::setFiltersTime(int filtersTime_ms){
    this->filtersTime_ms = filtersTime_ms;
}

bool ButtonPressed::checkInterval(string checkInterval, char seperator, int tol){
    StringIntervalSeperator intervalCheck(checkInterval, seperator);
    // Schleife für jeden Teil im checkInterval
    int correctCounter = 0;
    for (int i = 1; i < intervalCheck.getLength(); i++){
        string step = intervalCheck.getPos(i);
        bool stateOn = !(step.find("-"));
        int time = 0;
        int minTime = 0;
        int maxTime = 0;
        if (step.find("*")) {
            time = 0;
            minTime = 0;
            maxTime = 0;
        } else if (step.find("min") or step.find("max")) {
            time = 0;
            minTime = abs(stoi(step.substr(step.find("min")+3, step.find("max"))));
            maxTime = abs(stoi(step.substr(step.find("max")+3, step.length())));
        } else {
            time = abs(stoi(step));
            minTime = time - tol;
            maxTime = time + tol;
        }
        // Schleife für jedes Element im aufgezeichnetem Interval
        // j begint beim ältesten Erreignis  und geht bis zum neusten Erreignis
        // k ist das umgerechnete j für den Intervalspeicher
        for (int j = 0; j < writeInterval; j++){
            int k = j + countInterval + 1; // Position im Intervalspeicher für Status
            if (k > counterMax) k -= counterMax;
            else if (0 > k) k = 0;
            int l = k + 1; // Position im Intervalspeicher für Zeit, da die verstrichene Zeit in jeweiligem Status erst bei der nächsten änderung registriert wird.
            if (l > counterMax) l -= counterMax;
            // Prüfung beginnt, beim ersten Teil werden alle Erreignise geprüft, danach nur noch diese bei denen das vorherige korrekt war
            if (this->interval[k].state == stateOn and (i == 1 or checkData[j-1].correct)){
                int checkTime;
                if (stateOn){
                    checkTime = this->interval[l].timeOn;
                    if (l == countInterval + 1) checkTime = std::chrono::duration_cast<std::chrono::milliseconds>(button_pressed_timer.elapsed_time()).count(); //Falls es die aktuelle Messung ist
                } else {
                    checkTime = this->interval[l].timeOff;
                    if (l == countInterval + 1) checkTime = std::chrono::duration_cast<std::chrono::milliseconds>(button_released_timer.elapsed_time()).count(); //Falls es die aktuelle Messung ist
                }
                if (maxTime != 0) checkData[j].correct = (minTime <= checkTime <= maxTime);
                else checkData[j].correct = (minTime <= checkTime);
                if (i == 1) checkData[j].correctCounter = checkData[j].correct;
                else if (checkData[j].correct) checkData[j].correctCounter++;
                if (correctCounter < checkData[j].correctCounter) correctCounter = checkData[j].correctCounter;
            }
        }
    }
    return (correctCounter == intervalCheck.getLength());
}

bool ButtonPressed::getShortPressed(bool clear){
    bool state = checkInterval(("-min" + to_string(longTime) + " max0, min0 max" + to_string(shortTime) + ", -min" + to_string(shortTime) + " max0"), ',');
    if (clear) reset();
    return state;
}

bool ButtonPressed::getLongPressed(bool clear){
    bool state = checkInterval(("-min" + to_string(longTime) + " max0, min" + to_string(longTime) + " max0"), ',');
    if (clear) reset();
    return state;
}

void ButtonPressed::reset(){
    countInterval = -1;
    writeInterval = -1;
}

