# include "StateLed.h"
#include <string>

const float StateLed::TS = 0.001f;                       // period of 1 ms

StateLed::StateLed(PinName pin, string interval, char seperator) : DigitalOut(pin) {
    this->interval = StringIntervalSeperator(interval, seperator);
    
// set up thread
    thread.start(callback(this, &StateLed::run));
    ticker.attach(callback(this, &StateLed::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});

}

StateLed::~StateLed() {
    ticker.detach();
}

void StateLed::setInterval(string interval, char seperator){
    this->interval.setInterval(interval, seperator);
}

void StateLed::run(){
    while(true){
        // wait for the periodic signal
        ThisThread::flags_wait_any(threadFlag);

        // read timer
        int actual_task_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(task_timer.elapsed_time()).count();
        // Prüfen, ist die Zeit vergangen
        if (actual_task_time_ms >= task_time_ms) {
            // nächsten Befehl ermitteln
            actual_interval++;
            if (actual_interval > interval.getLength()) actual_interval = 1;
            int interval_time = stoi(interval.getPos(actual_interval));

            // Befehl ausführen, neue Zeit setzen
            if (interval_time > 0) write(1);
            else if (interval_time < 0) write(0);
            task_time_ms = abs(interval_time);
            task_timer.reset();
        }
    }
}

void StateLed::sendThreadFlag()
{
    thread.flags_set(threadFlag);
}