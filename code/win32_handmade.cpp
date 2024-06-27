#include <Windows.h>
#include <stdint.h>

// define to keep track of static variables using the name.
#define internal static
#define local_persist static
#define global_variable static

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

// TODO: This is a global for now.
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return(Result);
}

internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int BlueOffset, int GreenOffset)
{
	// TODO: Let's see what the optimizer does

	uint8* Row = (uint8*)Buffer.Memory;
	for (int Y = 0;
		Y < Buffer.Height;
		++Y)
	{
		uint32* Pixel = (uint32*)Row;
		for (int X = 0;
			X < Buffer.Width;
			++X)
		{
			// This is BLUE
			uint8 Blue = (X + BlueOffset);
			uint8 Green = (Y + GreenOffset);

			/*
				Memory:		BB GG RR xx
				Register:	xx RR GG BB

				Pixel (32-bits)
			*/

			*Pixel++ = ((Green << 8) | Blue);
		}
		Row += Buffer.Pitch;
	}
}

//Renaming all names with Win32 to avoid confusion with Windows actual function names.
internal void 
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	// TODO: Bulletproof this.
	// Maybe don't free first, free after, then free first if that fails.

	if (Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	// NOTE: When the biHeight field is negative, this is the clue to Windows
	// to treat this bitmap as top-down, not bottom-up, meaning that the first
	// three bytes of the image are the color for the top left pixel in the
	// bitmap, not the bottom left!
	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	// NOTE: No more DC
	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	Buffer->Pitch = Width * Buffer->BytesPerPixel;

	// TODO: Probably clear this to black
}

internal void 
Win32DisplayBufferInWindow(	HDC DeviceContext, int WindowWidth, int WindowHeight, 
							win32_offscreen_buffer Buffer,
							int X, int Y, int Width, int Height)
{
	// TODO: Aspect ratio correction
	StretchDIBits(DeviceContext, 
		/*
		X, Y, Width, Height, // Destination values
		X, Y, Width, Height, // Source Values
		*/
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS, // Table containing literal RGB Values
		SRCCOPY); // Coies directly from source to destination rectangle.
}

LRESULT CALLBACK 
Win32MainWindowCallback(
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
			
		} break;

		case WM_DESTROY:
		{
			// TODO: Handle this with an error - recreate window?
			GlobalRunning = false;
		} break;

		case WM_CLOSE:
		{
			// TODO: Handle this with a message to the user?
			GlobalRunning = false;
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
			int Width = Paint.rcPaint.right - Paint.rcPaint.left;
			int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			win32_window_dimension Dimension = Win32GetWindowDimension(Window);
			Win32DisplayBufferInWindow(	DeviceContext, Dimension.Width, Dimension.Height, 
										GlobalBackBuffer, X, Y, Width, Height);

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

int CALLBACK 
WinMain(
	HINSTANCE Instance,
	HINSTANCE PrevInstance,
	LPSTR CommandLine,
	int ShowCode)
{
//		MessageBoxA(0, "This is Handmade Hero.", "Handmade Hero", MB_OK | MB_ICONINFORMATION); // Sample Message Box

	WNDCLASS WindowClass = {};

	Win32ResizeDIBSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
//		WindowClass.hIcon;
	WindowClass.lpszClassName = "HandmadeHeroWindowClass";
	if (RegisterClass(&WindowClass))
	{
		HWND Window = 
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"Handmade Hero",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if (Window)
		{
			int XOffset = 0;
			int YOffset = 0;
			GlobalRunning = true;
			while(GlobalRunning)
			{
				MSG Message;
				while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
				{
					if (Message.message == WM_QUIT)
					{
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}

				RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

				HDC DeviceContext = GetDC(Window);

				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer, 0, 0, Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);

				++XOffset;
				YOffset += 2;
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