#include <Windows.h>

#define internal static
#define local_persist static
#define global_variable static

// TODO: This is a global for now.
global_variable bool Running;

internal void ResizeDIBSection(int Width, int Height)
{

}

LRESULT CALLBACK MainWindowCallback(
//		WNDPROC PrevWndFunc,
	HWND Window,
	UINT Message,
	WPARAM WParam,
	LPARAM LParam)
{
	LRESULT Result = 0;

	switch (Message)
	{
		case WM_SIZE:
		{
			RECT ClientRect;
			GetClientRect(Window, &ClientRect);
			LONG Width = ClientRect.right - ClientRect.left;
			LONG Height = ClientRect.bottom - ClientRect.top;
			ResizeDIBSection(Width, Height);
			OutputDebugStringA("WM_SIZE\n");
		} break;

		case WM_DESTROY:
		{
			// TODO: Handle this with an error - recreate window?
			Running = false;
		} break;

		case WM_CLOSE:
		{
			// TODO: Handle this with a message to the user?
			Running = false;
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			int X = Paint.rcPaint.left;
			int Y = Paint.rcPaint.top;
			LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
			LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
			local_persist DWORD Operation = WHITENESS;
			PatBlt(DeviceContext, X, Y, Height, Width, Operation);
			if (Operation == WHITENESS)
			{
				Operation = BLACKNESS;
			}
			else
			{
				Operation = WHITENESS;
			}
			EndPaint(Window, &Paint);
		} break;

		default:
		{
//				OutputDebugStringA("default\n");
			Result = DefWindowProcA(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);

}

int CALLBACK WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
//		MessageBoxA(0, "This is Handmade Hero.", "Handmade Hero", MB_OK | MB_ICONINFORMATION); // Sample Message Box

	WNDCLASS WindowClass = {};

	// TODO: check if CS_OWNDC|CS_HREDRAW|CS_VREDRAW still matter
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = MainWindowCallback;
	WindowClass.hInstance = Instance;
//		WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	if (RegisterClass(&WindowClass))
	{
		HWND WindowHandle = CreateWindowExA(
			0,
			WindowClass.lpszClassName,
			"Handmade Hero",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			0,
			0,
			Instance,
			0
		);
		if (WindowHandle)
		{
			Running = true;
			while(Running)
			{
                MSG Message;
				BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			// TODO: Logging
		}
	}
	else
	{
		// TODO: Logging
	}

	return(0);
}