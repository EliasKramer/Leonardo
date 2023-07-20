#include "BoardRepresentation.h"
#include <iostream>
BoardRepresentation::BoardRepresentation()
	:
	AllPieces(BITBOARD_NONE),
	PiecesOfColor{
	BITBOARD_NONE,
	BITBOARD_NONE },

	PiecesOfType{
	BITBOARD_NONE,
	BITBOARD_NONE,
	BITBOARD_NONE,
	BITBOARD_NONE,
	BITBOARD_NONE,
	BITBOARD_NONE },

	KingPos{ E1, E8 }
{}

ChessPiece BoardRepresentation::getPieceAt(Square square) const
{
	BitBoard squareBB = BB_SQUARE[square];
	//can be improved by adding null types
	ChessColor colFound = White;
	PieceType typeFound = Rook;

	bool foundColor = false;
	bool foundType = false;
	for (int i = 0; i < DIFFERENT_CHESS_COLORS; i++)
	{
		ChessColor currCol = (ChessColor)i;

		if (bitboardsOverlap(PiecesOfColor[currCol], squareBB))
		{
			colFound = currCol;
			foundColor = true;
			break;
		}
	}
	for (int i = 0; i < NUMBER_OF_DIFFERENT_PIECE_TYPES; i++)
	{
		PieceType currType = (PieceType)i;

		if (bitboardsOverlap(PiecesOfType[currType], squareBB))
		{
			typeFound = currType;
			foundType = true;
			break;
		}
	}

	return ChessPiece(colFound, typeFound);
}

void BoardRepresentation::copySquareToPos(Square copyField, Square pasteField)
{
	BitBoard copyPos = BB_SQUARE[copyField];
	BitBoard pastePos = BB_SQUARE[pasteField];

	for (int i = 0; i < DIFFERENT_CHESS_COLORS; i++)
	{
		ChessColor currCol = (ChessColor)i;

		if ((PiecesOfColor[currCol] & copyPos) != 0) //curr color has is 
		{
			PiecesOfColor[currCol] = PiecesOfColor[currCol] | pastePos;
		}
		else
		{
			PiecesOfColor[currCol] = PiecesOfColor[currCol] & (~pastePos);
		}
	}

	for (int i = 0; i < NUMBER_OF_DIFFERENT_PIECE_TYPES; i++)
	{
		PieceType currType = (PieceType)i;

		if ((PiecesOfType[currType] & copyPos) != 0)
		{
			PiecesOfType[currType] = PiecesOfType[currType] | pastePos;
		}
		else
		{
			PiecesOfType[currType] = PiecesOfType[currType] & (~pastePos);
		}
	}

	if ((AllPieces & copyPos) != 0)
	{
		AllPieces = AllPieces | pastePos;
	}
}

void BoardRepresentation::setAtPosition(ChessPiece piece, Square position)
{
	BitBoard piecePos = BB_SQUARE[position];

	setPieceBitBoard(piece, piecePos);
}

void BoardRepresentation::setPieceBitBoard(ChessPiece piece, BitBoard bitboard)
{
	AllPieces = AllPieces | bitboard;

	ChessColor col = piece.getColor();
	PiecesOfColor[col] = PiecesOfColor[col] | bitboard;

	PieceType type = piece.getType();
	PiecesOfType[type] = PiecesOfType[type] | bitboard;
}

void BoardRepresentation::delAtPos(Square position)
{
	BitBoard keepPiecesMask = ~(BB_SQUARE[position]);

	AllPieces = AllPieces & keepPiecesMask;

	for (int i = 0; i < DIFFERENT_CHESS_COLORS; i++)
	{
		ChessColor currCol = (ChessColor)i;
		PiecesOfColor[currCol] = PiecesOfColor[currCol] & keepPiecesMask;
	}

	for (int i = 0; i < NUMBER_OF_DIFFERENT_PIECE_TYPES; i++)
	{
		PieceType currType = (PieceType)i;
		PiecesOfType[currType] = PiecesOfType[currType] & keepPiecesMask;
	}
}

bool operator==(const BoardRepresentation& first, const BoardRepresentation& second)
{
	bool retVal = first.AllPieces == second.AllPieces
		&& (first.PiecesOfColor[White] == second.PiecesOfColor[White])
		&& (first.PiecesOfColor[Black] == second.PiecesOfColor[Black]);

	for (int i = 0; i < NUMBER_OF_DIFFERENT_PIECE_TYPES; i++)
	{
		retVal = retVal && (first.PiecesOfType[i] == second.PiecesOfType[i]);
	}

	retVal = retVal && (first.KingPos[White] == second.KingPos[White]);
	retVal = retVal && (first.KingPos[Black] == second.KingPos[Black]);

	return retVal;
}

bool operator!=(const BoardRepresentation& first, const BoardRepresentation& second)
{
	return !(first == second);
}