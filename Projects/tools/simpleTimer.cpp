// Simple Timer implementation
#include "SimpleTimer.h"

SimpleTimer::SimpleTimer(int durationInMillisecs)
{
  // Default duration in case of error is 1 sec
  this->duration = durationInMillisecs > 0 ? durationInMillisecs : 1000;
}

void SimpleTimer::Reset(unsigned long now)
{
  this->triggerTime = now + this->duration;
}

bool SimpleTimer::IsTriggered(unsigned long now)
{
  return this->triggerTime < now;
}

