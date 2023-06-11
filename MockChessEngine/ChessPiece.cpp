#include "ChessPiece.h"

ChessPiece::ChessPiece(
	ChessColor color,
	PieceType type) :
	_color(color),
	_type(type)
{}

ChessPiece::ChessPiece(char fenChar)
{
	_color = getColorOfFenChar(
		fenChar,
		"ERROR Chess Piece could not be created.");

	
	if (isUpperCase(fenChar))
	{
		fenChar = tolower(fenChar);
	}

	switch (fenChar)
	{
	case 'p':
		_type = Pawn;
		break;
	case 'r':
		_type = Rook;
		break;
	case 'n':
		_type = Knight;
		break;
	case 'b':
		_type = Bishop;
		break;
	case 'q':
		_type = Queen;
		break;
	case 'k':
		_type = King;
		break;
	default:
		throw "ERROR Tried to create Chess Piece from an invalid fen character. (Type not found)";
	}
}

PieceType ChessPiece::getType() const
{
	return _type;
}

ChessColor ChessPiece::getColor() const
{
	return _color;
}

bool operator==(const ChessPiece& first, const ChessPiece& second)
{
	return 
		first._type == second._type &&
		first._color == second._color;
}

bool operator!=(const ChessPiece& first, const ChessPiece& second)
{
	return !(first == second);
}