#include "LedBlinker.h"

/////////////////////////////////////////////////////
//
LedBlinker::LedBlinker(int16_t ledPin)
{
    //ctor
    if ( ledPin >= 0)
        mLedPin = ledPin;
    else
        mLedPin = LED_BUILTIN;

    pinMode(mLedPin, OUTPUT);
    digitalWrite(mLedPin, LOW);
}

/////////////////////////////////////////////////////
//
void LedBlinker::blinkLedCB(void *vpThis)
{
   LedBlinker *pThis = (LedBlinker *)vpThis;
   digitalWrite(pThis->mLedPin, !digitalRead(pThis->mLedPin));     // set pin to the opposite state
}

/////////////////////////////////////////////////////
//
LedBlinker::~LedBlinker()
{
    //dtor
    stop();
}
