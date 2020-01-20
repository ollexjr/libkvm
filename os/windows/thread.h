#ifndef CDD_WINDOWS_THREAD 
#define CDD_WINDOWS_THREAD
#include "..\..\type.h"
#include <Windows.h>
#include <thread>
class Thread_W64 : public IThread {
protected:
	HANDLE hThread;
	DWORD threadId = 0;

	DWORD _thread(LPVOID param);
	void _init_thread(fthread t) override;
protected:
	bool wait() override;
	int peek() override;
	bool send(int id, ptr data) override;
	void send_close() override;
public:
	Thread_W64(fthread t);
};
#endif