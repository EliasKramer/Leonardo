#include "engine.h"
#include <cmath>

std::vector<Move> getMoves(Board board, color color)
{
	std::vector<Move> moves;
	std::vector<piece> pieces = color == WHITE ? board.getWhitePiecesList() : board.getBlackPiecesList();
	for (piece piece : pieces)
	{
		std::vector<Move> pieceMoves = getMovesForPiece(board, piece);
		moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
	}
	return moves;
}

std::vector<Move> getMoves(Board board, color color, int depth)
{
	std::vector<Move> moves = getMoves(board, color);
	if (depth != 1)
	{
		for (Move move : moves)
		{
			Board newBoard(board);
			//move.apply(newBoard);
			std::vector<Move> nextMoves = getMoves(newBoard, color == WHITE ? BLACK : WHITE, depth - 1);
			moves.insert(moves.end(), nextMoves.begin(), nextMoves.end());
		}
	}
	return moves;
}


std::vector<Move> getMovesForPiece(Board board, piece piece)
{
	std::vector<Move> moves;
	
	switch (piece.type)
	{
		case PAWN:
			moves = getMovesForPawn(board, piece);
			break;
		case KNIGHT:
			moves = getMovesForKnight(board, piece);
			break;
		case BISHOP:
			moves = getMovesForBishop(board, piece);
			break;
		case ROOK:
			moves = getMovesForRook(board, piece);
			break;
		case QUEEN:
			moves = getMovesForQueen(board, piece);
			break;
		case KING:
			moves = getMovesForKing(board, piece);
			break;
	}
	return moves;
}

std::vector<Move> getMovesForPawn(Board board, piece pawn)
{
	std::vector<Move> moves;
	map position = 1ULL << pawn.position;
	if (position & RANK_8 || position & RANK_1)
	{
		Move promoteN(pawn.position, KNIGHT);
		Move promoteB(pawn.position, BISHOP);
		Move promoteR(pawn.position, ROOK);
		Move promoteQ(pawn.position, QUEEN);

		moves.push_back(promoteN);
		moves.push_back(promoteB);
		moves.push_back(promoteR);
		moves.push_back(promoteQ);
	}
	else
	{
		direction dir = pawn.color == WHITE ? NORTH : SOUTH;
		map piecesOfOtherColor = pawn.color == WHITE ? board.getBlackPieces() : board.getWhitePieces();

		if (!(position << dir & board.getAllPieces()))
		{
			Move move(pawn.position, (square)(pawn.position + dir));
			moves.push_back(move);
			if (position & RANK_2 && !(position & 2 * dir))
			{
				Move move(pawn.position, (square)(pawn.position + 2 * dir));
				moves.push_back(move);
			}
		}
		if (!(position & FILE_A) && (position << (dir + WEST) & piecesOfOtherColor))
		{
			Move move(pawn.position, (square)(pawn.position + dir + WEST));
			moves.push_back(move);
		}
		if (!(position & FILE_H) && (position << (dir + EAST) & piecesOfOtherColor))
		{
			Move move(pawn.position, (square)(pawn.position + dir + EAST));
			moves.push_back(move);
		}

	}
	return moves;
}

std::vector<Move> getMovesForKnight(Board board, piece knight)
{
	map position = 1ULL << knight.position;
	std::vector<Move> moves;
	return moves;
}

std::vector<Move> getMovesForBishop(Board board, piece bishop)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, bishop, NORTH_EAST);
	addSlidingMovesInDirection(moves, board, bishop, SOUTH_EAST);
	addSlidingMovesInDirection(moves, board, bishop, SOUTH_WEST);
	addSlidingMovesInDirection(moves, board, bishop, NORTH_WEST);

	return moves;
}

std::vector<Move> getMovesForRook(Board board, piece rook)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, rook, NORTH);
	addSlidingMovesInDirection(moves, board, rook, EAST);
	addSlidingMovesInDirection(moves, board, rook, SOUTH);
	addSlidingMovesInDirection(moves, board, rook, WEST);

	return moves;
}

std::vector<Move> getMovesForQueen(Board board, piece queen)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, queen, NORTH);
	addSlidingMovesInDirection(moves, board, queen, EAST);
	addSlidingMovesInDirection(moves, board, queen, SOUTH);
	addSlidingMovesInDirection(moves, board, queen, WEST);
	addSlidingMovesInDirection(moves, board, queen, NORTH_EAST);
	addSlidingMovesInDirection(moves, board, queen, SOUTH_EAST);
	addSlidingMovesInDirection(moves, board, queen, SOUTH_WEST);
	addSlidingMovesInDirection(moves, board, queen, NORTH_WEST);

	return moves;
}

void addSlidingMovesInDirection(std::vector<Move>& moves, Board board, piece piece, direction dir)
{
	map position = 1ULL << piece.position;
	map piecesOfOtherColor = piece.color == WHITE ? board.getBlackPieces() : board.getWhitePieces();
	map nextPosition = position;
	square nextSquare = piece.position;

	bool isAtEdge = position & EDGES.at(dir);
	while (!isAtEdge)
	{
		nextSquare = (square)(nextSquare + dir);
		nextPosition = 1ULL << nextSquare;
		if (nextPosition & piecesOfOtherColor)
		{
			Move move(piece.position, nextSquare);
			moves.push_back(move);
			break;
		}
		else if (nextPosition & board.getAllPieces())
		{
			break;
		}
		else
		{
			Move move(piece.position, nextSquare);
			moves.push_back(move);
		}
		isAtEdge = nextPosition & EDGES.at(dir);
	}
}


std::vector<Move> getMovesForKing(Board board, piece king)
{
	map position = 1ULL << king.position;
	std::vector<Move> moves;
	return moves;
}