#include "Types.h"

const std::map<direction, bitboard> EDGES = {
	{ NORTH, RANK_8 },
	{ NORTH_NORTH_EAST, RANK_8 | RANK_7 | FILE_H},
	{ NORTH_EAST, RANK_8 | FILE_A },
	{ EAST_NORTH_EAST, RANK_8 | RANK_7 | FILE_A },

	{ EAST, FILE_A },
	{ EAST_SOUTH_EAST, RANK_1 | RANK_2 | FILE_A },
	{ SOUTH_EAST, RANK_1 | FILE_A },
	{ SOUTH_SOUTH_EAST, RANK_1 | RANK_2 | FILE_H },

	{ SOUTH, RANK_1 },
	{ SOUTH_SOUTH_WEST, RANK_1 | RANK_2 | FILE_H },
	{ SOUTH_WEST, RANK_1 | FILE_H },
	{ WEST_SOUTH_WEST, RANK_1 | RANK_2 | FILE_H },

	{ WEST, FILE_H },
	{ WEST_NORTH_WEST, RANK_8 | RANK_7 | FILE_H },
	{ NORTH_WEST, RANK_8 | FILE_H },
	{ NORTH_NORTH_WEST, RANK_8 | RANK_7 | FILE_A }
};

const std::array<std::string, 64> SQUARE_NAMES = []()->std::array<std::string, 64>{
	std::array < std::string, 64> retVal = {
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
	};
	return retVal;
}();	