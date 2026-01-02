#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

// this is a global for now
global_variable bool _running;
global_variable BITMAPINFO _bitmapInfo;
global_variable void *_bitmapMemory;
global_variable int _bitmapWidth;
global_variable int _bitmapHeight;
global_variable int _bytesPerPixel = 4;

internal void RenderWeirdGradient(int xOffset, int yOffset)
{
	int pitch = _bitmapWidth * _bytesPerPixel;
	uint8_t* row = (uint8_t*)_bitmapMemory;
	for (int y = 0; y < _bitmapHeight; ++y)
	{
		uint32_t* pixel = (uint32_t*)row;
		for (int x = 0; x < _bitmapWidth; ++x)
		{
			uint8_t blue = (x + xOffset);
			uint8_t green = (y + yOffset);
			// blue | green | red | pad
			*pixel++ = (green << 8) | blue;
		}

		row += pitch;
	}
}

internal void Win32ResizeDIBSection(int width, int height)
{	
	if (_bitmapMemory)
	{
		VirtualFree(_bitmapMemory, 0, MEM_RELEASE);
	}

	_bitmapWidth = width;
	_bitmapHeight = height;

	_bitmapInfo.bmiHeader.biSize = sizeof(_bitmapInfo.bmiHeader);
	_bitmapInfo.bmiHeader.biWidth = _bitmapWidth;
	_bitmapInfo.bmiHeader.biHeight = -_bitmapHeight; // making this negative to force StretchDIBits to go top down
	_bitmapInfo.bmiHeader.biPlanes = 1;
	_bitmapInfo.bmiHeader.biBitCount = 32;
	_bitmapInfo.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = (_bitmapWidth * _bitmapHeight) * _bytesPerPixel;
	_bitmapMemory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win32UpdateWindow(HDC deviceContext, RECT *clientRect, int x, int y, int width, int height)
{
	int windowWidth = clientRect->right - clientRect->left;
	int windowHeight = clientRect->bottom - clientRect->top;

	StretchDIBits(deviceContext,
		/*x, y, width, height, 
		x, y, width, height, */
		0, 0, _bitmapWidth, _bitmapHeight,
		0, 0, windowWidth, windowHeight,
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

			RECT clientRect;
			GetClientRect(window, &clientRect);
			Win32UpdateWindow(deviceContext, &clientRect, x, y, width, height);
			
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
		HWND window = CreateWindowEx(
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

		if (window)
		{
			_running = true;
			MSG message;
			int xOffset = 0;
			int yOffset = 0;
			while (_running)
			{				
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
					{
						_running = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				RenderWeirdGradient(xOffset, yOffset);

				HDC deviceContext = GetDC(window);
				RECT clientRect;
				GetClientRect(window, &clientRect);
				int windowWidth = clientRect.right - clientRect.left;
				int windowHeight = clientRect.bottom - clientRect.top;
				Win32UpdateWindow(deviceContext, &clientRect, 0, 0, windowWidth, windowHeight);
				ReleaseDC(window, deviceContext);

				++xOffset;
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
