#include "thread.h"
#include "..\..\type.h"

#include <Windows.h>
#include <thread>
#include "util.h"

struct SWTpair {
	IThread * tc;
	fthread tptr;
	HANDLE signal;
};

DWORD __stdcall thread_wrapper(LPVOID param) {
	MSG msg;
	PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE); //force creation of thread messaging on OS
	
	SWTpair * p = (SWTpair*)(param);
	SetEvent(p->signal); //unlock creation thread
	int ret = (*(p->tptr))(p->tc); //execute thread
	
	//thread has returned 
	//delete execution object
	delete p->tc;
	delete p;

	printf("IThread: returned ret\n");
	return ret;
}

DWORD Thread_W64::_thread(LPVOID param)
{
	int ret = ((fthread)param)(this); //jump into generic thread type
	return ret;
}

void Thread_W64::_init_thread(fthread t)
{
	//thread execution details
	SWTpair*  p = new SWTpair();
	p->tc = this;
	p->tptr = t;
	p->signal = CreateEvent(NULL, FALSE, FALSE, L"THREAD_PROC_START");
	CreateThread(NULL, 0, thread_wrapper, p, 0, &this->threadId);
	WaitForSingleObject(p->signal, INFINITE); //INFINITE is possibly a bad idea lol

	printf("Thread_W64 -> thread_id %d\n", this->threadId);
	//Sleep(1);
}
bool Thread_W64::wait()
{
	/*
		https://docs.microsoft.com/en-us/windows/desktop/api/winuser/nf-winuser-getmessage
		If the function retrieves a message other than WM_QUIT, the return value is nonzero.
		If the function retrieves the WM_QUIT message, the return value is zero.
		If there is an error, the return value is -1. For example, the function fails if hWnd is an invalid window handle or lpMsg is an invalid pointer. 
		To get extended error information, call GetLastError.*/
	MSG msg;
	int ret = GetMessage(&msg, NULL, 0, 0);
	if (msg.message == WM_QUIT) {
		/* closing, ret = 0*/
		printf("w32 -> wm_quit\n");
		return false;
	}
	printf("tw32 -> %d\n", msg.message);
	if (ret == -1) {
		/* error */
		print_win32_error(GetLastError());
		return false;
	}
	//copy message
	this->msg = *(thread_message*)msg.lParam;
	//delete memory off heap
	delete (thread_message*)msg.lParam;

	return true;
}
int Thread_W64::peek()
{
	MSG msg;
	int ret = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
	//TranslateMessage()
	//DispatchMessage();

	//If no messages are available, the return value is zero.
	if (ret == 0){
		return 0; //return no message
	}

	if (msg.message == WM_QUIT) {
		/* thread close signaled */
		printf("Thread_W64 -> wm_quit\n");
		return -1; //return closing / fault
	}

	//copy message off heap, and delete pointer
	this->msg = *(thread_message*)msg.lParam;
	delete (thread_message*)msg.lParam;
	//if (ret == 0) {
	//	print_win32_error(GetLastError());
	//	return -1;
	//}

	//return message found
	return 1;
}
bool Thread_W64::send(int id, ptr data)
{
	/*
	If the function succeeds, the return value is nonzero.
	If the function fails, the return value is zero.*/
	thread_message * t = new thread_message;
	t->data = data;
	t->type = id;
	t->sender = this->uuid;
	int ret = PostThreadMessage(this->threadId, WM_USER+1, NULL, (LPARAM)t);
	if (ret == 0) {
		print_win32_error(GetLastError());
		return false;
	}
	return true;
}
void Thread_W64::send_close()
{
	int ret = PostThreadMessage(this->threadId, WM_QUIT, NULL, NULL);
	if (ret == 0) {
		print_win32_error(GetLastError());
		return;
	}
	return;
}
Thread_W64::Thread_W64(fthread t) : IThread(t){
	this->_init_thread(t);
}