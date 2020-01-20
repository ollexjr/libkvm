#include "win-window.h"
#include "util.h"
#include <Windows.h>
#include <tchar.h>
#include <psapi.h>


//enum processaes
#pragma (comment, "Psapi.lib")

struct IWindowData {
	HANDLE ref;
};
struct IProcessInternal {
	IProcessInfo a;
	HWND hWindow;
};

struct IProcessP : IProcessInfo {
	HWND window_handle;
	unsigned long process_id;
	void get_name();
};

struct InternalWindowManager {
	HINSTANCE hInstance;
	HANDLE hEvent;
	HWND hFocused;
	window_ef cb_user_wnd;
	system_ef cb_sys;
	HHOOK hShellHook;

	std::vector<IProcessInfo> processes;

	static LRESULT __stdcall
		_windowsproc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void
		_winEventProcCallback(
			HWINEVENTHOOK hWinEventHook,
			DWORD dwEvent,
			HWND hwnd,
			LONG idObject, LONG idChild,
			DWORD dwEventThread, DWORD dwmsEventTime);
	static LRESULT
		_shellProcCallback(int nCode, WPARAM wParam, LPARAM lParam);
} g_wmdata;

struct s_wm_cb_pair {
	WindowManager_W64 * wm;
	Window_W64 * client;
};


LRESULT InternalWindowManager::_windowsproc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	//printf("[WindowManager_W64] -> event\n");
	s_wm_cb_pair * pair = (s_wm_cb_pair*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	switch (uMsg)
	{
	case WM_CREATE:
		//created window pointer
		//GetWindowLongPtr(hWnd, -21);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)lParam);
		pair = (s_wm_cb_pair*)lParam;
		printf("[WindowManager_W64] -> WM_CREATE\n");
		if (g_wmdata.cb_user_wnd != nullptr)
			g_wmdata.cb_user_wnd(IWindow::window_event::CREATING, pair->client, nullptr);
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		break;
	case WM_PAINT:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		break;
	case WM_NCCREATE:
		printf("[WindowManager_W64] -> WM_NCCREATE\n");
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_CLOSE:
		if (g_wmdata.cb_user_wnd != nullptr)
			g_wmdata.cb_user_wnd(IWindow::window_event::CLOSING, pair->client, nullptr);
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		printf("[WindowManager_W64] -> WM_DESTROY\n");
		PostQuitMessage(0);
		break;
	case WM_QUIT:
		printf("[WindowManager_W64] -> WM_QUIT\n");
		break;
	default:
		return DefWindowProcW(hWnd, uMsg, wParam, lParam);
		break;
	}
	return (LRESULT)0;
}
void InternalWindowManager::_winEventProcCallback(
	HWINEVENTHOOK hWinEventHook,
	DWORD dwEvent,
	HWND hWnd,
	LONG idObject,
	LONG idChild,
	DWORD dwEventThread,
	DWORD dwmsEventTime)
{
	switch (dwEvent) {
	case EVENT_SYSTEM_FOREGROUND:
		g_wmdata.hFocused = hWnd;
		printf("[WindowManager_AW64] -> EVENT_SYSTEM_FOREGROUND\n");
		g_wmdata.cb_sys(system_event::WINDOW_FOREGROUND, &WindowListener_AW64(hWnd));
		//get_window_manager()->peek_focused().get_texture(); /* copy texturero */
		break;
	case EVENT_SYSTEM_MINIMIZESTART:
		printf("[WindowManager_AW64] -> EVENT_SYSTEM_MINIMIZESTART\n");
		break;
	case EVENT_SYSTEM_MINIMIZEEND:
		printf("[WindowManager_AW64] -> EVENT_SYSTEM_MINIMIZEEND\n");
		break;
	case EVENT_SYSTEM_MOVESIZESTART:
		printf("[WindowManager_AW64] -> EVENT_SYSTEM_MOVESIZESTART\n");
		break;
	case EVENT_SYSTEM_MOVESIZEEND:
		printf("[WindowManager_AW64] -> EVENT_SYSTEM_MOVESIZEEND\n");
		break;
	}
}

LRESULT InternalWindowManager::_shellProcCallback(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0) {
		return CallNextHookEx(NULL, nCode, wParam, lParam);
	}
	switch (nCode) {
	case HSHELL_WINDOWCREATED:
		printf("InternalWindowManager -> HSHELL_WINDOWCREATED\n");
		break;
	case HSHELL_WINDOWACTIVATED:
		printf("InternalWindowManager -> HSHELL_WINDOWACTIVATED\n");
		break;
	case HSHELL_WINDOWDESTROYED:
		printf("InternalWindowManager -> HSHELL_WINDOWDESTROYED\n");
		break;
	case HSHELL_WINDOWREPLACED:
		printf("InternalWindowManager -> HSHELL_WINDOWREPLACED\n");
		break;
	}
}

IWindowOwned * WindowManager_W64::create(int x, int y, int width, int height)
{
	s_wm_cb_pair * pair = new s_wm_cb_pair;
	pair->client = nullptr;
	pair->wm = this;

	HWND hwnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW | WS_EX_CLIENTEDGE,
		L"_ccd_managedwindow",
		L"window",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		//WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		NULL,
		NULL,
		this->m_wmdata->hInstance, pair);
	if (hwnd == NULL) {
		printf("[WindowManager_W64] Failed to create window\n");
		print_win32_error(GetLastError());
		delete pair;
		return nullptr;
	}

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	Window_W64 * window = new Window_W64(hwnd);
	pair->client = window;
	this->windows.push_back(window);
	return window;
}

IWindowListener * WindowManager_W64::create_listener()
{

	//this->listeners.push_back();
	return nullptr;
}

void WindowManager_W64::destroy_window(IWindowOwned * w)
{
	//CloseWindow(w->handle());
}

void WindowManager_W64::destroy_window_by_ref(whandle ref)
{

}

void WindowManager_W64::set_system_cb(system_ef f)
{
	this->m_wmdata->cb_sys = f;
}

void WindowManager_W64::set_window_cb(window_ef f)
{
	this->m_wmdata->cb_user_wnd = f;
}

void WindowManager_W64::run()
{
	MSG msg;
	while (true) {
		int ret = GetMessage(&msg, NULL, 0, 0);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (ret <= 0) return;
	}
}

void WindowManager_W64::unload()
{
	for (int i = 0; i < this->windows.size(); ++i) {
		IWindowOwned * wptr = this->windows[i];
		//wptr->OnEvent(wptr->CLOSING);
		wptr->close();

		//this->destroy_window(wptr);
	}
}

IWindowListener & WindowManager_W64::peek_focused()
{
	return WindowListener_AW64(this->m_wmdata->hFocused);
}
BOOL is_main_window(HWND handle) {
	return GetWindow(handle, GW_OWNER) == (HWND)0 && IsWindowVisible(handle);
}

void PrintProcessNameAndID(DWORD processID) {
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
	// Get a handle to the process.
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
		PROCESS_VM_READ,
		FALSE, processID);
	// Get the process name.
	if (NULL != hProcess) {
		HMODULE hMod;
		DWORD cbNeeded;
		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
			&cbNeeded)) {
			GetModuleBaseName(hProcess, hMod, szProcessName,
				sizeof(szProcessName) / sizeof(TCHAR));
		}
	}
	// Print the process name and identifier.
	_tprintf(TEXT("%s  (PID: %u)\n"), szProcessName, processID);
	// Release the handle to the process.
	CloseHandle(hProcess);
}
BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam) {
	IProcessInfo& data = *(IProcessInfo*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	if (data.process_id != process_id)
		return TRUE;

	if (IsWindowVisible(handle)) {
		WCHAR buffer[1024];
		GetWindowTextW(handle, (LPWSTR)&buffer, 1024);
		//IWindowListener * t = new WindowListener_AW64(handle);
		data.windows.push_back(new WindowListener_AW64(handle));
		_tprintf(TEXT("\tWindow: %s\n"), buffer);
	}

	if (data.root == nullptr && is_main_window(handle)) {
		data.root = new WindowListener_AW64(handle);
	}
	return TRUE;
}

int WindowManager_W64::poll_processes(int) {
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	unsigned int i;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		return 1;
	}
	cProcesses = cbNeeded / sizeof(DWORD);
	// Print the name and process identifier for each process.
	for (i = 0; i < cProcesses; i++) {
		if (aProcesses[i] != 0) {

			IProcessInfo pi;
			pi.root = nullptr;

			pi.process_id = aProcesses[i];
			//PrintProcessNameAndID(pi.process_id);
			//filter out all programs without a valid window

			printf("Program:\n");
			if (!EnumWindows(enum_windows_callback, (LPARAM)&pi))
				continue;

			printf("\tWindows: %d\n", pi.windows.size());

			if (pi.root != nullptr) {
				g_wmdata.processes.push_back(pi);
			}
			//IProcessInternal p;
			//IProcessInfo p;
			//p.process_id = aProcesses[i];
			//g_wmdata.processes = pi;
		}
	}
	return 0;
}

std::vector<IProcessInfo> * WindowManager_W64::get_processes_list() {
	return &g_wmdata.processes;
}

IWindowListener* WindowManager_W64::get_process_main_window(whandle ref) {
	IProcessInternal p;
	p.a = g_wmdata.processes[ref];
	if (EnumWindows(enum_windows_callback, (LPARAM)&p))
		return nullptr;
	return new WindowListener_AW64(p.hWindow);
}

WindowManager_W64::WindowManager_W64(){
	//use a pointer becuse reasons
	this->m_wmdata = &g_wmdata;

	this->m_wmdata->hShellHook = SetWindowsHookEx(
		WH_SHELL,
		this->m_wmdata->_shellProcCallback,
		this->m_wmdata->hInstance,
		NULL);
	if (this->m_wmdata == NULL) {
		print_win32_error(GetLastError());
	}

	this->m_wmdata->hEvent = SetWinEventHook(
		EVENT_SYSTEM_FOREGROUND,
		EVENT_SYSTEM_SCROLLINGSTART, NULL,
		this->m_wmdata->_winEventProcCallback, 0, 0,
		WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

	/* construct windows window class*/
	this->m_wmdata->hInstance = GetModuleHandle(NULL);
	WNDCLASSEXW wd = { 0 };
	//memset(&wd, 0, sizeof(WNDCLASSEXW));

	wd.cbClsExtra = 0;
	wd.cbSize = sizeof(WNDCLASSEXW);
	wd.hInstance = this->m_wmdata->hInstance;
	wd.lpfnWndProc = this->m_wmdata->_windowsproc;
	wd.style = CS_HREDRAW | CS_VREDRAW;
	wd.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);

	wd.hIcon = NULL; //LoadIcon(NULL, IDI_APPLICATION); //default icon
	wd.hCursor = NULL;// LoadCursor(NULL, IDC_ARROW);   //default arrow mouse curso
	wd.lpszClassName = L"_ccd_managedwindow";

	if (!RegisterClassEx(&wd)) {
		/* error */
		printf("[WindowManager_W64] Failed to register window class\n");
		print_win32_error(GetLastError());
	}
	printf("[WindowManager_W64] Ok\n");
}

WindowManager_W64::~WindowManager_W64() {
	UnregisterClassW(L"_ccd_managedwindow", this->m_wmdata->hInstance);
	UnhookWinEvent((HWINEVENTHOOK)this->m_wmdata->hEvent);
	UnhookWindowsHookEx(this->m_wmdata->hShellHook);
}

HWND Window_W64::handle()
{
	return this->hWindow;
}

void Window_W64::set_visiblity(bool v)
{
	ShowWindow(this->hWindow, (v ? SW_SHOW : SW_HIDE));
}

const std::string Window_W64::get_name()
{
	return this->name;
}

void Window_W64::set_name()
{
}

void Window_W64::setsize(int width, int height)
{
	this->width = width;
	this->height = height;

	SetWindowPos(this->hWindow, 
		NULL, NULL, NULL, 
		this->width, this->height, 
		SWP_NOMOVE | SWP_NOREDRAW | SWP_NOZORDER);
	//MoveWindow(this->handle, -1, -1, this->width, this->height, NULL)
}

void Window_W64::setpos(int, int)
{
}

bool Window_W64::write_buffer(ITexture * tex)
{
	/*PAINTSTRUCT     ps;
	HDC             hdc;
	BITMAP          bitmap;
	HDC             hdcMem;
	HGDIOBJ         oldBitmap;
	hdc = BeginPaint(this->hWindow, &ps);
	hdcMem = CreateCompatibleDC(hdc);
	oldBitmap = SelectObject(hdcMem, hSourceBitmap);
	GetObject(hSourceBitmap, sizeof(bitmap), &bitmap);
	BitBlt(hdc, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, oldBitmap);
	DeleteDC(hdcMem);
	EndPaint(hWnd, &ps);*/


	// Creating temp bitmap
	HBITMAP map = CreateBitmap(
		tex->width,
		tex->height,
		1,
		8 * tex->format, //24
		tex->buffer);

	HDC hdc = GetDC(this->hWindow);
	HDC src = CreateCompatibleDC(hdc);

	// hdc - Device context for window, I've got earlier with GetDC(hWnd) or GetDC(NULL);
	SelectObject(src, map);
	// Inserting picture into our temp HDC
	// Copy image from temp HDC to window

	if (!BitBlt(hdc, // Destination
		0,  // x and
		0,  // y - upper-left corner of place, where we'd like to copy
		tex->width, // width of the region
		tex->height, // height
		src, // source
		0,   // x and
		0,   // y of upper left corner  of part of the source, from where we'd like to copy
		SRCCOPY)) {
		// Defined DWORD to juct copy pixels. Watch more on msdn;
		printf("failed to blit\n");
	}
	DeleteDC(src); // Deleting temp HDC
	return false;
}

Window_W64::Window_W64(HWND window)
{
	this->hWindow = window;
}

void Window_W64::close()
{
	CloseWindow(this->hWindow);
}

bool Window_W64::is_visible()
{
	return false;
}

void Window_W64::minmise()
{
}

void Window_W64::maximise()
{
}

int Window_W64::get_height()
{
	return 0;
}

int Window_W64::get_width()
{
	return 0;
}

int Window_W64::get_x()
{
	return 0;
}

int Window_W64::get_y()
{
	return 0;
}

void WindowListener_AW64::_get_size()
{
	RECT rect;
	if (GetWindowRect(this->hWindow, &rect))
	{
		this->width = rect.right - rect.left;
		this->height =  rect.bottom - rect.top;
		this->x = rect.left;
		this->y = rect.top;
	}
}

bool WindowListener_AW64::_is_valid()
{
	return IsWindowVisible(this->hWindow);
}

const std::string WindowListener_AW64::get_name()
{
	//WCHAR buffer[1024];
	//GetWindowTextW(this->hWindow, (LPWSTR)&buffer, 1024);
	CHAR buffer[1024];
	GetWindowTextA(this->hWindow, (LPSTR)&buffer, 1024);
	return std::string(buffer);
	//std::string("__undefined_please_impliment__");
}

bool WindowListener_AW64::is_visible()
{
	return false;
}

int WindowListener_AW64::get_height()
{
	_get_size();
	return this->height;
}

int WindowListener_AW64::get_width()
{
	_get_size();
	return this->width;
}

int WindowListener_AW64::get_x()
{
	_get_size();
	return this->x;
}

int WindowListener_AW64::get_y()
{
	_get_size();
	return this->y;
}

bool WindowListener_AW64::set_foreground()
{
	return SetForegroundWindow(this->hWindow);
}

ITexture * WindowListener_AW64::get_texture() {
	if (!this->_is_valid()) return nullptr;

	_get_size();

	//HDC hdc = GetDC(NULL); // get the desktop device context
	HDC hdcSource = GetDC(this->hWindow); // get the desktop device context
	HDC hDest = CreateCompatibleDC(hdcSource);
	// create a device context to use yourself


	// get the height and width of the screen
	//int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	//int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);

	
	int bwidth = ITexture::format::RGB_32;
	int bits = 8 * bwidth;

	unsigned char* lpBitmapBits;
	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof(BITMAPINFO));
	bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = -height;  //negative so (0,0) is at top left
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = bits;

	// create a bitmap
	HBITMAP hBitmap = CreateDIBSection(hdcSource, &bi, DIB_RGB_COLORS, (void**)&lpBitmapBits, NULL, NULL);
	//HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, width, height);

	// use the previously created device context with the bitmap
	SelectObject(hDest, hBitmap);

	// copy from the desktop device context to the bitmap device context
	// call this once per 'frame'
	BitBlt(hDest, 0, 0, width, height, hdcSource, 0, 0, SRCCOPY);

	//GetObject(hDest,)
	BITMAP bitmap = { 0 };
	GetObject(hBitmap, sizeof(bitmap), (LPVOID)&bitmap);
	//printf("%dw %dh\n", bitmap.bmWidth, bitmap.bmHeight);

	ITexture * tex = new ITexture;
	tex->height = bitmap.bmHeight;
	tex->width = bitmap.bmWidth;
	//tex->buffer = (byte*)_aligned_malloc((height * width * bwidth) + 32, 32);
	tex->buffer = new byte[(height * width * bwidth)];

	tex->x = 0;
	tex->y = 0;
	tex->format = bwidth;
	
	memcpy(tex->buffer, lpBitmapBits, (height * width * bwidth));

	//tex.buffer = new char[w * h * bitmap.bmBits];
	// after the recording is done, release the desktop context you got..
	ReleaseDC(NULL, hdcSource);

	// ..delete the bitmap you were using to capture frames..
	DeleteObject(hBitmap);

	// ..and delete the context you created
	DeleteDC(hDest);
	return tex;
}



WindowListener_AW64::WindowListener_AW64(HWND window)
{
	this->hWindow = window;
}
