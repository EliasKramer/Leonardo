#include "MoveCastle.h"

MoveCastle::MoveCastle(Square givenStart, Square givenDest, Square rookStart, Square rookDest)
	:Move(givenStart, givenDest),
	_rookStart(rookStart),
	_rookDest(rookDest)
{
	if (_start != E8 || _start != E1)
		throw "ERROR. A Castling Move must start from E1 or E8";
}

MoveCastle::MoveCastle(ChessColor color, CastlingType castlingType)
	:Move(SQUARES_FOR_KING_CASTLING[color][castlingType][0],
		SQUARES_FOR_KING_CASTLING[color][castlingType][2]), //bc 1 is not used
	_rookStart(SQUARES_FOR_ROOK_CASTLING[color][castlingType][0]),
	_rookDest(SQUARES_FOR_ROOK_CASTLING[color][castlingType][1])
{
	if(_start != E8 && _start != E1)
		throw "ERROR. A Castling Move must start from E1 or E8";

}

const Square MoveCastle::getRookStart()
{
	return _rookStart;
}

const Square MoveCastle::getRookDest()
{
	return _rookDest;
}

void MoveCastle::execute(BoardRepresentation& board) const
{
	Move::execute(board);
	board.copySquareToPos(_rookStart, _rookDest);
	board.delAtPos(_rookStart);
}
std::string MoveCastle::getString() const
{
	//if (_start != E8 && _start != E1)
		//throw "ERROR. A Castling Move must start from E1 or E8";
	if (_destination == G1 || _destination == G8)
	{
		return "O-O";
	}
	else
	{
		return "O-O-O";
	}
}
BitBoard MoveCastle::getBBWithMoveDone()
{
	throw "ERROR. A Caslting Move does not produce a Move Bitboard";
}