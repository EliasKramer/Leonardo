#pragma once
#include "BitBoard.h"
#include "DataAndTypes.h"
#include "Constants.h"
#include "ChessPiece.h"
#include <vector>

class BoardRepresentation
{
public:
	BitBoard AllPieces;
	BitBoard PiecesOfColor[DIFFERENT_CHESS_COLORS];
	BitBoard PiecesOfType[NUMBER_OF_DIFFERENT_PIECE_TYPES];

	Square KingPos[DIFFERENT_CHESS_COLORS];

	BoardRepresentation();

	ChessPiece getPieceAt(Square square) const;

	void copySquareToPos(Square copyField, Square pasteField);
	//be aware, that this does not overwrite the square, it only adds it
	void setAtPosition(ChessPiece piece, Square position);
	//be aware, that this does not overwrite the Bitboard, it only adds it
	void setPieceBitBoard(ChessPiece piece, BitBoard bitboard);

	void delAtPos(Square position);
};
bool operator==(const BoardRepresentation& first, const BoardRepresentation& second);
bool operator!=(const BoardRepresentation& first, const BoardRepresentation& second);