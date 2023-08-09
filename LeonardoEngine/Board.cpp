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

bool Board::squareIsAttackedBy(square square, color color)
{
	bitboard position = 1ULL << square;

	bitboard enemyPawns = color == WHITE ? whitePieces & pawns : blackPieces & pawns;
	bitboard enemyKnights = color == WHITE ? whitePieces & knights : blackPieces & knights;
	bitboard enemyBishops = color == WHITE ? whitePieces & bishops : blackPieces & bishops;
	bitboard enemyRooks = color == WHITE ? whitePieces & rooks : blackPieces & rooks;
	bitboard enemyQueens = color == WHITE ? whitePieces & queens : blackPieces & queens;
	bitboard enemyKing = color == WHITE ? whitePieces & kings : blackPieces & kings;

	bitboard dangerousPawns = color == WHITE ? position >> 7 | position >> 9 : position << 7 | position << 9;
	bitboard dangerousKnights = position << 17 | position << 15 | position << 10 | position << 6 | position >> 17 | position >> 15 | position >> 10 | position >> 6;
	bitboard dangerousKings = position << 8 | position << 1 | position >> 1 | position >> 8 | position << 7 | position << 9 | position >> 7 | position >> 9;
	
	if ((dangerousPawns & enemyPawns) || (dangerousKnights & enemyKnights) || (dangerousKings & enemyKing))
	{
		return true;
	}

	bitboard enemyRooksQueens = enemyRooks | enemyQueens;
	bitboard enemyBishopsQueens = enemyBishops | enemyQueens;
	bitboard file = 0x101010101010101ULL << (square % 8);
	bitboard rank = 0xFFULL << (square - (square % 8));
	//checking each direction for attack, but for the file and rank we only check if there is a rook or queen
	if (((enemyRooksQueens & rank) && (checkDirectionForAttack(square, LEFT, enemyRooksQueens) || checkDirectionForAttack(square, RIGHT, enemyRooksQueens))) ||
		((enemyRooksQueens & file) && (checkDirectionForAttack(square, UP, enemyRooksQueens) || checkDirectionForAttack(square, DOWN, enemyRooksQueens))) ||
		checkDirectionForAttack(square, LEFT_UP, enemyBishopsQueens) || checkDirectionForAttack(square, RIGHT_UP, enemyBishopsQueens) ||
		checkDirectionForAttack(square, LEFT_DOWN, enemyBishopsQueens) || checkDirectionForAttack(square, RIGHT_DOWN, enemyBishopsQueens))
	{
		return true;
	}

	return false;
}

bool Board::checkDirectionForAttack(square position, direction dir, bitboard enemySlidingPieces)
{
	square nextSquare = position;
	bitboard nextPosition = 1ULL << nextSquare;
	bool isAtEdge = nextPosition & EDGES.at(dir);

	while (!isAtEdge) 
	{
		nextSquare = (square)(nextSquare + dir);
		nextPosition = 1ULL << nextSquare;
		if (nextPosition & getAllPieces())
		{
			if (nextPosition & enemySlidingPieces) 
			{
				return true;
			}
			return false;
		}
		isAtEdge = nextPosition & EDGES.at(dir);
	}
	return false;
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

color Board::getTurnColor()
{
	return turnColor;
}

void Board::switchTurnColor()
{
	turnColor = (color)!turnColor;
}

bitboard Board::getEnPassant()
{
	return enPassant;
}

void Board::setEnPassant(bitboard position)
{
	enPassant = position;
}

bool Board::getWhiteLeftCastleAvailable()
{
	return whiteLeftCastleAvailable;
}

bool Board::getWhiteRightCastleAvailable()
{
	return whiteRightCastleAvailable;
}

bool Board::getBlackLeftCastleAvailable()
{
	return blackLeftCastleAvailable;
}

bool Board::getBlackRightCastleAvailable()
{
	return blackRightCastleAvailable;
}

void Board::whiteCastle()
{
	whiteLeftCastleAvailable = false;
	whiteRightCastleAvailable = false;
}
void Board::blackCastle()
{
	blackLeftCastleAvailable = false;
	blackRightCastleAvailable = false;
}
