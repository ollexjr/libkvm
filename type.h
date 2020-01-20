#pragma once
#ifndef CDD_TYPES
#define CDD_TYPES
#include <thread>

#define dexp __declspec(dllimport)

typedef int handle;
typedef int freturn;
typedef void* ptr;
typedef void* function;
typedef void* generic_ptr;

//typedef unsigned char byte;

class IThread;
typedef freturn(*fthread)(IThread *);

struct ITexture;

struct IType {
public:
	IType();
	~IType();
};

struct thread_message {
	handle type;
	handle sender;
	ptr data;
};
struct thread_response {
	handle type;
	ptr data;
};

/* 
	Interface to a message based threading model
	Practical implimentation is subclassed inheriting this

	Current implimentations
		- Win64/Win32

	push message (from any contet) -> thread
	thread (thread context) -> read message

	On windows it simply abstracts the OS SendMessage etc threading model
	On linux, you will have to make it work like the interface defines, IDK, I'm writing this on windows
*/
class IThread {
protected:
	int nmessages;
	virtual void _init_thread(fthread t) {}; //create thread
	int uuid;
	thread_message msg;
public:
	enum status {
		GET_ERROR = -1,
		INIT = 0,
		SHUTTING_DOWN = 1,
		NO_MESSAGE = 0,
		GOT_MESSAGE = 1,
	};
	virtual bool send(int id, ptr data) = 0;
	virtual void send_close() = 0;

	//get result from peek, or wait
	thread_message message();

	//if wait returns false, the thread is closing
	virtual bool wait() = 0; //lock waiting for data
	
	//if peek returns false, the thread is closing
	virtual int peek() = 0; //peek waiting for data
	IThread(fthread f); //init with function we are threading
	~IThread();
};


IThread * create_thread(fthread t);
#endif 
