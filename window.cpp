#include "os\windows\win-window.h"
#ifdef __linux__ 
//linux code goes here
#elif _WIN32
#include "os\windows\win-window.h"
#else
#pragma comment(failed)
#endif

IWindowManager * _wmgr;
void destroy_window_manager(IWindowManager *)
{
	if (_wmgr != nullptr) {
		_wmgr->unload();
		delete _wmgr; //deleto
		_wmgr = nullptr;
	}
}
IWindowManager * get_window_manager()
{
	if (_wmgr != nullptr)
		return _wmgr;
#ifdef __linux__ 
	//linux code goes here
#elif _WIN32
	_wmgr = new WindowManager_W64();
	return _wmgr;
#else
#pragma comment(failed)
#endif
}
whandle IWindow::get_ref()
{
	return this->ref;
}

wret IWindow::OnEvent(window_event wm)
{
	switch (wm) {
	case window_event::CLOSING:
		break;
	}
	return 0;
}

handle g_ref_uuid = 0;
handle get_incr_uuid() {
	return g_ref_uuid++;
}

IWindow::IWindow(){
	//static std::atomic_int id;
	this->ref = get_incr_uuid();
}

ITexture::~ITexture()
{
	if (this->buffer != nullptr)
		delete[] this->buffer;
		//free(this->buffer);
}
