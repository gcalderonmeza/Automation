#ifndef _SIMPLE_TIMER_H
#define _SIMPLE_TIMER_H

class SimpleTimer {
  public:
    SimpleTimer(int durationInMillisecs);

    void Reset(unsigned long now);
    bool IsTriggered(unsigned long now);

  private:
    unsigned long duration;
    unsigned long triggerTime;
};

#endif
