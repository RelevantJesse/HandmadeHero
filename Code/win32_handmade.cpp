#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct win32_window_dimensions
{
	int Width;
	int Height;
};

// this is a global for now
global_variable bool _running;
global_variable win32_offscreen_buffer _globalBackBuffer;

internal win32_window_dimensions Win32GetWindowDimensions(HWND window)
{
	win32_window_dimensions result;

	RECT clientRect;
	GetClientRect(window, &clientRect);
	result.Width = clientRect.right - clientRect.left;
	result.Height = clientRect.bottom - clientRect.top;

	return result;
}

internal void RenderWeirdGradient(win32_offscreen_buffer buffer, int xOffset, int yOffset)
{
	uint8_t* row = (uint8_t*)buffer.Memory;
	for (int y = 0; y < buffer.Height; ++y)
	{
		uint32_t* pixel = (uint32_t*)row;
		for (int x = 0; x < buffer.Width; ++x)
		{
			uint8_t blue = (x + xOffset);
			uint8_t green = (y + yOffset);
			// blue | green | red | pad
			*pixel++ = (green << 8) | blue;
		}

		row += buffer.Pitch;
	}
}

internal void Win32ResizeDIBSection(win32_offscreen_buffer* buffer, int width, int height)
{	
	if (buffer->Memory)
	{
		VirtualFree(buffer->Memory, 0, MEM_RELEASE);
	}

	buffer->Width = width;
	buffer->Height = height;
	buffer->BytesPerPixel = 4;

	buffer->Info.bmiHeader.biSize = sizeof(buffer->Info.bmiHeader);
	buffer->Info.bmiHeader.biWidth = buffer->Width;
	buffer->Info.bmiHeader.biHeight = -buffer->Height; // making this negative to force StretchDIBits to go top down
	buffer->Info.bmiHeader.biPlanes = 1;
	buffer->Info.bmiHeader.biBitCount = 32;
	buffer->Info.bmiHeader.biCompression = BI_RGB;

	int bitmapMemorySize = (buffer->Width * buffer->Height) * buffer->BytesPerPixel;
	buffer->Memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	buffer->Pitch = width * buffer->BytesPerPixel;
}

internal void Win32DisplayBufferInWindow(HDC deviceContext, int windowWidth, int windowHeight,
	win32_offscreen_buffer buffer, 
	int x, int y, int width, int height)
{
	StretchDIBits(deviceContext,
		/*x, y, width, height, 
		x, y, width, height, */		
		0, 0, windowWidth, windowHeight,
		0, 0, buffer.Width, buffer.Height,
		buffer.Memory,
		&buffer.Info,
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

			win32_window_dimensions dimensions = Win32GetWindowDimensions(window);
			Win32DisplayBufferInWindow(deviceContext, dimensions.Width, dimensions.Height, _globalBackBuffer, x, y, width, height);
			
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

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE prevInstance,
	PWSTR cmdLine,
	int showCode)
{
	WNDCLASS windowClass = {};

	Win32ResizeDIBSection(&_globalBackBuffer, 1280, 720);

	windowClass.style = CS_HREDRAW | CS_VREDRAW;
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
			
			int xOffset = 0;
			int yOffset = 0;
			while (_running)
			{		
				MSG message;
				while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
				{
					if (message.message == WM_QUIT)
					{
						_running = false;
					}

					TranslateMessage(&message);
					DispatchMessage(&message);
				}

				RenderWeirdGradient(_globalBackBuffer, xOffset, yOffset);

				HDC deviceContext = GetDC(window);
				win32_window_dimensions dimensions = Win32GetWindowDimensions(window);
				Win32DisplayBufferInWindow(deviceContext, dimensions.Width, dimensions.Height, _globalBackBuffer, 0, 0, dimensions.Width, dimensions.Height);
				ReleaseDC(window, deviceContext);

				++xOffset;
				yOffset += 2;
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
