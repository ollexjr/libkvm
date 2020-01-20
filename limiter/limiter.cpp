#include "limiter.h"
#include <chrono>

bool c_limit_bucket::check(){
	auto timediff =
		std::chrono::duration_cast<std::chrono::milliseconds>(
			start_time - std::chrono::system_clock::now()
		).count();

	int drain = bucket - ((timediff / 1000) * 2) / 100;
	bucket = (drain < 0) ? 0 : drain; //clamp
	
	this->start_time = std::chrono::system_clock::now();
	if (drain + 10 > bucket_max){
		bucket = bucket_max;
		return false;
	}
	else {
		bucket += 10;
		return true;
	}
}

c_limit_bucket::c_limit_bucket(){
	this->start_time = std::chrono::system_clock::now();
}
