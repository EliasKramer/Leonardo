#pragma once
#include <cstdint>
#include <array>
#include <map>

typedef uint64_t BitBoard;

const BitBoard BITBOARD_ALL = 0xFFFFFFFFFFFFFFFF;
const BitBoard BITBOARD_NONE = 0;

const BitBoard FILE_A = 0x101010101010101ULL;
const BitBoard FILE_B = FILE_A << 1;
const BitBoard FILE_C = FILE_A << 2;
const BitBoard FILE_D = FILE_A << 3;
const BitBoard FILE_E = FILE_A << 4;
const BitBoard FILE_F = FILE_A << 5;
const BitBoard FILE_G = FILE_A << 6;
const BitBoard FILE_H = FILE_A << 7;

constexpr BitBoard RANK_1 = 0xFF;
const BitBoard RANK_2 = RANK_1 << (8 * 1);
const BitBoard RANK_3 = RANK_1 << (8 * 2);
const BitBoard RANK_4 = RANK_1 << (8 * 3);
const BitBoard RANK_5 = RANK_1 << (8 * 4);
const BitBoard RANK_6 = RANK_1 << (8 * 5);
const BitBoard RANK_7 = RANK_1 << (8 * 6);
const BitBoard RANK_8 = RANK_1 << (8 * 7);

const BitBoard BLACK_SQUARES = 0xaa55aa55aa55aa55ULL;
const BitBoard WHITE_SQUARES = 0x55aa55aa55aa55aaULL;

enum Square : int {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,

	SQUARE_NONE
};

extern const std::array<BitBoard, 64> BB_SQUARE;

const int NUMBER_OF_DIRECTIONS = 16;

enum Direction : int8_t {
	NORTH = 8, // bitshift 8 left
	EAST = 1, // bitshift 1 left
	SOUTH = -NORTH, // bitshift 8 right
	WEST = -EAST, // bitshift 1 right

	NORTH_EAST = NORTH + EAST, // bitshift 9 left
	SOUTH_EAST = SOUTH + EAST, // bitshift 1 left
	SOUTH_WEST = SOUTH + WEST, // bitshift 7 left
	NORTH_WEST = NORTH + WEST, // bitshift 7 right

	NORTH_NORTH_EAST = NORTH + NORTH_EAST,
	EAST_NORTH_EAST = EAST + NORTH_EAST,
	EAST_SOUTH_EAST = EAST + SOUTH_EAST,
	SOUTH_SOUTH_EAST = SOUTH + SOUTH_EAST,

	SOUTH_SOUTH_WEST = SOUTH + SOUTH_WEST,
	WEST_SOUTH_WEST = WEST + SOUTH_WEST,
	WEST_NORTH_WEST = WEST + NORTH_WEST,
	NORTH_NORTH_WEST = NORTH + NORTH_WEST
};

//if you start at one of the bits in the bitboard and go in the key direction, 
//then your final position will not be on the board

const std::map<Direction, BitBoard> INVALID_FIELDS_FOR_DIR = {
	{NORTH, RANK_8},
	{ SOUTH, RANK_1 },
	{ WEST, FILE_A },
	{ EAST, FILE_H },

	{ NORTH_EAST, FILE_H | RANK_8 },
	{ SOUTH_EAST, FILE_H | RANK_1 },
	{ SOUTH_WEST, FILE_A | RANK_1 },
	{ NORTH_WEST, FILE_A | RANK_8 },

	{ NORTH_NORTH_EAST, FILE_H | RANK_8 | RANK_7 },
	{ EAST_NORTH_EAST, FILE_H | FILE_G | RANK_8 },
	{ EAST_SOUTH_EAST, FILE_H | FILE_G | RANK_1 },
	{ SOUTH_SOUTH_EAST, FILE_H | RANK_1 | RANK_2 },

	{ SOUTH_SOUTH_WEST, FILE_A | RANK_1 | RANK_2 },
	{ WEST_SOUTH_WEST, FILE_A | FILE_B | RANK_1 },
	{ WEST_NORTH_WEST, FILE_A | FILE_B | RANK_8 },
	{ NORTH_NORTH_WEST, FILE_A | RANK_8 | RANK_7 }
};

bool destinationIsOnBoard(Square start, Direction direction);
bool squareOverlapsWithBB(Square pos, BitBoard bb);
bool bitboardsOverlap(BitBoard first, BitBoard second);