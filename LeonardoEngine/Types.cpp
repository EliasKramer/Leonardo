#include "Types.h"

const std::map<direction, bitboard> EDGES = {
	{ UP, RANK_8 },
	{ UP_UP_RIGHT, RANK_8 | RANK_7 | FILE_H},
	{ RIGHT_UP, RANK_8 | FILE_H },
	{ RIGHT_RIGHT_UP, RANK_8 | FILE_G | FILE_H },

	{ RIGHT, FILE_H },
	{ RIGHT_RIGHT_DOWN, RANK_1 | FILE_G | FILE_H },
	{ RIGHT_DOWN, RANK_1 | FILE_H },
	{ DOWN_DOWN_RIGHT, RANK_1 | RANK_2 | FILE_H },

	{ DOWN, RANK_1 },
	{ DOWN_DOWN_LEFT, RANK_1 | RANK_2 | FILE_A },
	{ LEFT_DOWN, RANK_1 | FILE_A },
	{ LEFT_LEFT_DOWN, RANK_1 | FILE_A | FILE_B },

	{ LEFT, FILE_A },
	{ LEFT_LEFT_UP, RANK_8 | FILE_A | FILE_B },
	{ LEFT_UP, RANK_8 | FILE_A },
	{ UP_UP_LEFT, RANK_8 | RANK_7 | FILE_A }
};

const std::array<std::string, 64> SQUARE_NAMES = {
	"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
	"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
	"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
	"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
	"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
	"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
	"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
	"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};
