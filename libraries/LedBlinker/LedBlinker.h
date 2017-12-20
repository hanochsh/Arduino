#ifndef LEDBLINKER_H
#define LEDBLINKER_H

#include "Arduino.h"
#include <Ticker.h>

/////////////////////////////////////////////////////
// Class LedBlinker
/////////////////////////////////////////////////////
class LedBlinker:public Ticker
{
    private:
        static void blinkLedCB(void *pVThis);

    public:
        LedBlinker(int16_t ledPin);
        ~LedBlinker();
        void start(float seconds)
        {
            digitalWrite(mLedPin, LOW);
            attach(seconds, blinkLedCB, (void *)this);
        }
        void stop()
        {
            digitalWrite(mLedPin, LOW);
            detach();
        }

 //   private:
        uint8_t mLedPin;
};

#endif // LEDBLINKER_H
