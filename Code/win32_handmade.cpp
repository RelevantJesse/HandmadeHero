#include <windows.h>

LRESULT CALLBACK MainWindowCallback(
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
		case WM_DESTROY:
		{
			OutputDebugString("WM_DESTROY\n");
		} break;
		case WM_CLOSE:
		{
			OutputDebugString("WM_CLOSE\n");
		} break;
		case WM_ACTIVATEAPP:
		{
			OutputDebugString("WM_ACTIVATEAPP\n");
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			 HDC deviceContext = BeginPaint(window, &paint);
			 int x = paint.rcPaint.left;
			 int y = paint.rcPaint.top;
			 int width = paint.rcPaint.right - paint.rcPaint.left;
			 int height = paint.rcPaint.bottom - paint.rcPaint.top;
			 static DWORD operation = WHITENESS;

			 if (operation == WHITENESS)
			 {
				 operation = BLACKNESS;
			 }
			 else
			 {
				 operation = WHITENESS;
			 }

			 PatBlt(deviceContext, x, y, width, height, operation);
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
	windowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = MainWindowCallback;
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
			MSG message;
			for (;;)
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
