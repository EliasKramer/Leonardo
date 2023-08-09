#include "Board.h"

Board::Board(bitboard pawns, bitboard knights, bitboard bishops, bitboard rooks, bitboard queens, bitboard kings, bitboard whitePieces, bitboard blackPieces) : 
	pawns(pawns), 
	knights(knights), 
	bishops(bishops), 
	rooks(rooks), 
	queens(queens), 
	kings(kings), 
	whitePieces(whitePieces), 
	blackPieces(blackPieces),
	whitePiecesList(getPiecesOfColor(WHITE)),
	blackPiecesList(getPiecesOfColor(BLACK))
{
}


std::vector<piece> Board::getPiecesOfColor(color color)
{
	std::vector<piece> pieces;
	bitboard piecesBB = color == WHITE ? whitePieces : blackPieces;
	bitboard currentPosition = 0x1;
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

pieceType Board::getType(bitboard pieceBB) 
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

bitboard Board::getPawns()
{
	return pawns;
}
bitboard Board::getKnights()
{
	return knights;
}
bitboard Board::getBishops()
{
	return bishops;
}
bitboard Board::getRooks()
{
	return rooks;
}
bitboard Board::getQueens()
{
	return queens;
}
bitboard Board::getKings()
{
	return kings;
}

bitboard Board::getWhitePieces()
{
	return whitePieces;
}
bitboard Board::getBlackPieces()
{
	return blackPieces;
}
bitboard Board::getAllPieces()
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