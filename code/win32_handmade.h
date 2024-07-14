#if !defined(WIN32_HANDMADE_H)

struct win32_offscreen_buffer
{
	// NOTE: Pixels are always 32-bits wide, Memory Order BB GG RR XX
	BITMAPINFO Info;
	void* Memory;
	int Width;
	int Height;
	int Pitch;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

struct win32_sound_output
{
	int SamplesPerSecond;
	int ToneHz;
	int ToneVolume;
	uint32 RunningSampleIndex;
	int BytesPerSample;
	int SecondaryBufferSize;
	real32 tSine;
	int LatencySampleCount;
};

#define WIN32_HANDMADE_H
#endif