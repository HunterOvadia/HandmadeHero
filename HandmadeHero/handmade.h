#if !defined(HANDMADE_H)

#include <math.h>
#include <stdint.h>

#define HANDMADE_INTERNAL 1

#define internal      static
#define global		static
#define local	 	static

#define Pi32		 3.14159265359f

typedef int8_t		 int8;
typedef int16_t		int16;
typedef int32_t		int32;
typedef int64_t		int64;
typedef int32		  bool32;

typedef uint8_t		uint8;
typedef uint16_t   	uint16;
typedef uint32_t	   uint32;
typedef uint64_t	   uint64;

typedef float		  real32;
typedef double		 real64;

#ifdef HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) { *(int *)0 = 0; }
#else
#define Assert(Expression)
#endif

#ifdef HANDMADE_INTERNAL
struct debug_read_file_result
{
	uint32 ContentSize;
	void *Contents;
};
debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
void DEBUGPlatformFreeFileMemory(void *Memory);
bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint64 MemorySize, void *Memory);
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Kilobytes(n) ((n) * 1024)
#define Megabytes(n) (Kilobytes(n) * 1024)
#define Gigabytes(n) (Megabytes(n) * 1024)
#define Terabytes(n) (Gigabytes(n) * 1024)

inline uint32
SafeTruncateUInt64(uint64 Value)
{
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return(Result);
}


struct game_offscreen_buffer
{
	void *Memory;
	int Width;
	int Height;
	int Pitch;
};

struct game_sound_output_buffer
{
	int SampleCount;
	int SamplesPerSecond;
	int16 *Samples;
};

struct game_button_state
{
	int HalfTransitionCount;
	bool32 EndedDown;
};

struct game_controller_input
{
    
	bool32 IsAnalog;
    
	real32 StartX;
	real32 StartY;
    
	real32 MinX;
	real32 MinY;
    
	real32 MaxX;
	real32 MaxY;
    
	real32 EndY;
	real32 EndX;
    
	union
	{
		game_button_state Buttons[6];
		struct
		{
			game_button_state Up;
			game_button_state Down;
			game_button_state Left;
			game_button_state Right;
			game_button_state LeftShoulder;
			game_button_state RightShoulder;
		};
	};
    
};


struct game_input
{
	game_controller_input Controllers[4];
};


struct game_memory
{
	bool32 IsInitialized;
    
	uint64 PermanentStorageSize;
	void *PermanentStorage;
    
	uint64 TransientStorageSize;
	void *TransientStorage;
};


internal void GameUpdateAndRender(game_memory *Memory,
								  game_input *Input,
								  game_offscreen_buffer *Buffer,
								  game_sound_output_buffer *SoundBuffer);
struct game_state
{
	int BlueOffset;
	int GreenOffset;
	int ToneHz;
};

#define HANDMADE_H
#endif