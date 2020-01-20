#pragma once

#include "type.h"
typedef handle whandle;
typedef freturn wret;
#include <vector>


/* operating system defined in /os/XXXX/window.h */
struct IWindowData;


/* The format must be specified in the type*/

struct ITexture {
	enum format: int {
		RGB_16 = 2,
		RGB_24 = 3,
		RGB_32 = 4,
	};
	int format;
	int source;
	int x, y;
	int width, height;
	unsigned char * buffer = nullptr;
	//alloc();
	//dealloc();

	~ITexture();
};
/* The most basic implimentation of a system window, 
	allows the programmer to create read and write window details*/
class IWindow {
protected:
	//friend IWindowManager;
	std::string name;
	int x, y;
	int width, height;
	whandle ref = 0;
public:
	enum window_event {
		CLOSING = 0,
		CREATING = 1,
		MINIMISING = 2,
		MAXIMISING = 3,
		
		CLICKED = 4,
		CLICK_DOWN = 5,
		CLICK_UP = 6,
		
		KEYDOWN = 7,
		KEYUP = 8,
	};
	struct key_info {
		int key;
		bool repeat;
		int scan; //special keys
	};
	whandle get_ref();

	virtual const std::string get_name() = 0;
	virtual int get_height() = 0;
	virtual int get_width() = 0;
	virtual int get_x() = 0;
	virtual int get_y() = 0;
	virtual bool is_visible() = 0;

	/*
	virtual wret OnResize(int, int) = 0;
	virtual wret OnKey(bool, int) = 0;
	virtual wret OnMouse(int, int) = 0;
	virtual wret OnMouseMove() = 0;
	*/
	virtual wret OnEvent(window_event);
	IWindow();
};
class IWindowOwned : public IWindow {
public:
	virtual void close() = 0;
	virtual bool write_buffer(ITexture * tex) = 0;
	virtual void minmise() = 0;
	virtual void maximise() = 0;

	virtual void set_visiblity(bool) = 0;
	virtual void set_name() = 0;
	virtual void setsize(int, int) = 0;
	virtual void setpos(int, int) = 0;
};
typedef freturn(*window_ef)(IWindow::window_event ev, IWindowOwned * w, generic_ptr ptr);


/* IWindowListener
	Listens to window events from unowned windows, 
	and allows you to:
		read window information
		read window texture
*/
class IWindowListener : public IWindow{
private:
	handle window;
public:
	virtual bool refresh();
	virtual bool set_foreground() = 0;
	virtual ITexture * get_texture() = 0;
};

/**
	window_ef generic_ptr, pointer to structure data concerning the event, do not delete!
*/


struct IInternal;
struct IProcessInfo {
	unsigned long process_id;
	std::string name;
	std::vector<IWindowListener*> windows;
	IWindowListener* root;
	//IWindowListener info;
};

enum system_event {
	WINDOW_FOREGROUND = 0,
	VALID_PROCESS_STARTED = 1,
	VALID_PROCESS_WINDOW_CREATED = 2,
	VALID_PROCESS_WINDOW_DESTROYING = 3,
};
//should be IWindowManager::system_event
typedef freturn(*system_ef)(system_event ev, generic_ptr data);
class IWindowManager {
protected:
	std::vector<IWindowOwned*> windows;
	std::vector<IWindowListener*> listeners;
	//window_ef m_cb_user_wnd;
public:
	virtual void run() = 0;
	//virtual void run_loop() = 0;
	virtual IWindowListener* create_listener() = 0;
	virtual IWindowOwned* create(int x, int y, int width, int height) = 0;
	virtual IWindowListener & peek_focused() = 0;
	virtual void destroy_window(IWindowOwned *) = 0;
	virtual void destroy_window_by_ref(whandle ref) = 0;
	virtual void set_window_cb(window_ef f) = 0;
	virtual void set_system_cb(system_ef f) = 0;
	virtual int poll_processes(int) = 0;
	virtual std::vector<IProcessInfo>* get_processes_list() = 0;
	virtual IWindowListener* get_process_main_window(whandle ref) = 0;
	virtual void unload() = 0;
};


void destroy_window_manager(IWindowManager *);
IWindowManager * get_window_manager();

