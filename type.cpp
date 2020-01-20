#include "type.h"
#include "os\windows\thread.h"
#include <thread>

IThread * create_thread(fthread t)
{
#ifdef __linux__ 
	//linux code goes here
#elif _WIN32
	return new Thread_W64(t);
#else
#pragma comment(failed)
#endif
}

thread_message IThread::message()
{
	return this->msg;
}

IThread::IThread(fthread t)
{
	this->_init_thread(t);
}

IThread::~IThread()
{
	printf("[IThread] destroying thread!");
}
