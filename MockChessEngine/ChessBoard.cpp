#include "ChessBoard.h"
#include <iostream>
#include <random>
bool ChessBoard::destinationIsSameColor(Square start, Direction direction, ChessColor color) const
{
	int newPos = (start + direction);

	if (newPos < A1 || newPos > H8)
	{
		return false;
	}

	return positionIsSameColor((Square)newPos, color);
}

bool ChessBoard::positionIsSameColor(Square pos, ChessColor color) const
{
	return (_board.PiecesOfColor[color] & BB_SQUARE[pos]) != 0;
}

void ChessBoard::addIfDestinationIsValid(UniqueMoveList& moves, Square start, Direction dir) const
{
	if (destinationIsOnBoard(start, dir))
	{
		if (!destinationIsSameColor(start, dir, _currentTurnColor))
		{
			moves.push_back(std::make_unique<Move>(start, (Square)(start + dir)));
		}
	}
}

void ChessBoard::addIfDestinationIsColor(
	UniqueMoveList& moves,
	Square start,
	Direction dir,
	ChessColor color) const
{
	if (destinationIsOnBoard(start, dir))
	{
		Square newPos = (Square)(start + dir);
		if (positionIsSameColor(newPos, color))
		{
			moves.push_back(std::make_unique<Move>(start, newPos));
		}
	}
}

UniqueMoveList ChessBoard::getAllPseudoLegalMoves() const
{
	UniqueMoveList moveList;

	getCastlingMoves(moveList);

	getPawnMoves(moveList);
	getKnightMoves(moveList);
	getBishopMoves(moveList);
	getRookMoves(moveList);
	getQueenMoves(moveList);
	getKingMoves(moveList);

	getEnPassantMove(moveList);

	return moveList;
}

void ChessBoard::getPawnMoves(UniqueMoveList& moves) const
{
	Direction forward = getForwardForColor(_currentTurnColor);
	BitBoard startRank = _currentTurnColor == White ? RANK_2 : RANK_7;

	for (uint8_t currSquareIdx = A1; currSquareIdx <= H8; currSquareIdx++)
	{
		Square currSquare = (Square)currSquareIdx;
		BitBoard piecePosBB = BB_SQUARE[currSquareIdx];

		//if the current square is a pawn and the same color
		if ((piecePosBB &
			_board.PiecesOfType[Pawn] &
			_board.PiecesOfColor[_currentTurnColor]) != 0ULL)
		{
			//one move forward
			if (destinationIsOnBoard(currSquare, forward))
			{
				Square forwardPos = (Square)(currSquare + forward);
				BitBoard forwardPosBB = BB_SQUARE[forwardPos];

				if ((forwardPosBB & _board.AllPieces) == 0ULL)
				{
					addPawnMove(moves, currSquare, forwardPos);
					//moves.push_back(std::make_unique<Move>(currSquare, forwardPos));
					//TODO
					//if move ends on promotion rank it should be a promotion move

					//two moves forward (only possible if one move is possible
					Square doubleForward = (Square)(forwardPos + forward);
					if ((piecePosBB & startRank) != 0 &&
						destinationIsOnBoard(forwardPos, forward) &&
						(BB_SQUARE[doubleForward] & _board.AllPieces) == 0ULL)
					{
						moves.push_back(std::make_unique<Move>(currSquare, doubleForward));
					}
				}
			}

			//taking diagonal checks
			//this loop checks one time left and one time right
			Direction leftAndRight[2] = { WEST, EAST };
			for (int i = 0; i < 2; i++)
			{
				Direction direction = (Direction)(leftAndRight[i] + forward);
				ChessColor opponentColor = getOppositeColor(_currentTurnColor);

				if (destinationIsSameColor(currSquare, direction, opponentColor)
					&& destinationIsOnBoard(currSquare, direction))
				{
					addPawnMove(moves, currSquare, (Square)(currSquare + direction));
				}
			}
		}
	}
}

void ChessBoard::getKnightMoves(UniqueMoveList& moves) const
{
	for (uint8_t currSquareIdx = A1; currSquareIdx <= H8; currSquareIdx++)
	{
		if ((BB_SQUARE[currSquareIdx] &
			_board.PiecesOfType[Knight] &
			_board.PiecesOfColor[_currentTurnColor]) != 0ULL)
		{
			addIfDestinationIsValid(moves, (Square)currSquareIdx, NORTH_NORTH_EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, EAST_NORTH_EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, EAST_SOUTH_EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, SOUTH_SOUTH_EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, SOUTH_SOUTH_WEST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, WEST_SOUTH_WEST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, WEST_NORTH_WEST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, NORTH_NORTH_WEST);
		}
	}
}

void ChessBoard::getBishopMoves(UniqueMoveList& moves) const
{
	const int numberOfDirections = 4;
	Direction directions[numberOfDirections] =
	{ NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

	for (uint8_t currSquareIdx = A1; currSquareIdx <= H8; currSquareIdx++)
	{
		if ((BB_SQUARE[currSquareIdx] &
			_board.PiecesOfType[Bishop] &
			_board.PiecesOfColor[_currentTurnColor]) != 0ULL)
		{
			addRayMoves(moves, (Square)currSquareIdx, directions, numberOfDirections);
		}
	}

}

void ChessBoard::getRookMoves(UniqueMoveList& moves) const
{
	const int numberOfDirections = 4;
	Direction directions[numberOfDirections] =
	{ NORTH, EAST, SOUTH, WEST };

	for (uint8_t currSquareIdx = A1; currSquareIdx <= H8; currSquareIdx++)
	{
		if ((BB_SQUARE[currSquareIdx] &
			_board.PiecesOfType[Rook] &
			_board.PiecesOfColor[_currentTurnColor]) != 0ULL)
		{
			addRayMoves(moves, (Square)currSquareIdx, directions, numberOfDirections);
		}
	}
}

void ChessBoard::getQueenMoves(UniqueMoveList& moves) const
{
	const int numberOfDirections = 8;
	Direction directions[numberOfDirections] =
	{ NORTH, EAST, SOUTH, WEST, NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

	for (uint8_t currSquareIdx = A1; currSquareIdx <= H8; currSquareIdx++)
	{
		if ((BB_SQUARE[currSquareIdx] &
			_board.PiecesOfType[Queen] &
			_board.PiecesOfColor[_currentTurnColor]) != 0ULL)
		{
			addRayMoves(moves, (Square)currSquareIdx, directions, numberOfDirections);
		}
	}
}

void ChessBoard::getKingMoves(UniqueMoveList& moves) const
{
	for (uint8_t currSquareIdx = A1; currSquareIdx <= H8; currSquareIdx++)
	{
		if ((BB_SQUARE[currSquareIdx] &
			_board.PiecesOfType[King] &
			_board.PiecesOfColor[_currentTurnColor]) != 0ULL)
		{
			addIfDestinationIsValid(moves, (Square)currSquareIdx, NORTH);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, NORTH_EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, SOUTH_EAST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, SOUTH);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, SOUTH_WEST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, WEST);
			addIfDestinationIsValid(moves, (Square)currSquareIdx, NORTH_WEST);
		}
	}
}

void ChessBoard::addPawnMove(UniqueMoveList& moves, Square start, Square dest) const
{
	BitBoard promotionRank = _currentTurnColor == White ? RANK_8 : RANK_1;

	if ((BB_SQUARE[dest] & promotionRank) != 0ULL)
	{
		moves.push_back(std::make_unique<MovePromote>(start, dest, ChessPiece(_currentTurnColor, Queen)));
		moves.push_back(std::make_unique<MovePromote>(start, dest, ChessPiece(_currentTurnColor, Rook)));
		moves.push_back(std::make_unique<MovePromote>(start, dest, ChessPiece(_currentTurnColor, Bishop)));
		moves.push_back(std::make_unique<MovePromote>(start, dest, ChessPiece(_currentTurnColor, Knight)));
	}
	else
	{
		moves.push_back(std::make_unique<Move>(start, dest));
	}
}

void ChessBoard::getCastlingMoves(UniqueMoveList& moves) const
{
	//can improve performance -> 
	//combine all fields in a bitboard and check for the non-sliding pieces
	//if the can take at these positions
	for (int castlingIdx = 0; castlingIdx < 2; castlingIdx++)
	{
		CastlingType currCastlingType = (CastlingType)castlingIdx;

		if (_canCastle[_currentTurnColor][currCastlingType])
		{
			bool castlingAllowed = true;
			for (int i = 0; i < 3; i++)
			{
				Square currSquareToCheck =
					SQUARES_FOR_KING_CASTLING[_currentTurnColor][currCastlingType][i];

				//square should not be attacked or should not have any piece there
				if (fieldIsUnderAttack(currSquareToCheck) ||
					(i != 0 && (_board.AllPieces & BB_SQUARE[currSquareToCheck]) != 0ULL))
				{
					castlingAllowed = false;
				}
			}
			//can be done better, but has to do for now
			if (currCastlingType == CastleLong)
			{
				//square that can be in check, but has to have no piece on it
				Square passingSquare = _currentTurnColor == White ? B1 : B8;
				if ((_board.AllPieces & BB_SQUARE[passingSquare]) != 0ULL)
				{
					castlingAllowed = false;
				}
			}

			if (castlingAllowed)
			{
				moves.push_back(std::make_unique<MoveCastle>(_currentTurnColor, currCastlingType));
			}
		}
	}
}

void ChessBoard::getEnPassantMove(UniqueMoveList& moves) const
{
	if (_enPassantSquare == SQUARE_NONE)
	{
		return;
	}

	Direction backwards = getBackwardForColor(_currentTurnColor);

	Direction directionsWhereOwnPawnCouldBe[2] =
	{ (Direction)(backwards + EAST), (Direction)(backwards + WEST) };

	for (int i = 0; i < 2; i++)
	{
		if (destinationIsOnBoard(_enPassantSquare, directionsWhereOwnPawnCouldBe[i]))
		{
			Square ownPawnPos = (Square)(_enPassantSquare + directionsWhereOwnPawnCouldBe[i]);
			if ((BB_SQUARE[ownPawnPos] &
				_board.PiecesOfColor[_currentTurnColor] &
				_board.PiecesOfType[Pawn]) != 0ULL)
			{
				Square pawnPosToDelete = (Square)(_enPassantSquare + backwards);
				moves.push_back(std::make_unique<MoveEnPassant>(
					ownPawnPos, _enPassantSquare,
					pawnPosToDelete));
			}
		}
	}
}

void ChessBoard::addRayMoves(
	UniqueMoveList& moves,
	Square start,
	Direction directions[],
	int numberOfDirections) const
{
	ChessColor opponentColor = getOppositeColor(_currentTurnColor);

	for (int i = 0; i < numberOfDirections; i++)
	{
		Direction currentDirection = directions[i];

		Square currentSquare = start;
		while (true)
		{
			//if the new position would be on the board
			if (destinationIsOnBoard(currentSquare, currentDirection))
			{
				//set the new position
				currentSquare = (Square)(currentSquare + currentDirection);

				//if the new position is the same color as the piece to check
				if (positionIsSameColor(currentSquare, _currentTurnColor))
				{
					//you cannot go on a square where a piece of the same color is
					break;
				}
				else if (positionIsSameColor(currentSquare, opponentColor))
				{
					//if an opponent is on the new field you can take him,
					//but cannot continue after that (you cannot jump over opponents)
					moves.push_back(std::make_unique<Move>(start, currentSquare));
							 
					break;
				}
				else {
					//if there is no piece at the new position you can move there.
					moves.push_back(std::make_unique<Move>(start, currentSquare));
				}
			}
			else {
				//if the new position is not on the board, the search should not continue
				break;
			}
		}
	}
}

bool ChessBoard::fieldIsUnderAttack(Square pos, BitBoard moveBB) const
{
	//is only used for king checks and castling, so no en passant implemented

	ChessColor opponentColor = getOppositeColor(_currentTurnColor);

	BitBoard newCurrentColorBB = _board.PiecesOfColor[_currentTurnColor] ^ moveBB;

	BitBoard opponentColorBB = (_board.PiecesOfColor[opponentColor] & (~moveBB));

	BitBoard knightBB = _board.PiecesOfType[Knight];
	BitBoard pawnsBB = _board.PiecesOfType[Pawn];
	BitBoard kingsBB = _board.PiecesOfType[King];

	//gets attacked by knight
	if ((KNIGHT_ATTACK_BB[pos] &
		opponentColorBB &
		knightBB) != 0ULL)
	{
		return true;
	}

	//gets attacked by pawn
	//it is handled like the current field is a pawn, 
	//because if the current field has a pawn on its attacking squares, 
	//it can also be attacked by the opponent pawn
	if ((PAWN_ATTACK_BB[_currentTurnColor][pos] &
		opponentColorBB &
		pawnsBB) != 0ULL)
	{
		return true;
	}

	//field gets attacked by king
	if ((KING_ATTACKS_BB[pos] &
		opponentColorBB &
		kingsBB) != 0ULL)
	{
		return true;
	}

	//raycast in every direction and check for queen, rook or bishop attacks
	return fieldGetsAttackedBySlidingPiece(pos, moveBB);
}

bool ChessBoard::fieldGetsAttackedBySlidingPiece(Square pos, BitBoard moveBB) const
{
	ChessColor opponentColor = getOppositeColor(_currentTurnColor);

	BitBoard queenBB = _board.PiecesOfType[Queen];
	BitBoard rookBB = _board.PiecesOfType[Rook];
	BitBoard bishopBB = _board.PiecesOfType[Bishop];

	BitBoard newCurrentColorBB = _board.PiecesOfColor[_currentTurnColor] ^ moveBB;

	BitBoard moveBBWithoutStart = moveBB & ~_board.PiecesOfColor[_currentTurnColor];

	//if there are 2 destination fields, it is an en passant move
	if (((moveBBWithoutStart & _board.PiecesOfColor[opponentColor]) != 0ULL) &&
		((moveBBWithoutStart & ~_board.PiecesOfColor[opponentColor]) != 0ULL))
	{
		newCurrentColorBB &= ~_board.PiecesOfColor[opponentColor];
	}

	BitBoard opponentColorBB = (_board.PiecesOfColor[opponentColor] & (~moveBB));

	for (int i = 0; i < 8; i++)
	{
		Direction currentDirection = ALL_SLIDING_DIRECTIONS[i];

		Square currentSquare = pos;
		while (true)
		{
			//if the new position would be on the board
			if (destinationIsOnBoard(currentSquare, currentDirection))
			{
				//set the new position
				currentSquare = (Square)(currentSquare + currentDirection);

				//if the new position is the same color as the piece to check
				if (squareOverlapsWithBB(currentSquare, newCurrentColorBB))
				{
					//you cannot go on a square where a piece of the same color is
					//your piece blocks the sliding attack

					break;
				}
				else if (squareOverlapsWithBB(currentSquare, opponentColorBB))
				{
					BitBoard currPosBB = BB_SQUARE[currentSquare];

					bool currDirIsDiagonal = i > 3;

					//found piece is a queen -> will attack no matter in which direction you are sliding
					if ((currPosBB & queenBB) != 0ULL)
					{
						//is queen
						return true;
					}
					//if found piece is a rook or a bishop, sliding direction matters
					else if (currDirIsDiagonal)
					{
						if ((currPosBB & bishopBB) != 0ULL)
						{
							//is bishop
							return true;
						}
					}
					else
					{
						if ((currPosBB & rookBB) != 0ULL)
						{
							//is rook
							return true;
						}
					}
					//if the found piece is an enemy, but no sliding piece then it does not attack you
					break;
				}
			}
			else
			{
				//if the new position is not on the board, the search should not continue
				break;
			}
		}
	}
	return false;
}

bool ChessBoard::moveIsLegal(const std::unique_ptr<Move>& move) const
{
	//if move is castle -> return true
	if (dynamic_cast<MoveCastle*>(move.get()))
	{
		//because a castling move has been checked to be legal before
		return true;
	}

	Square start = move.get()->getStart();
	Square dest = move.get()->getDestination();

	BitBoard BBforNextMove = move.get()->getBBWithMoveDone();

	Square kingPos = _board.KingPos[_currentTurnColor];

	return start == kingPos ?
		!fieldIsUnderAttack(dest, BBforNextMove) :
		!fieldIsUnderAttack(kingPos, BBforNextMove);
}

bool ChessBoard::isCaptureMove(const std::unique_ptr<Move>& move) const
{
	if (dynamic_cast<MoveEnPassant*>(move.get()))
	{
		return true;
	}

	if (dynamic_cast<MoveCastle*>(move.get()))
	{
		return false;
	}

	BitBoard oppositeColorBB = _board.PiecesOfColor[getOppositeColor(_currentTurnColor)];
	BitBoard potentialCaptureField = BB_SQUARE[move.get()->getDestination()];

	return bitboardsOverlap(oppositeColorBB, potentialCaptureField);
}

void ChessBoard::udpateCastlingRightsAfterMove(const Move& m)
{
	BitBoard start = BB_SQUARE[m.getStart()];
	BitBoard dest = BB_SQUARE[m.getDestination()];
	if (bitboardsOverlap(start, SQUARES_EFFECTED_BY_CASTLING_BB) ||
		bitboardsOverlap(dest, SQUARES_EFFECTED_BY_CASTLING_BB))
	{

		if ((KINGFILE & BACKRANK[_currentTurnColor] & start) != 0)
		{
			_canCastle[_currentTurnColor][CastleLong] = false;
			_canCastle[_currentTurnColor][CastleShort] = false;
		}

		for (int i = 0; i < 2; i++)
		{
			ChessColor currCol = (ChessColor)i;
			for (int j = 0; j < 2; j++)
			{
				CastlingType currCastleType = (CastlingType)j;
				BitBoard fieldToCheck = BACKRANK[currCol] & CASTLINGSIDE[currCastleType];
				if (bitboardsOverlap(start, fieldToCheck) ||
					bitboardsOverlap(dest, fieldToCheck))
				{
					_canCastle[currCol][currCastleType] = false;
				}
			}
		}
	}
}

void ChessBoard::updateEnPassantRightsAfterMove(const Move& m)
{
	_enPassantSquare = SQUARE_NONE;
	Square start = m.getStart();
	if (squareOverlapsWithBB(start, _board.PiecesOfType[Pawn]))
	{
		Square dest = m.getDestination();
		BitBoard startRankForDoubleMove = _currentTurnColor == White ? RANK_2 : RANK_7;
		BitBoard destRankForDoubleMove = _currentTurnColor == White ? RANK_4 : RANK_5;

		if (squareOverlapsWithBB(start, startRankForDoubleMove) &&
			squareOverlapsWithBB(dest, destRankForDoubleMove))
		{
			_enPassantSquare = (Square)(start + getForwardForColor(_currentTurnColor));
		}
	}
}

void ChessBoard::update50MoveRule(const Move& m)
{
	BitBoard startBB = BB_SQUARE[m.getStart()];
	BitBoard destBB = BB_SQUARE[m.getDestination()];
	ChessColor opponentColor = getOppositeColor(_currentTurnColor);

	if (bitboardsOverlap(startBB, _board.PiecesOfType[Pawn]) ||
		bitboardsOverlap(destBB, _board.PiecesOfColor[opponentColor]))
	{
		_halfMoveClock = 0;
	}
	else
	{
		_halfMoveClock++;
	}
}

bool ChessBoard::insufficientMaterialCheck() const
{
	//if any of these pieces are on the board, the game can be won
	if (_board.PiecesOfType[Queen] != 0 ||
		_board.PiecesOfType[Rook] != 0 ||
		_board.PiecesOfType[Pawn] != 0)
	{
		return false;
	}

	if (_board.PiecesOfType[Bishop] == 0 &&
		_board.PiecesOfType[Knight] == 0)
	{
		return true;
	}

	//TODO
	//if only one bishop or one knight is on the board, the game cant be won

	return false;
}

char ChessBoard::getPieceCharAt(Square pos) const
{
	BitBoard posBB = BB_SQUARE[pos];

	if (bitboardsOverlap(posBB, ~_board.AllPieces))
	{
		return ' ';
	}

	char pieceChar = '~';

	for (int i = 0; i < NUMBER_OF_DIFFERENT_PIECE_TYPES; i++)
	{
		PieceType currType = (PieceType)i;

		if (bitboardsOverlap(_board.PiecesOfType[currType], posBB))
		{
			pieceChar = PIECETYPE_CHAR[currType];
		}
	}

	if (pieceChar == '~')
	{
		std::cout
			<< "Could not find PieceType on pos" << pos <<
			" in the Chessboard configuration\n"
			<< getString() << "\n";

		return ' ';
	}

	if (bitboardsOverlap(_board.PiecesOfColor[Black], posBB))
	{
		pieceChar = charToLower(pieceChar);
	}

	return pieceChar;
}


ChessBoard::ChessBoard(const std::string& given_fen_code)
{
	//set all castling rights to false at the beginning
	for (int i = 0; i < 2; i++)
	{
		ChessColor currColor = (ChessColor)i;
		for (int j = 0; j < 2; j++)
		{
			CastlingType currCastleType = (CastlingType)j;
			_canCastle[currColor][currCastleType] = false;
		}
	}

	std::string prefixForErrors = "ERROR The given FEN Code is invalid. ";

	std::vector<std::string> split_fen_code = splitString(given_fen_code, ' ');
	if (split_fen_code.size() != 6)
	{
		throw prefixForErrors + "It had not the right size";
	}

	//the implementation of the iteration will be a bit weird,
	//since index 0 is a1 and in the fen code it is a8

	int currRankStartIdx = 56;
	int currFile = 0;

	//fill the board
	for (int currStringIdx = 0;
		currStringIdx < split_fen_code[0].size();
		currStringIdx++)
	{
		char currChar = given_fen_code[currStringIdx];

		int boardPos = currRankStartIdx + currFile;

		if (currChar <= '8' && currChar >= '1')
		{
			int fieldsToSkip = currChar - '0';
			currFile += fieldsToSkip;
		}
		else if (currChar == '/')
		{
			currRankStartIdx -= 8;
			currFile = -1;
			currFile++;
		}
		else
		{
			BitBoard bbToAdd = BB_SQUARE[boardPos];

			ChessPiece piece(currChar);

			_board.setPieceBitBoard(piece, bbToAdd);

			if (piece.getType() == King)
			{
				_board.KingPos[piece.getColor()] = (Square)boardPos;
			}

			currFile++;
		}
	}

	//get the turn color
	if (split_fen_code[1].size() != 1)
	{
		throw prefixForErrors + "The turn color should only occupy 1 char";
	}
	_currentTurnColor =
		split_fen_code[1][0] == 'w' ? White :
		split_fen_code[1][0] == 'b' ? Black :
		throw prefixForErrors + "The color could not be found";

	//castling rights
	for (int i = 0; i < split_fen_code[2].size(); i++)
	{
		char currChar = split_fen_code[2][i];

		if (currChar != '-')
		{
			_canCastle
				[getColorOfFenChar(currChar)]
			[getCastlingTypeOfFenChar(currChar)] = true;
		}
	}

	//en passant field
	_enPassantSquare = getSquareFromString(split_fen_code[3]);

	//50 move rule - after half moves without pushing a pawn and
	//without capturing a piece the game ends in a draw
	_halfMoveClock = std::stoi(split_fen_code[4]);

	//keeps track of the moves (one move consists of one move of white and one move of black)
	_moveNumber = std::stoi(split_fen_code[5]);
}

std::string ChessBoard::getString() const
{
	std::string result = "";
	std::string rowSeperatorString = "\n+---+---+---+---+---+---+---+---+\n";
	result += rowSeperatorString;

	for (int rank = 7; rank >= 0; rank--)
	{
		for (int file = 0; file < 8; file++)
		{
			Square pos = (Square)(rank * 8 + file);
			result += "| ";
			result += getPieceCharAt(pos);
			result += " ";

		}
		result += "|" + std::to_string(rank + 1);
		result += rowSeperatorString;
	}
	result += "  a   b   c   d   e   f   g   h\n";

	return result;
}

std::string ChessBoard::getFen() const
{
	std::string result = "";

	//written with copilot. can be improved i think

	for (int rank = 7; rank >= 0; rank--)
	{
		int emptyFields = 0;
		for (int file = 0; file < 8; file++)
		{
			Square pos = (Square)(rank * 8 + file);
			char pieceChar = getPieceCharAt(pos);

			if (pieceChar == ' ')
			{
				emptyFields++;
			}
			else
			{
				if (emptyFields > 0)
				{
					result += std::to_string(emptyFields);
					emptyFields = 0;
				}
				result += pieceChar;
			}
		}
		if (emptyFields > 0)
		{
			result += std::to_string(emptyFields);
		}
		if (rank > 0)
		{
			result += "/";
		}
	}

	result += " ";

	result += _currentTurnColor == White ? "w" : "b";

	result += " ";

	if (_canCastle[White][CastleShort] ||
		_canCastle[White][CastleLong] ||
		_canCastle[Black][CastleShort] ||
		_canCastle[Black][CastleLong])
	{
		if (_canCastle[White][CastleShort])
		{
			result += "K";
		}
		if (_canCastle[White][CastleLong])
		{
			result += "Q";
		}
		if (_canCastle[Black][CastleShort])
		{
			result += "k";
		}
		if (_canCastle[Black][CastleLong])
		{
			result += "q";
		}
	}
	else
	{
		result += "-";
	}

	result += " ";

	result += SQUARE_STRING[_enPassantSquare];

	result += " ";

	result += std::to_string(_halfMoveClock);

	result += " ";

	result += std::to_string(_moveNumber);

	return result;
}

ChessColor ChessBoard::getCurrentTurnColor() const
{
	return _currentTurnColor;
}

int ChessBoard::getNumberOfMovesPlayed() const
{
	return _moveNumber;
}

bool ChessBoard::isKingInCheck() const
{
	return fieldIsUnderAttack(_board.KingPos[_currentTurnColor]);
}

UniqueMoveList ChessBoard::getAllLegalMoves() const
{
	UniqueMoveList list = getAllPseudoLegalMoves();
	list.erase
	(
		std::remove_if
		(
			list.begin(),
			list.end(),
			[this](const std::unique_ptr<Move>& move)
			{
				return !moveIsLegal(move);
			}
		),
		list.end()
				);

	return list;
}

UniqueMoveList ChessBoard::getAllLegalCaptureMoves() const
{
	UniqueMoveList list = getAllLegalMoves();

	list.erase
	(
		std::remove_if
		(
			list.begin(),
			list.end(),
			[this](const std::unique_ptr<Move>& move)
			{
				return !isCaptureMove(move);
			}
		),
		list.end()
				);

	return list;
}

void ChessBoard::makeMove(const Move& move)
{
	Square moveStart = move.getStart();
	Square moveDest = move.getDestination();

	udpateCastlingRightsAfterMove(move);
	updateEnPassantRightsAfterMove(move);
	update50MoveRule(move);

	move.execute(_board);

	if (moveStart == _board.KingPos[_currentTurnColor])
	{
		_board.KingPos[_currentTurnColor] = moveDest;
	}

	if (_currentTurnColor == Black)
	{
		_moveNumber++;
	}

	_currentTurnColor = getOppositeColor(_currentTurnColor);
}

ChessBoard ChessBoard::getCopyByValue() const
{
	ChessBoard board(EMPTY_FEN);
	board._board = _board;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			board._canCastle[i][j] = _canCastle[i][j];
		}
	}

	board._currentTurnColor = _currentTurnColor;

	board._enPassantSquare = _enPassantSquare;

	board._halfMoveClock = _halfMoveClock;

	board._moveNumber = _moveNumber;

	return board;
}

GameState ChessBoard::getGameState() const
{
	if (getAllLegalMoves().size() == 0)
	{
		if (fieldIsUnderAttack(_board.KingPos[_currentTurnColor]))
		{
			return _currentTurnColor == White ? BlackWon : WhiteWon;
		}
		else
		{
			return Stalemate;
		}
	}
	if (_halfMoveClock == 50 ||
		insufficientMaterialCheck())
	{
		return Draw;
	}
	return Ongoing;
}

GameDurationState ChessBoard::getGameDurationState() const
{
	//HAS TO BE IMPROVED ALOT
	// 
	// 
	//description at the end of: https://www.chessprogramming.org/Simplified_Evaluation_Function

	//definiton on the site of when the endgame begins:
	//- Both sides have no queens or
	//- Every side which has a queen has additionally no other pieces or one minorpiece maximum.

	//there are different game states for different colors
	//for example: if one color got lots of pieces, but the other almost none, 
	//it is useful to bring the king into the game
	ChessColor col = _currentTurnColor;
	BitBoard colorBB = _board.PiecesOfColor[col];

	if (((_board.PiecesOfType[Queen] == 0ULL) &&
		//a queen can be exchanged very early on. So we need to check if the game has already progressed a bit
		_moveNumber > 10))
	{
		//gets activated often at the start of games, 
		//which is not exactly an "endgame" state
		//return EndGame;
	}


	if ((_board.PiecesOfType[Queen] & colorBB) != 0ULL)
	{
		BitBoard colorBBWithoutKingOrQueen =
			colorBB &
			~_board.PiecesOfType[King] &
			~_board.PiecesOfType[Queen];

		if ((colorBBWithoutKingOrQueen & ~_board.PiecesOfType[Bishop]) == 0ULL ||
			(colorBBWithoutKingOrQueen & ~_board.PiecesOfType[Knight]) == 0ULL ||
			(colorBBWithoutKingOrQueen & ~_board.PiecesOfType[Pawn]) == 0ULL ||
			(colorBBWithoutKingOrQueen & ~_board.PiecesOfType[Rook]) == 0ULL)
		{
			return EndGame;
		}
	}


	return MidGame;
}

BoardRepresentation ChessBoard::getBoardRepresentation() const
{
	return _board;
}

bool operator==(const ChessBoard& first, const ChessBoard& second)
{
	bool retVal = first._board == second._board;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			retVal = retVal && (first._canCastle[i][j] == second._canCastle[i][j]);
		}
	}

	retVal = retVal && (first._currentTurnColor == second._currentTurnColor);

	retVal = retVal && (first._enPassantSquare == second._enPassantSquare);

	retVal = retVal && (first._halfMoveClock == second._halfMoveClock);

	retVal = retVal && (first._moveNumber == second._moveNumber);

	return retVal;
}

bool operator!=(const ChessBoard& first, const ChessBoard& second)
{
	return !(first == second);
}
size_t chess_board_hasher::operator()(const ChessBoard& board) const
{
	static std::random_device rd;
	static std::mt19937_64 generator(rd());
	static std::uniform_int_distribution<BitBoard> distribution(0, std::numeric_limits<BitBoard>::max());

	const static BitBoard black_to_move_zobrist = distribution(generator);

	const static std::array<std::array<BitBoard, 6>, 2> zobrist_table =
		[]()->std::array<std::array<BitBoard, 6>, 2>
	{
		std::array<std::array<BitBoard, 6>, 2> result = {};

		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 6; j++)
			{
				result[i][j] = distribution(generator);
			}
		}

		return result;
	}();

	const BoardRepresentation& rep = board.getBoardRepresentation();

	BitBoard hash = 0ULL;

	if (board.getCurrentTurnColor() == ChessColor::Black)
	{
		hash ^= black_to_move_zobrist;
	}

	for (int piece_type = 0; piece_type < 6; piece_type++)
	{
		for (int color = 0; color < 2; color++)
		{
			BitBoard piece_table = (rep.PiecesOfType[piece_type] | rep.PiecesOfColor[color]) ^ zobrist_table[color][piece_type];
			hash ^= piece_table;
		}
	}

	return hash;
}