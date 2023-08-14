#pragma once
#include <map>
#include <string>
#include <array>

using bitboard = uint64_t;

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
	UP = 8,
	RIGHT = 1,
	DOWN = -UP,
	LEFT = -RIGHT,

	RIGHT_UP = UP + RIGHT,
	RIGHT_DOWN = DOWN + RIGHT,
	LEFT_DOWN = DOWN + LEFT,
	LEFT_UP = UP + LEFT,

	UP_UP_RIGHT = UP + RIGHT_UP,
	RIGHT_RIGHT_UP = RIGHT + RIGHT_UP,
	RIGHT_RIGHT_DOWN = RIGHT + RIGHT_DOWN,
	DOWN_DOWN_RIGHT = DOWN + RIGHT_DOWN,

	DOWN_DOWN_LEFT = DOWN + LEFT_DOWN,
	LEFT_LEFT_DOWN = LEFT + LEFT_DOWN,
	LEFT_LEFT_UP = LEFT + LEFT_UP,
	UP_UP_LEFT = UP + LEFT_UP
};

enum row : bitboard
{
	RANK_1 = 0xFF, 
	RANK_2 = RANK_1 << UP, 
	RANK_3 = RANK_1 << 2 * UP,
	RANK_4 = RANK_1 << 3 * UP,
	RANK_5 = RANK_1 << 4 * UP,
	RANK_6 = RANK_1 << 5 * UP,
	RANK_7 = RANK_1 << 6 * UP,
	RANK_8 = RANK_1 << 7 * UP,

	FILE_A = 0x0101010101010101,
	FILE_B = FILE_A << RIGHT,
	FILE_C = FILE_A << 2 * RIGHT,
	FILE_D = FILE_A << 3 * RIGHT,
	FILE_E = FILE_A << 4 * RIGHT,
	FILE_F = FILE_A << 5 * RIGHT,
	FILE_G = FILE_A << 6 * RIGHT,
	FILE_H = FILE_A << 7 * RIGHT
};

extern const std::map<direction, bitboard> EDGES;

struct piece 
{
	pieceType type;
	color color;
	square position;
};