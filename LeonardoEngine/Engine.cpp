#include "engine.h"
#include <cmath>

std::vector<Move> getMoves(Board board)
{
	std::vector<Move> moves;
	std::vector<Piece> &pieces = board.getTurnColor() == WHITE ? board.getWhitePiecesList() : board.getBlackPiecesList();
	for (Piece &piece : pieces)
	{
		std::vector<Move> pieceMoves = getMovesForPiece(board, piece);
		moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
	}
	return moves;
}

std::vector<Move> getMoves(Board board, int depth)
{
	std::vector<Move> moves = getMoves(board);
	if (depth != 1)
	{
		int size = moves.size();
		int i = 0;
		//for (Move move : moves)
		for(auto it = moves.begin(); i < size ; ++it)
		{
			Board newBoard(board);
			//newBoard.executeMove(move);
			newBoard.executeMove(*it);
			std::vector<Move> nextMoves = getMoves(newBoard, depth - 1);
			moves.insert(moves.end(), nextMoves.begin(), nextMoves.end());

			it = moves.begin();
			std::advance(it, i);
			i++;
		}
	}
	return moves;
}


std::vector<Move> getMovesForPiece(Board board, Piece &piece)
{	switch (piece.type)
	{
		case PAWN:
			return getMovesForPawn(board, piece);
		case KNIGHT:
			return getMovesForKnight(board, piece);
		case BISHOP:
			return getMovesForBishop(board, piece);
		case ROOK:
			return getMovesForRook(board, piece);
		case QUEEN:
			return getMovesForQueen(board, piece);
		case KING:
			return getMovesForKing(board, piece);
	}
	throw;
}

std::vector<Move> getMovesForPawn(Board board, Piece &pawn)
{
	std::vector<Move> moves;
	bitboard position = 1ULL << pawn.position;

	direction dir = pawn.color == WHITE ? UP : DOWN;
	bitboard piecesOfOtherColor = pawn.color == WHITE ? board.getBlackPieces() : board.getWhitePieces();

	square targetSquare = (square)(pawn.position + dir);
	bitboard target = 1ULL << targetSquare;

	if (!(target & board.getAllPieces()))
	{
		if (target & RANK_8 || target & RANK_1)
		{
			Move promoteN(&pawn, pawn.position, targetSquare, KNIGHT);

			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(promoteN))
			{
				Move promoteB(&pawn, pawn.position, targetSquare, BISHOP);
				Move promoteR(&pawn, pawn.position, targetSquare, ROOK);
				Move promoteQ(&pawn, pawn.position, targetSquare, QUEEN);

				moves.push_back(promoteN);
				moves.push_back(promoteB);
				moves.push_back(promoteR);
				moves.push_back(promoteQ);
			}
		}
		else
		{
			Move move(&pawn, pawn.position, targetSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}

		if (position & RANK_2 || position & RANK_7)
		{
			targetSquare = (square)(pawn.position + 2 * dir);
			if (!((1ULL << targetSquare) & piecesOfOtherColor))
			{
				Move move(&pawn, pawn.position, targetSquare);
				Board newBoard(board);
				if (newBoard.isMoveStrictlyLegal(move))
					moves.push_back(move);
			}
		}
	}
	if (!(position & FILE_A))
	{
		targetSquare = (square)(pawn.position + dir + LEFT);
		bitboard targetPosition = 1ULL << targetSquare;
		if (targetPosition & piecesOfOtherColor)
		{
			Move move(&pawn, pawn.position, targetSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
		if (targetPosition & board.getEnPassantSquare())
		{
			Move move(&pawn, pawn.position, targetSquare, EN_PASSANT);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
	}
	if (!(position & FILE_H))
	{
		targetSquare = (square)(pawn.position + dir + RIGHT);
		bitboard targetPosition = 1ULL << targetSquare;
		if (targetPosition & piecesOfOtherColor)
		{
			Move move(&pawn, pawn.position, targetSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
		if (targetPosition & board.getEnPassantSquare())
		{
			Move move(&pawn, pawn.position, targetSquare, EN_PASSANT);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
	}

	return moves;
}

std::vector<Move> getMovesForKnight(Board board, Piece &knight)
{
	std::vector<Move> moves;

	addMoveInDirection(moves, board, knight, UP_UP_RIGHT);
	addMoveInDirection(moves, board, knight, RIGHT_RIGHT_UP);
	addMoveInDirection(moves, board, knight, RIGHT_RIGHT_DOWN);
	addMoveInDirection(moves, board, knight, DOWN_DOWN_RIGHT);
	addMoveInDirection(moves, board, knight, DOWN_DOWN_LEFT);
	addMoveInDirection(moves, board, knight, LEFT_LEFT_DOWN);
	addMoveInDirection(moves, board, knight, LEFT_LEFT_UP);
	addMoveInDirection(moves, board, knight, UP_UP_LEFT);

	

	return moves;
}

std::vector<Move> getMovesForBishop(Board board, Piece &bishop)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, bishop, RIGHT_UP);
	addSlidingMovesInDirection(moves, board, bishop, RIGHT_DOWN);
	addSlidingMovesInDirection(moves, board, bishop, LEFT_DOWN);
	addSlidingMovesInDirection(moves, board, bishop, LEFT_UP);

	return moves;
}

std::vector<Move> getMovesForRook(Board board, Piece &rook)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, rook, UP);
	addSlidingMovesInDirection(moves, board, rook, RIGHT);
	addSlidingMovesInDirection(moves, board, rook, DOWN);
	addSlidingMovesInDirection(moves, board, rook, LEFT);

	return moves;
}

std::vector<Move> getMovesForQueen(Board board, Piece &queen)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, queen, UP);
	addSlidingMovesInDirection(moves, board, queen, RIGHT);
	addSlidingMovesInDirection(moves, board, queen, DOWN);
	addSlidingMovesInDirection(moves, board, queen, LEFT);
	addSlidingMovesInDirection(moves, board, queen, RIGHT_UP);
	addSlidingMovesInDirection(moves, board, queen, RIGHT_DOWN);
	addSlidingMovesInDirection(moves, board, queen, LEFT_DOWN);
	addSlidingMovesInDirection(moves, board, queen, LEFT_UP);

	return moves;
}

std::vector<Move> getMovesForKing(Board board, Piece &king)
{
	std::vector<Move> moves;

	addMoveInDirection(moves, board, king, UP);
	addMoveInDirection(moves, board, king, RIGHT_UP);
	addMoveInDirection(moves, board, king, RIGHT);
	addMoveInDirection(moves, board, king, RIGHT_DOWN);
	addMoveInDirection(moves, board, king, DOWN);
	addMoveInDirection(moves, board, king, LEFT_DOWN);
	addMoveInDirection(moves, board, king, LEFT);
	addMoveInDirection(moves, board, king, LEFT_UP);

	bool leftCastleAvailable = king.color == WHITE ? board.getWhiteLeftCastleAvailable() : board.getBlackLeftCastleAvailable();
	bool rightCastleAvailable = king.color == WHITE ? board.getWhiteRightCastleAvailable() : board.getBlackRightCastleAvailable();
	bitboard freeSpacesLeft = king.color == WHITE ? 0xe : 0xe00000000000000;
	bitboard freeSpacesRight = king.color == WHITE ? 0x60 : 0x6000000000000000;

	if (((leftCastleAvailable && !(freeSpacesLeft & board.getAllPieces())) || (rightCastleAvailable && !(freeSpacesRight & board.getAllPieces()))) && !board.squareIsAttackedBy(king.position, (Color)!board.getTurnColor()))
	{
		square c18 = king.color == WHITE ? C1 : C8;
		square d18 = king.color == WHITE ? D1 : D8;

		square f18 = king.color == WHITE ? F1 : F8;
		square g18 = king.color == WHITE ? G1 : G8;

		if (leftCastleAvailable && !(freeSpacesLeft & board.getAllPieces()) &&
			!board.squareIsAttackedBy(c18, (Color)!board.getTurnColor()) &&
			!board.squareIsAttackedBy(d18, (Color)!board.getTurnColor()))
		{
			Move move(&king, king.position, c18, CASTLE_LEFT);
			moves.push_back(move);
		}

		if (rightCastleAvailable && !(freeSpacesRight & board.getAllPieces()) &&
			!board.squareIsAttackedBy(f18, (Color)!board.getTurnColor()) &&
			!board.squareIsAttackedBy(g18, (Color)!board.getTurnColor()))
		{
			Move move(&king, king.position, g18, CASTLE_RIGHT);
			moves.push_back(move);
		}
	}

	return moves;
}

void addSlidingMovesInDirection(std::vector<Move>& moves, Board board, Piece &piece, direction dir)
{
	bitboard position = 1ULL << piece.position;
	bitboard piecesOfOtherColor = piece.color == WHITE ? board.getBlackPieces() : board.getWhitePieces();
	bitboard nextPosition = position;
	square nextSquare = piece.position;

	bool isAtEdge = position & EDGES.at(dir);
	while (!isAtEdge)
	{
		nextSquare = (square)(nextSquare + dir);
		nextPosition = 1ULL << nextSquare;
		if (nextPosition & piecesOfOtherColor)
		{
			Move move(&piece, piece.position, nextSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
			break;
		}
		else if (nextPosition & board.getAllPieces())
		{
			break;
		}
		else
		{
			Move move(&piece, piece.position, nextSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
		isAtEdge = nextPosition & EDGES.at(dir);
	}
}



void addMoveInDirection(std::vector<Move>& moves, Board board, Piece &piece, direction dir)
{
	bitboard position = 1ULL << piece.position;
	bitboard piecesOfSameColor = piece.color == WHITE ? board.getWhitePieces() : board.getBlackPieces();

	square targetSquare = (square)(piece.position + dir);
	if (!((1ULL << piece.position) & EDGES.at(dir)) && !((1ULL << targetSquare) & piecesOfSameColor))
	{
		Move move(&piece, piece.position, targetSquare);
		Board newBoard(board);
		if (newBoard.isMoveStrictlyLegal(move))
			moves.push_back(move);
	}
}
