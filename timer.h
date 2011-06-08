#ifndef _TIMER_H_
#define _TIMER_H_

#include <SDL/SDL.h>

class Timer {
    private:
    int startTicks, pausedTicks;
    bool started, paused; 

    public:
    Timer();

    void start();
    void stop();
    void pause();
    void unpause();

    int get_ticks();

    bool is_started();
    bool is_paused();
};

#endif
