#include "BitBoard.h"

const std::array<BitBoard, 64> BB_SQUARE = []()->std::array<BitBoard, 64> {
	std::array<BitBoard, 64> retVal = {};

	for (int i = A1; i <= H8; i++)
	{
		retVal[i] = 1ULL << i;
	}

	return retVal;
}();

bool destinationIsOnBoard(Square start, Direction direction)
{
	//if the invalid board for the direction and
	//the start square dont overlap then the new pos is valid
	return (INVALID_FIELDS_FOR_DIR.at(direction) & BB_SQUARE[start]) == 0;
}

bool squareOverlapsWithBB(Square pos, BitBoard bb)
{
	return (BB_SQUARE[pos] & bb) != 0ULL;
}

bool bitboardsOverlap(BitBoard first, BitBoard second)
{
	return (first & second) != 0ULL;
}
