#include "Board.h"

Board::Board(map pawns, map knights, map rooks, map bishops, map queens, map kings, map whitePieces, map blackPieces) 
{
	this->pawns = pawns;
	this->knights = knights;
	this->rooks = rooks;
	this->bishops = bishops;
	this->queens = queens;
	this->kings = kings;

	this->whitePieces = whitePieces;
	this->blackPieces = blackPieces;

	this->whitePiecesList = getPiecesOfColor(WHITE);
	this->blackPiecesList = getPiecesOfColor(BLACK);
}


std::vector<piece> Board::getPiecesOfColor(color color)
{
	std::vector<piece> pieces;
	map piecesBB = color == WHITE ? whitePieces : blackPieces;
	map currentPosition = 0x1;
	for (uint8_t i = 0; i < 64; i++)
	{
		if (piecesBB & currentPosition)
		{
			piece currentPiece;
			currentPiece.position = (square) i;
			currentPiece.color = color;
			currentPiece.type = getType(currentPosition);
			pieces.push_back(currentPiece);
		}
		currentPosition = currentPosition << 1;
	}
	return pieces;
}

pieceType Board::getType(map pieceBB) 
{
	pieceType type = NONE;
	if (pieceBB & pawns)
	{
		type = PAWN;
	}
	else if (pieceBB & knights)
	{
		type = KNIGHT;
	}
	else if (pieceBB & rooks)
	{
		type = ROOK;
	}
	else if (pieceBB & bishops)
	{
		type = BISHOP;
	}
	else if (pieceBB & queens)
	{
		type = QUEEN;
	}
	else if (pieceBB & kings)
	{
		type = KING;
	}
	return type;
}

map Board::getPawns()
{
	return pawns;
}
map Board::getKnights()
{
	return knights;
}
map Board::getBishops()
{
	return bishops;
}
map Board::getRooks()
{
	return rooks;
}
map Board::getQueens()
{
	return queens;
}
map Board::getKings()
{
	return kings;
}

map Board::getWhitePieces()
{
	return whitePieces;
}
map Board::getBlackPieces()
{
	return blackPieces;
}
map Board::getAllPieces()
{
	return whitePieces | blackPieces;
}

std::vector<piece> Board::getWhitePiecesList() 
{
	return whitePiecesList;
}
std::vector<piece> Board::getBlackPiecesList()
{
	return blackPiecesList;
}