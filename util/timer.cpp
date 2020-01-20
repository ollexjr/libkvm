#include "timer.h"

void c_timer::start(){
	_time_start = std::chrono::steady_clock::now();
}

std::chrono::milliseconds c_timer::elapsed_ms(){
	return std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - this->_time_start);
}

c_timer::c_timer(){
	
	this->start();
}
