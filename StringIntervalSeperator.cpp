#include "StringIntervalSeperator.h"
#include <string>

StringIntervalSeperator::StringIntervalSeperator(string interval, char seperator){
setInterval(interval, seperator);
}

StringIntervalSeperator::~StringIntervalSeperator(){
    
}

void StringIntervalSeperator::setInterval(string interval, char seperator){
    this->interval = interval;
    this->seperator = seperator;
    defLength();
}

string StringIntervalSeperator::getPos(int pos){
    string w = "";
    int index = 1;
    if (1 < pos < length) return 0;
    for (auto x : interval) {
        if (x == seperator) {
            index++;
            w = "";
        } else {
            w = w + x;
        }
        if (index == pos) {
            break;
        }
    }
    return w;
}

int StringIntervalSeperator::getLength(){
    return length;
}

void StringIntervalSeperator::defLength(){
    int index = 0;
    for (auto x : interval) {
        if (x == seperator) index++;
    }
    length = index + 1;
}