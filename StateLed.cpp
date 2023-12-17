# include "StateLed.h"
//#include <cstdio>
//#include <string>

const float StateLed::TS = 0.001f;                       // period of 1 ms

// "stoi" does not work reliably enough, so this is currently replaced by an array

/*StateLed::StateLed(PinName pin, string interval, char seperator) : DigitalOut(pin) {
    this->interval = StringIntervalSeperator(interval, seperator);
    printf("LED load");
// set up thread
    thread.start(callback(this, &StateLed::run));
    ticker.attach(callback(this, &StateLed::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});

    task_timer.start();
}*/

StateLed::StateLed(PinName pin, int interval_mode) : DigitalOut(pin){
    setInterval(interval_mode);

// set up thread
    thread.start(callback(this, &StateLed::run));
    ticker.attach(callback(this, &StateLed::sendThreadFlag), std::chrono::microseconds{static_cast<long int>(1.0e6f * TS)});

    task_timer.start();
}

StateLed::~StateLed() {
    ticker.detach();
}

/*void StateLed::setInterval(string interval, char seperator){
    this->interval.setInterval(interval, seperator);
}*/

void StateLed::setInterval(int interval_mode){
    switch (interval_mode) {
    case 0:
    interval2[0] = 100;
    interval2[1] = -100;
    interval2_length = 2;
    break;
    case 1:
    interval2[0] = 300;
    interval2[1] = -300;
    interval2_length = 2;
    break;
    case 2:
    interval2[0] = 500;
    interval2[1] = -500;
    interval2_length = 2;
    break;
    case 3:
    interval2[0] = 100;
    interval2[1] = -100;
    interval2[2] = 100;
    interval2[3] = -500;
    interval2_length = 4;
    break;
    case 4:
    interval2[0] = 100;
    interval2[1] = -100;
    interval2[2] = 500;
    interval2[3] = -100;
    interval2_length = 4;
    break;
    case 11:
    interval2[0] = 100;
    interval2[1] = -900;
    interval2_length = 2;
    break;
    case 12:
    interval2[0] = 100;
    interval2[1] = -100;
    interval2[2] = 100;
    interval2[3] = -700;
    interval2_length = 4;
    break;
    case 13:
    interval2[0] = 100;
    interval2[1] = -100;
    interval2[2] = 100;
    interval2[3] = -100;
    interval2[4] = 100;
    interval2[5] = -500;
    interval2_length = 6;
    break;
    case 14:
    interval2[0] = 100;
    interval2[1] = -100;
    interval2[2] = 100;
    interval2[3] = -100;
    interval2[4] = 100;
    interval2[5] = -100;
    interval2[6] = 100;
    interval2[7] = -300;
    interval2_length = 8;
    break;
    }
}

// "stoi" does not work reliably enough, so this is currently replaced by an array
void StateLed::run(){
    while(true){
        // wait for the periodic signal
        ThisThread::flags_wait_any(threadFlag);
        //printf("LED check");
        // read timer
        int actual_task_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(task_timer.elapsed_time()).count();
        // Prüfen, ist die Zeit vergangen
        if (actual_task_time_ms >= task_time_ms) {
            // nächsten Befehl ermitteln
            actual_interval++;
            if (actual_interval > interval2_length-1) actual_interval = 0;
            int interval_time = interval2[actual_interval];
            //if (actual_interval > interval.getLength()) actual_interval = 1;
            //try {
            //    int interval_time = stoi(interval.getPos(actual_interval));
            //    
            //}
            //catch (int e) {
            //    int interval_time = 0;
            //}
            //string test = interval.getPos(actual_interval);
            //printf("LED Intervall: %s", test.c_str());
            printf("LED Intervall: %d ", interval_time);
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