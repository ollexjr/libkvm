#ifndef CCD_WINDOW_WINDOW
#define CCD_WINDOW_WINDOW
#include "..\..\window.h"
#include <Windows.h>


struct InternalWindowManager;
class Window_W64 : public IWindowOwned {
private:
	HWND hWindow;
public:
	//void isAlive();
	void close();
	bool is_visible();
	void minmise();
	void maximise();
	int get_height();
	int get_width();
	int get_x();
	int get_y();
	void set_visiblity(bool);

	const std::string get_name();

	void set_name();
	void setsize(int, int);
	void setpos(int, int);
	bool write_buffer(ITexture * tex);
	HWND handle();
	
	Window_W64(HWND internalHandle);
	~Window_W64();
};

class WindowManager_W64 : public IWindowManager {
private:
	InternalWindowManager * m_wmdata;
public:
	IWindowOwned * create(int x, int y, int width, int height);
	IWindowListener * create_listener();
	void destroy_window(IWindowOwned *);
	void destroy_window_by_ref(whandle ref);

	void set_system_cb(system_ef f);
	void set_window_cb(window_ef f);
	void run();
	void unload();
	IWindowListener & peek_focused();

	int poll_processes(int);
	std::vector<IProcessInfo>* get_processes_list();
	IWindowListener * get_process_main_window(whandle ref);

	WindowManager_W64();
	~WindowManager_W64();
};

class WindowListener_AW64 : public IWindowListener {
private:
	HWND hWindow;
public:
	void _get_size();
	bool _is_valid();

	const std::string get_name();
	bool is_visible();
	int get_height();
	int get_width();
	int get_x();
	int get_y();
	
	bool set_foreground();

	//int debug_focus();

	ITexture * get_texture();

	WindowListener_AW64(HWND window);
};

#endif //CCD_WINDOW_WINDOW