#include "MoveCastle.h"

MoveCastle::MoveCastle(Square givenStart, Square givenDest, Square rookStart, Square rookDest)
	:Move(givenStart, givenDest),
	_rookStart(rookStart),
	_rookDest(rookDest)
{}

MoveCastle::MoveCastle(ChessColor color, CastlingType castlingType)
	:Move(SQUARES_FOR_KING_CASTLING[color][castlingType][0],
		SQUARES_FOR_KING_CASTLING[color][castlingType][2]),
	_rookStart(SQUARES_FOR_ROOK_CASTLING[color][castlingType][0]),
	_rookDest(SQUARES_FOR_ROOK_CASTLING[color][castlingType][1])
{}

const Square MoveCastle::getRookStart()
{
	return _rookStart;
}

const Square MoveCastle::getRookDest()
{
	return _rookDest;
}

void MoveCastle::execute(BoardRepresentation& board)
{
	Move::execute(board);
	board.copySquareToPos(_rookStart, _rookDest);
	board.delAtPos(_rookStart);
}

BitBoard MoveCastle::getBBWithMoveDone()
{
	throw "ERROR. A Caslting Move does not produce a Move Bitboard";
}