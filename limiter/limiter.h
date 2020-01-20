#ifndef H_CDD_LIMITER_H
#define H_CDD_LIMITER_H

#include <chrono>

/* 
	This is not threadsafe
	*/
class c_limit_bucket {
private:
//	int start_time;
	std::chrono::time_point<std::chrono::system_clock> start_time;
	int bucket_max = 1000;
	int bucket = 500;
public:
	bool set_size();
	bool check();
	c_limit_bucket();
};

#endif //H_CDD_LIMITER_H