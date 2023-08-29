#include "Board.h"
#include <algorithm>
#include <sstream>

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
	if (kings & whitePieces)
	{
		whiteKingSquare = (square)(log2(kings & whitePieces));
	}
	if (kings & blackPieces)
	{
		blackKingSquare = (square)(log2(kings & blackPieces));
	}
}

Board::Board(std::string FEN, Color turnColor, square enPassantSquare, bool whiteLeftCastle, bool whiteRightCastle, bool blackLeftCastle, bool blackRightCastle) :
	pawns(0),
	knights(0),
	bishops(0),
	rooks(0),
	queens(0),
	kings(0),
	turnColor(turnColor),
	whiteLeftCastleAvailable(whiteLeftCastle),
	whiteRightCastleAvailable(whiteRightCastle),
	blackLeftCastleAvailable(blackLeftCastle),
	blackRightCastleAvailable(blackRightCastle)
{
	std::replace(FEN.begin(), FEN.end(), '/', ' ');
	std::stringstream iss(FEN);
	std::string piecePlacement;

	while (iss >> piecePlacement)
	{
		for (int i = (int)piecePlacement.size() - 1; i >= 0; i--)
		{
			whitePieces <<= 1;
			blackPieces <<= 1;
			pawns<<= 1;
			knights <<= 1;
			rooks <<= 1;
			bishops <<= 1;
			queens <<= 1;
			kings <<= 1;
			if (piecePlacement[i] >= '0' && piecePlacement[i] <= '8') 
			{
				whitePieces <<= (piecePlacement[i] - '0' - 1);
				blackPieces <<= (piecePlacement[i] - '0' - 1);
				pawns <<= (piecePlacement[i] - '0' - 1);
				knights <<= (piecePlacement[i] - '0' - 1);
				rooks <<= (piecePlacement[i] - '0' - 1);
				bishops <<= (piecePlacement[i] - '0' - 1);
				queens <<= (piecePlacement[i] - '0' - 1);
				kings <<= (piecePlacement[i] - '0' - 1);
				continue;
			}
			if (piecePlacement[i] >= 'A' && piecePlacement[i] <= 'Z')
			{
				whitePieces += 1;
			}
			else if (piecePlacement[i] >= 'a' && piecePlacement[i] <= 'z')
			{
				blackPieces += 1;
			}
			switch (piecePlacement[i])
			{
			case 'P': case 'p':
				pawns += 1;
				break;
			case 'N': case 'n':
				knights += 1;
				break;
			case 'R': case 'r':
				rooks += 1;
				break;
			case 'B': case 'b':
				bishops += 1;
				break;
			case 'Q': case 'q':
				queens += 1;
				break;
			case 'K': case 'k':
				kings += 1;
				break;
			default:
				throw;
			}
		}
	}

	this->enPassantSquare = enPassantSquare == SQUARE_NONE ? 0 : 1ULL << enPassantSquare;

	if (kings & whitePieces)
	{
		whiteKingSquare = (square)(log2(kings & whitePieces));
	}
	if (kings & blackPieces)
	{
		blackKingSquare = (square)(log2(kings & blackPieces));
	}
	whitePiecesList = getPiecesOfColor(WHITE);
	blackPiecesList = getPiecesOfColor(BLACK);
}

std::string Board::getFEN() 
{
	std::string FEN = "";
	bitboard cur = 0x100000000000000;
	for (int i = 0; i < 8; i++) 
	{
		int emptyCnt = 0;
		for (int j = 0; j < 8; j++) 
		{
			if (cur & (whitePieces | blackPieces))
			{
				if (emptyCnt > 0)
				{
					FEN += std::to_string(emptyCnt);
					emptyCnt = 0;
				}
				switch (getType(cur))
					{
				case PAWN:
					FEN += cur & whitePieces ? 'P' : 'p';
					break;
				case KNIGHT:
					FEN += cur & whitePieces ? 'N' : 'n';
					break;
				case BISHOP:
					FEN += cur & whitePieces ? 'B' : 'b';
					break;
				case ROOK:
					FEN += cur & whitePieces ? 'R' : 'r';
					break;
				case QUEEN:
					FEN += cur & whitePieces ? 'Q' : 'q';
					break;
				case KING:
					FEN += cur & whitePieces ? 'K' : 'k';
					break;
				}
			}
			else
			{
				emptyCnt++;
			}
			if (j < 7)
			{
				cur	<<= 1;
			}
		}
		if (emptyCnt > 0)
		{
			FEN += std::to_string(emptyCnt);
		}
		if (i != 7) 
		{
			FEN += '/';
		}
		cur >>= 15;
	}

	return FEN;
}
std::vector<Piece> Board::getPiecesOfColor(Color color)
{
	std::vector<Piece> pieces;
	bitboard piecesBB = color == WHITE ? whitePieces : blackPieces;
	bitboard currentPosition = 0x1;
	for (uint8_t i = 0; i < 64; i++)
	{
		if (piecesBB & currentPosition)
		{
			Piece currentPiece;
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
	pieceType type = PIECE_NONE;
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

bool Board::squareIsAttackedBy(square square, Color color)
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

void Board::executeMove(Move move) 
{
	square from = move.getFrom();
	square to = move.getTo();
	bitboard fromBB = 1ULL << from;
	bitboard toBB = 1ULL << to;
	bitboard fromToBB = fromBB | toBB;

	if (fromToBB & 0x1)
	{
		whiteLeftCastleAvailable = false;
	}
	else if (fromToBB & 0x80)
	{
		whiteRightCastleAvailable = false;
	}
	if (fromToBB & 0x100000000000000)
	{
		blackLeftCastleAvailable = false;
	}
	else if (fromToBB & 0x8000000000000000)
	{
		blackRightCastleAvailable = false;
	}

	bitboard *turnPiecesBB_p = turnColor == WHITE ? &whitePieces : &blackPieces;
	bitboard *otherPiecesBB_p = turnColor == WHITE ? &blackPieces : &whitePieces;

	*turnPiecesBB_p = (*turnPiecesBB_p & ~fromBB) | toBB;
	if (*otherPiecesBB_p & toBB)
	{
		*otherPiecesBB_p = *otherPiecesBB_p & ~toBB;

		pawns = pawns & ~fromToBB;
		knights = knights & ~fromToBB;
		bishops = bishops & ~fromToBB;
		rooks = rooks & ~fromToBB;
		queens = queens & ~fromToBB;
		kings = kings & ~fromToBB;

		removePieceFromList((Color)!turnColor, to);
	}

	Piece *piece = move.getPiece();
	piece->position = to;
	if (move.getType() == PROMOTION)
	{
		piece->type = move.getPromotion();
	}

	switch (piece->type)
	{
		case PAWN:
			pawns = pawns | toBB;
			if (toBB & (fromBB << 16 | fromBB >> 16))
			{
				enPassantSquare = turnColor == WHITE ? toBB >> 8 : toBB << 8;
			}
			break;
		case KNIGHT:
			knights = knights | toBB;
			break;
		case BISHOP:
			bishops = bishops | toBB;
			break;
		case ROOK:
			rooks = rooks | toBB;
			break;
		case QUEEN:
			queens = queens | toBB;
			break;
		case KING:
			kings = kings | toBB;
			if (turnColor == WHITE) 
			{
				whiteCastle();
				whiteKingSquare = to;
			}
			else
			{
				blackCastle();
				blackKingSquare = to;
			}
			break;
	}

	switch (move.getType())
	{
		case EN_PASSANT:
			if (turnColor == WHITE) 
			{
				removePieceFromList(BLACK, (square)(to - 8));
				*otherPiecesBB_p = *otherPiecesBB_p & ~(toBB >> 8);
			}
			else
			{
				removePieceFromList(WHITE, (square)(to + 8));
				*otherPiecesBB_p = *otherPiecesBB_p & ~(toBB << 8);
			}
			pawns = pawns & ~(toBB >> 8 | toBB << 8);
			break;
		case CASTLE_LEFT:
			*turnPiecesBB_p = (*turnPiecesBB_p & ~(toBB >> 2)) | (toBB << 1);
			rooks = (rooks & ~(toBB >> 2)) | (toBB << 1);
			movePieceFromList((Color)turnColor, (square)(to - 2), (square)(to + 1));
			break;
		case CASTLE_RIGHT:
			*turnPiecesBB_p = (*turnPiecesBB_p & ~(toBB << 1)) | (toBB >> 1);
			rooks = (rooks & ~(toBB << 1)) | (toBB >> 1);
			movePieceFromList((Color)turnColor, (square)(to + 1), (square)(to - 1));
			break;
	}

	turnColor = (Color)!turnColor;
}

void Board::movePieceFromList(Color color, square from, square to)
{
	std::vector<Piece> *list_p = color == WHITE ? &whitePiecesList : &blackPiecesList;

	auto it = std::find_if(list_p->begin(), list_p->end(), [from](Piece piece) {return piece.position == from; });
	if (it != list_p->end())
	{
		it->position = to;
	}
}

void Board::removePieceFromList(Color color, square square)
{
	std::vector<Piece> *list_p = color == WHITE ? &whitePiecesList : &blackPiecesList;
	auto it = std::find_if(list_p->begin(), list_p->end(), [square](Piece piece) {return piece.position == square; });
	int index = std::distance(list_p->begin(), it);
	list_p->erase(std::next(list_p->begin(), index));
}

bool Board::isMoveStrictlyLegal(Move move)
{
	moveType moveType = move.getType();
	if (moveType == CASTLE_LEFT || moveType == CASTLE_RIGHT)
		return true;

	square from = move.getFrom();
	square to = move.getTo();
	bitboard fromBB = 1ULL << from;
	bitboard toBB = 1ULL << to;
	bitboard fromToBB = fromBB | toBB;
	bitboard* turnPiecesBB_p = turnColor == WHITE ? &whitePieces : &blackPieces;
	bitboard* otherPiecesBB_p = turnColor == WHITE ? &blackPieces : &whitePieces;

	*turnPiecesBB_p = (*turnPiecesBB_p & ~fromBB) | toBB;
	*otherPiecesBB_p = *otherPiecesBB_p & ~toBB;

	pawns = pawns & ~fromToBB;
	knights = knights & ~fromToBB;
	bishops = bishops & ~fromToBB;
	rooks = rooks & ~fromToBB;
	queens = queens & ~fromToBB;
	kings = kings & ~fromToBB;

	Piece* piece = move.getPiece();

	switch (piece->type)
	{
	case PAWN:
		pawns = pawns | toBB;
		if (toBB & (fromBB << 16 | fromBB >> 16))
		{
			enPassantSquare = turnColor == WHITE ? toBB >> 8 : toBB << 8;
		}
		break;
	case KNIGHT:
		knights = knights | toBB;
		break;
	case BISHOP:
		bishops = bishops | toBB;
		break;
	case ROOK:
		rooks = rooks | toBB;
		break;
	case QUEEN:
		queens = queens | toBB;
		break;
	case KING:
		kings = kings | toBB;
		turnColor == WHITE ? whiteKingSquare = to : blackKingSquare = to;
		break;
	}


	if(moveType == EN_PASSANT)
	{
		//we can empty the squares above and below the to square, because when a pawn is moved two squares, the square it came from must be empty 
		*otherPiecesBB_p = *otherPiecesBB_p & ~(toBB >> 8 | toBB << 8);
		pawns = pawns & ~(toBB >> 8 | toBB << 8);
	}

	return !squareIsAttackedBy(turnColor == WHITE ? whiteKingSquare : blackKingSquare, (Color)!turnColor);
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

std::vector<Piece> &Board::getWhitePiecesList() 
{
	return whitePiecesList;
}
std::vector<Piece> &Board::getBlackPiecesList()
{
	return blackPiecesList;
}

Color Board::getTurnColor()
{
	return turnColor;
}

bitboard Board::getEnPassantSquare()
{
	return enPassantSquare;
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
