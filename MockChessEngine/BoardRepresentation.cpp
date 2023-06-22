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

void BoardRepresentation::validation(const std::string& s) const
{
	if (AllPieces != (PiecesOfColor[White] | PiecesOfColor[Black]))
	{
		std::cout << "not valid \n"
			<< s << "\n"
			<< "PiecesOfColor[White]: " << PiecesOfColor[White] << "\n"
			<< "PiecesOfColor[Black]: " << PiecesOfColor[Black] << "\n"
			<< "PiecesOfType[King]: " << PiecesOfType[King] << "\n"
			<< "PiecesOfType[Queen]: " << PiecesOfType[Queen] << "\n"
			<< "PiecesOfType[Rook]: " << PiecesOfType[Rook] << "\n"
			<< "PiecesOfType[Bishop]: " << PiecesOfType[Bishop] << "\n"
			<< "PiecesOfType[Knight]: " << PiecesOfType[Knight] << "\n"
			<< "PiecesOfType[Pawn]: " << PiecesOfType[Pawn] << "\n"
			<< "AllPieces: " << AllPieces << "\n";
		throw std::exception("AllPieces is not equal to the sum of PiecesOfColor[White] and PiecesOfColor[Black]");
	}
}

ChessPiece BoardRepresentation::getPieceAt(Square square) const
{
	validation("get piece at " + std::to_string(square) + "start");


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

	if (!foundColor || !foundType)
	{
		std::cout << "exception. no piece found at square: " << std::to_string(square) << "\n"
			<< "PiecesOfColor[White]: " << PiecesOfColor[White] << "\n"
			<< "PiecesOfColor[Black]: " << PiecesOfColor[Black] << "\n"
			<< "PiecesOfType[King]: " << PiecesOfType[King] << "\n"
			<< "PiecesOfType[Queen]: " << PiecesOfType[Queen] << "\n"
			<< "PiecesOfType[Rook]: " << PiecesOfType[Rook] << "\n"
			<< "PiecesOfType[Bishop]: " << PiecesOfType[Bishop] << "\n"
			<< "PiecesOfType[Knight]: " << PiecesOfType[Knight] << "\n"
			<< "PiecesOfType[Pawn]: " << PiecesOfType[Pawn] << "\n"
			<< "squareBB: " << squareBB << "\n"
			<< "AllPieces: " << AllPieces << "\n";
		throw std::exception("There was no piece found at the given square");
	}
	validation("get piece at " + std::to_string(square) + "end");

	return ChessPiece(colFound, typeFound);
}


void BoardRepresentation::copySquareToPos(Square copyField, Square pasteField)
{
	validation("copy field " + std::to_string(copyField) + " paste field " + std::to_string(pasteField) + " start ");

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
	validation("copy field " + std::to_string(copyField) + " paste field " + std::to_string(pasteField) + " end ");

}

void BoardRepresentation::setAtPosition(ChessPiece piece, Square position)
{
	validation("copy field " + std::to_string(PIECETYPE_CHAR[piece.getType()]) + " | " + std::to_string(piece.getColor()) + " pos" + std::to_string(position) + "start ");

	if (AllPieces != (PiecesOfColor[White] | PiecesOfColor[Black]))
		throw std::exception("AllPieces is not equal to the sum of PiecesOfColor[White] and PiecesOfColor[Black]");

	BitBoard piecePos = BB_SQUARE[position];

	setPieceBitBoard(piece, piecePos);

	validation("copy field " + std::to_string(PIECETYPE_CHAR[piece.getType()]) + " | " + std::to_string(piece.getColor()) + " pos" + std::to_string(position) + "end ");
}

void BoardRepresentation::setPieceBitBoard(ChessPiece piece, BitBoard bitboard)
{
	validation("set piece bitboard field " + std::to_string(PIECETYPE_CHAR[piece.getType()]) + " | " + std::to_string(piece.getColor()) + " bb " + std::to_string(bitboard) + "start ");

	AllPieces = AllPieces | bitboard;

	ChessColor col = piece.getColor();
	PiecesOfColor[col] = PiecesOfColor[col] | bitboard;

	PieceType type = piece.getType();
	PiecesOfType[type] = PiecesOfType[type] | bitboard;
	validation("set piece bitboard field " + std::to_string(PIECETYPE_CHAR[piece.getType()]) + " | " + std::to_string(piece.getColor()) + " bb " + std::to_string(bitboard) + "end ");

}

void BoardRepresentation::delAtPos(Square position)
{
	validation("del at pos start " + std::to_string(position));

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
	validation("del at pos end " + std::to_string(position));

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