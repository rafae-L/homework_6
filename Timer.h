#ifndef TRY_TO_GIT_TIMER_H
#define TRY_TO_GIT_TIMER_H

#include <chrono>
#include <iostream>

using namespace std::chrono;

template<typename T>
class Timer {
public:
    int Get(){
        steady_clock::time_point finish = steady_clock::now();

        return duration_cast<T>( finish - start).count();
    }

private:
    steady_clock::time_point start = steady_clock::now();
};

#endif //TRY_TO_GIT_TIMER_H
