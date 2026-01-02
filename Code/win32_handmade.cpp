#include <windows.h>

#define internal static
#define local_persist static
#define global_variable static

// this is a global for now
global_variable bool _running;
global_variable BITMAPINFO _bitmapInfo;
global_variable void *_bitmapMemory;
global_variable HBITMAP _bitmapHandle;
global_variable HDC _bitmapDeviceContext;

internal void Win32ResizeDIBSection(int width, int height)
{
	// Maybe change this to try to create first then free
	if (_bitmapHandle)
	{
		DeleteObject(_bitmapHandle);
	}
	
	if (!_bitmapDeviceContext)
	{
		_bitmapDeviceContext = CreateCompatibleDC(0);
	}
	
	_bitmapInfo.bmiHeader.biSize = sizeof(_bitmapInfo.bmiHeader);
	_bitmapInfo.bmiHeader.biWidth = width;
	_bitmapInfo.bmiHeader.biHeight = height;
	_bitmapInfo.bmiHeader.biPlanes = 1;
	_bitmapInfo.bmiHeader.biBitCount = 32;
	_bitmapInfo.bmiHeader.biCompression = BI_RGB;

	_bitmapDeviceContext = CreateCompatibleDC(0);
	_bitmapHandle = CreateDIBSection(
		_bitmapDeviceContext,
		&_bitmapInfo,
		DIB_RGB_COLORS,
		&_bitmapMemory,
		0, 0);
}

internal void Win32UpdateWindow(HDC deviceContext, int x, int y, int width, int height)
{
	StretchDIBits(deviceContext,
		x, y, width, height, 
		x, y, width, height, 
		_bitmapMemory, 
		&_bitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(
	HWND   window,
	UINT   message,
	WPARAM wParam,
	LPARAM lParam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_SIZE:
		{
			RECT clientRect;
			GetClientRect(window, &clientRect);
			int width = clientRect.right - clientRect.left;
			int height = clientRect.bottom - clientRect.top;

			Win32ResizeDIBSection(width, height);
			OutputDebugString("WM_SIZE\n");
		} break;

		case WM_CLOSE:
		{
			_running = false;
			OutputDebugString("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;

		case WM_DESTROY:
		{
			_running = false;
			OutputDebugString("WM_DESTROY\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC deviceContext = BeginPaint(window, &paint);
			
			int x = paint.rcPaint.left;
			int y = paint.rcPaint.top;
			int width = paint.rcPaint.right - paint.rcPaint.left;
			int height = paint.rcPaint.bottom - paint.rcPaint.top;
			Win32UpdateWindow(deviceContext, x, y, width, height);
			
			EndPaint(window, &paint);
		} break;

		default:
		{
			//OutputDebugStringA("default\n");
			result = DefWindowProc(window, message, wParam, lParam);
		} break;
	}

	return result;
}


int WINAPI WinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	PSTR cmdLine,
	int showCode)
{
	WNDCLASS windowClass = {};
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = Win32MainWindowCallback;
	windowClass.hInstance = instance;
	//windowClass.hIcon;
	windowClass.lpszClassName = "HandmadeHeroWindowClass";

	if (RegisterClass(&windowClass))
	{
		HWND windowHandle = CreateWindowEx(
			0,
			windowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW | WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			instance,
			0);

		if (windowHandle)
		{
			_running = true;
			MSG message;
			while (_running)
			{
				BOOL messageResult = GetMessage(&message, 0, 0, 0);

				if (messageResult > 0)
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				}
				else
				{
					break;
				}
			}
			
		}
		else
		{
			// Logging
		}
	}
	else
	{
		//Logging
	}

	return 0;
}
