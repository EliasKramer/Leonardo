#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <array>

using map = uint64_t;

enum square : uint8_t 
{
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8
};

extern const std::array<std::string, 64> SQARE_NAMES;

enum pieceType : uint8_t
{
	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NONE
};

enum color : uint8_t
{
	WHITE, BLACK 
};

enum direction : int8_t {
	NORTH = 8,
	EAST = 1,
	SOUTH = -NORTH,
	WEST = -EAST,

	NORTH_EAST = NORTH + EAST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST,
	NORTH_WEST = NORTH + WEST,

	NORTH_NORTH_EAST = NORTH + NORTH_EAST,
	EAST_NORTH_EAST = EAST + NORTH_EAST,
	EAST_SOUTH_EAST = EAST + SOUTH_EAST,
	SOUTH_SOUTH_EAST = SOUTH + SOUTH_EAST,

	SOUTH_SOUTH_WEST = SOUTH + SOUTH_WEST,
	WEST_SOUTH_WEST = WEST + SOUTH_WEST,
	WEST_NORTH_WEST = WEST + NORTH_WEST,
	NORTH_NORTH_WEST = NORTH + NORTH_WEST
};

enum row : map
{
	RANK_1 = 0xFF, 
	RANK_2 = RANK_1 << NORTH, 
	RANK_3 = RANK_1 << 2 * NORTH,
	RANK_4 = RANK_1 << 3 * NORTH,
	RANK_5 = RANK_1 << 4 * NORTH,
	RANK_6 = RANK_1 << 5 * NORTH,
	RANK_7 = RANK_1 << 6 * NORTH,
	RANK_8 = RANK_1 << 7 * NORTH,

	FILE_A = 0x0101010101010101,
	FILE_B = FILE_A << EAST,
	FILE_C = FILE_A << 2 * EAST,
	FILE_D = FILE_A << 3 * EAST,
	FILE_E = FILE_A << 4 * EAST,
	FILE_F = FILE_A << 5 * EAST,
	FILE_G = FILE_A << 6 * EAST,
	FILE_H = FILE_A << 7 * EAST
};

extern const std::map<direction, map> EDGES;

struct piece 
{
	pieceType type;
	color color;
	square position;
};