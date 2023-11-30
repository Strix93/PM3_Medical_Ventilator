#ifndef STRINGINTERVALSEPERATOR_H
#define STRINGINTERVALSEPERATOR_H

#include "mbed.h"
#include <string>

class StringIntervalSeperator
{
    public:
        /**
        * 
        *
        * @param interval - Eine Zeichenkette die mit dem Trenzeichen in meherere teile geglidert ist
        * @param seperator - Trennzeichen zwischen teilen
        */
        StringIntervalSeperator(string interval = "100, -100", char seperator = ',');

        virtual ~StringIntervalSeperator();
        
        /**
        * Anpassung des Interval
        *
        * @param interval - Eine Zeichenkette die mit dem Trenzeichen in meherere teile geglidert ist
        * @param seperator - Trennzeichen zwischen teilen
        */
        void        setInterval(string interval = "100, -100", char seperator = ',');

        /**
        *   Gibt den Teil von der gewünschten stelle zurück
        *
        *   @param pos - Die stelle von der den Wert zurückgegeben werten soll
        */
        string         getPos(int pos);
        /**
        *   Gibt die anzahl Teile des Intervals zurück
        */
        int         getLength();

    private:
        string      interval;
        char        seperator;
        int         length;

        void        defLength();
};

#endif