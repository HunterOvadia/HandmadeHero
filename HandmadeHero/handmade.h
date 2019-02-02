#if !defined(HANDMADE_H)

#include "handmade_platform.h"

#define internal		static
#define global			static
#define local	 		static

#define Pi32		 3.14159265359f

#ifdef HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) { *(int *)0 = 0; }
#else
#define Assert(Expression)
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

inline game_controller_input *
GetController(game_input *Input, unsigned int ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return(Result);
}

struct canonical_position
{
	int32 TileMapX;
	int32 TileMapY;

	int32 TileX;
	int32 TileY;

	real32 TileRelX;
	real32 TileRelY;
};

struct raw_position
{
	int32 TileMapX;
	int32 TileMapY;

	real32 X;
	real32 Y;
};

struct tile_map
{
	uint32 *Tiles;
};

struct world
{
	int32 CountX;
	int32 CountY;

	real32 UpperLeftX;
	real32 UpperLeftY;
	real32 TileWidth;
	real32 TileHeight;

	int32 TileMapCountX;
	int32 TileMapCountY;

	tile_map *TileMaps;
};


struct game_state
{
	int32 PlayerTileMapX;
	int32 PlayerTileMapY;

	real32 PlayerX;
	real32 PlayerY;
};

#define HANDMADE_H
#endif