#include "engine.h"
#include <cmath>

std::vector<Move> getMoves(Board &board)
{
	std::vector<Move> moves;
	std::vector<Piece> &pieces = board.getTurnColor() == WHITE ? board.getWhitePiecesList() : board.getBlackPiecesList();
	/*for (Piece &piece : pieces)
	{
		std::vector<Move> pieceMoves = getMovesForPiece(board, piece);
		moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
	}*/
	for (uint8_t i = 0; i < pieces.size(); i++)
	{
		std::vector<Move> pieceMoves = getMovesForPiece(board, pieces[i], i);
		moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
	}
	return moves;
}

int getNodesForDepth(Board& board, int depth)
{
	std::vector<Move> moves = getMoves(board);
	if (depth > 1)
	{
		int nodes = 0;
		for (Move move : moves)
		{
			Board newBoard(board);
			newBoard.executeMove(move);
			nodes += getNodesForDepth(newBoard, depth - 1);
		}
		return nodes;
	}
	return moves.size();
}

int getNodesOfTypeForDepth(Board& board, int depth, moveType type) 
{
	std::vector<Move> moves = getMoves(board);
	int nodesOfType = 0;
	if (depth > 1)
	{
		for (Move move : moves)
		{
			Board newBoard(board);
			newBoard.executeMove(move);
			nodesOfType += getNodesOfTypeForDepth(newBoard, depth - 1, type);
		}
		return nodesOfType;
	}
	for (Move move : moves)
	{
		if (move.getType() == type) nodesOfType++;
	}
	return nodesOfType;
}


std::vector<Move> getMovesForPiece(const Board &board, Piece &piece, uint8_t pieceIndex)
{	switch (piece.type)
	{
		case PAWN:
			return getMovesForPawn(board, piece, pieceIndex);
		case KNIGHT:
			return getMovesForKnight(board, piece, pieceIndex);
		case BISHOP:
			return getMovesForBishop(board, piece, pieceIndex);
		case ROOK:
			return getMovesForRook(board, piece, pieceIndex);
		case QUEEN:
			return getMovesForQueen(board, piece, pieceIndex);
		case KING:
			return getMovesForKing(board, piece, pieceIndex);
	}
	throw;
}

std::vector<Move> getMovesForPawn(const Board &board, Piece &pawn, uint8_t pieceIndex)
{
	std::vector<Move> moves;
	bitboard position = 1ULL << pawn.position;

	direction dir = pawn.color == WHITE ? UP : DOWN;
	bitboard piecesOfOtherColor = pawn.color == WHITE ? board.getBlackPieces() : board.getWhitePieces();

	square targetSquare = (square)(pawn.position + dir);
	bitboard target = 1ULL << targetSquare;

	if (!(target & board.getAllPieces()))
	{
		//promotion
		if (target & RANK_8 || target & RANK_1)
		{
			Move promoteN(&pawn, pieceIndex, pawn.position, targetSquare, KNIGHT);

			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(promoteN))
			{
				Move promoteB(&pawn, pieceIndex, pawn.position, targetSquare, BISHOP);
				Move promoteR(&pawn, pieceIndex, pawn.position, targetSquare, ROOK);
				Move promoteQ(&pawn, pieceIndex, pawn.position, targetSquare, QUEEN);

				moves.push_back(promoteN);
				moves.push_back(promoteB);
				moves.push_back(promoteR);
				moves.push_back(promoteQ);
			}
		}
		//normal move forward
		else
		{
			Move move(&pawn, pieceIndex, pawn.position, targetSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}

		//double move forward
		if ((position & RANK_2) && pawn.color == WHITE || ((position & RANK_7) && pawn.color == BLACK))
		{
			targetSquare = (square)(pawn.position + 2 * dir);
			if (!((1ULL << targetSquare) & piecesOfOtherColor))
			{
				Move move(&pawn, pieceIndex, pawn.position, targetSquare);
				Board newBoard(board);
				if (newBoard.isMoveStrictlyLegal(move))
					moves.push_back(move);
			}
		}
	}
	//left capture
	if (!(position & FILE_A))
	{
		targetSquare = (square)(pawn.position + dir + LEFT);
		bitboard targetPosition = 1ULL << targetSquare;
		if (targetPosition & piecesOfOtherColor)
		{
			//promotion
			if (target & RANK_8 || target & RANK_1)
			{
				Move promoteN(&pawn, pieceIndex, pawn.position, targetSquare, KNIGHT);

				Board newBoard(board);
				if (newBoard.isMoveStrictlyLegal(promoteN))
				{
					Move promoteB(&pawn, pieceIndex, pawn.position, targetSquare, BISHOP);
					Move promoteR(&pawn, pieceIndex, pawn.position, targetSquare, ROOK);
					Move promoteQ(&pawn, pieceIndex, pawn.position, targetSquare, QUEEN);

					moves.push_back(promoteN);
					moves.push_back(promoteB);
					moves.push_back(promoteR);
					moves.push_back(promoteQ);
				}
			}
			//normal caputre
			else 
			{
				Move move(&pawn, pieceIndex, pawn.position, targetSquare);
				Board newBoard(board);
				if (newBoard.isMoveStrictlyLegal(move))
					moves.push_back(move);
			}
		}
		//en passant
		if (targetPosition & board.getEnPassantSquare())
		{
			Move move(&pawn, pieceIndex, pawn.position, targetSquare, EN_PASSANT);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
	}
	//right capture
	if (!(position & FILE_H))
	{
		targetSquare = (square)(pawn.position + dir + RIGHT);
		bitboard targetPosition = 1ULL << targetSquare;
		if (targetPosition & piecesOfOtherColor)
		{
			//promotion
			if (target & RANK_8 || target & RANK_1)
			{
				Move promoteN(&pawn, pieceIndex, pawn.position, targetSquare, KNIGHT);

				Board newBoard(board);
				if (newBoard.isMoveStrictlyLegal(promoteN))
				{
					Move promoteB(&pawn, pieceIndex, pawn.position, targetSquare, BISHOP);
					Move promoteR(&pawn, pieceIndex, pawn.position, targetSquare, ROOK);
					Move promoteQ(&pawn, pieceIndex, pawn.position, targetSquare, QUEEN);

					moves.push_back(promoteN);
					moves.push_back(promoteB);
					moves.push_back(promoteR);
					moves.push_back(promoteQ);
				}
			}
			//normal capture
			else
			{
				Move move(&pawn, pieceIndex, pawn.position, targetSquare);
				Board newBoard(board);
				if (newBoard.isMoveStrictlyLegal(move))
					moves.push_back(move);
			}
		}
		//en passant
		if (targetPosition & board.getEnPassantSquare())
		{
			Move move(&pawn, pieceIndex, pawn.position, targetSquare, EN_PASSANT);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
	}

	return moves;
}

std::vector<Move> getMovesForKnight(const Board &board, Piece &knight, uint8_t pieceIndex)
{
	std::vector<Move> moves;

	addMoveInDirection(moves, board, knight, pieceIndex, UP_UP_RIGHT);
	addMoveInDirection(moves, board, knight, pieceIndex, RIGHT_RIGHT_UP);
	addMoveInDirection(moves, board, knight, pieceIndex, RIGHT_RIGHT_DOWN);
	addMoveInDirection(moves, board, knight, pieceIndex, DOWN_DOWN_RIGHT);
	addMoveInDirection(moves, board, knight, pieceIndex, DOWN_DOWN_LEFT);
	addMoveInDirection(moves, board, knight, pieceIndex, LEFT_LEFT_DOWN);
	addMoveInDirection(moves, board, knight, pieceIndex, LEFT_LEFT_UP);
	addMoveInDirection(moves, board, knight, pieceIndex, UP_UP_LEFT);

	

	return moves;
}

std::vector<Move> getMovesForBishop(const Board &board, Piece &bishop, uint8_t pieceIndex)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, bishop, pieceIndex, RIGHT_UP);
	addSlidingMovesInDirection(moves, board, bishop, pieceIndex, RIGHT_DOWN);
	addSlidingMovesInDirection(moves, board, bishop, pieceIndex, LEFT_DOWN);
	addSlidingMovesInDirection(moves, board, bishop, pieceIndex, LEFT_UP);

	return moves;
}

std::vector<Move> getMovesForRook(const Board &board, Piece &rook, uint8_t pieceIndex)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, rook, pieceIndex, UP);
	addSlidingMovesInDirection(moves, board, rook, pieceIndex, RIGHT);
	addSlidingMovesInDirection(moves, board, rook, pieceIndex, DOWN);
	addSlidingMovesInDirection(moves, board, rook, pieceIndex, LEFT);

	return moves;
}

std::vector<Move> getMovesForQueen(const Board &board, Piece &queen, uint8_t pieceIndex)
{
	std::vector<Move> moves;

	addSlidingMovesInDirection(moves, board, queen, pieceIndex, UP);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, RIGHT);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, DOWN);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, LEFT);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, RIGHT_UP);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, RIGHT_DOWN);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, LEFT_DOWN);
	addSlidingMovesInDirection(moves, board, queen, pieceIndex, LEFT_UP);

	return moves;
}

std::vector<Move> getMovesForKing(const Board &board, Piece &king, uint8_t pieceIndex)
{
	std::vector<Move> moves;

	addMoveInDirection(moves, board, king, pieceIndex, UP);
	addMoveInDirection(moves, board, king, pieceIndex, RIGHT_UP);
	addMoveInDirection(moves, board, king, pieceIndex, RIGHT);
	addMoveInDirection(moves, board, king, pieceIndex, RIGHT_DOWN);
	addMoveInDirection(moves, board, king, pieceIndex, DOWN);
	addMoveInDirection(moves, board, king, pieceIndex, LEFT_DOWN);
	addMoveInDirection(moves, board, king, pieceIndex, LEFT);
	addMoveInDirection(moves, board, king, pieceIndex, LEFT_UP);

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
			Move move(&king, pieceIndex, king.position, c18, CASTLE_LEFT);
			moves.push_back(move);
		}

		if (rightCastleAvailable && !(freeSpacesRight & board.getAllPieces()) &&
			!board.squareIsAttackedBy(f18, (Color)!board.getTurnColor()) &&
			!board.squareIsAttackedBy(g18, (Color)!board.getTurnColor()))
		{
			Move move(&king, pieceIndex, king.position, g18, CASTLE_RIGHT);
			moves.push_back(move);
		}
	}

	return moves;
}

void addSlidingMovesInDirection(std::vector<Move>& moves, const Board &board, Piece &piece, uint8_t pieceIndex, direction dir)
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
			Move move(&piece, pieceIndex, piece.position, nextSquare);
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
			Move move(&piece, pieceIndex, piece.position, nextSquare);
			Board newBoard(board);
			if (newBoard.isMoveStrictlyLegal(move))
				moves.push_back(move);
		}
		isAtEdge = nextPosition & EDGES.at(dir);
	}
}



void addMoveInDirection(std::vector<Move>& moves, const Board &board, Piece &piece, uint8_t pieceIndex, direction dir)
{
	bitboard position = 1ULL << piece.position;
	bitboard piecesOfSameColor = piece.color == WHITE ? board.getWhitePieces() : board.getBlackPieces();

	square targetSquare = (square)(piece.position + dir);
	if (!((1ULL << piece.position) & EDGES.at(dir)) && !((1ULL << targetSquare) & piecesOfSameColor))
	{
		Move move(&piece, pieceIndex, piece.position, targetSquare);
		Board newBoard(board);
		if (newBoard.isMoveStrictlyLegal(move))
			moves.push_back(move);
	}
}
