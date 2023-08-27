#include "MyBitBoard.h"

const std::array<BitBoard, 64> BB_SQUARE = []()->std::array<BitBoard, 64> {
	std::array<BitBoard, 64> retVal = {};

	for (int i = A1; i <= H8; i++)
	{
		retVal[i] = 1ULL << i;
	}

	return retVal;
}();

//this is bs remove later
const std::map<Direction, BitBoard>& getInvalidFieldsForDirection() {
	static const std::map<Direction, BitBoard> INVALID_FIELDS_FOR_DIR = {
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
	{ NORTH_NORTH_WEST, FILE_A | RANK_8 | RANK_7 } };
	return INVALID_FIELDS_FOR_DIR;
}


bool destinationIsOnBoard(Square start, Direction direction)
{
	static const std::map<Direction, BitBoard>& invalidFields = getInvalidFieldsForDirection();
	//if the invalid board for the direction and
	//the start square dont overlap then the new pos is valid
	return (invalidFields.at(direction) & BB_SQUARE[start]) == 0;
}

bool squareOverlapsWithBB(Square pos, BitBoard bb)
{
	return (BB_SQUARE[pos] & bb) != 0ULL;
}

bool bitboardsOverlap(BitBoard first, BitBoard second)
{
	return (first & second) != 0ULL;
}
