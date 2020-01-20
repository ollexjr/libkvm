#ifndef H_UTIL_TIMER_H
#define H_UTIL_TIMER_H

#include <chrono>

class c_timer {
private:
	std::chrono::steady_clock::time_point _time_start;
public:
	void start();
	std::chrono::milliseconds elapsed_ms();
	c_timer();
};

#endif // !H_UTIL_TIMER_H
