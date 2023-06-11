#pragma once
#include "BitBoard.h"
#include "DataAndTypes.h"
#include <string>
#include "ChessPiece.h"
#include "HelpMethods.h"
#include "Move.h"
#include "MoveEnPassant.h"
#include "MoveCastle.h"
#include "MovePromote.h"
#include "Constants.h"
#include "BoardRepresentation.h"

const std::string STARTING_FEN =
"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

const std::string EMPTY_FEN =
"8/8/8/8/8/8/8/8 w - - 0 1";

class ChessBoard
{
protected:
	BoardRepresentation _board;

	//the first dimension is color and 
	//the second is the type. 
	//aka long or short castling
	bool _canCastle[2][2];

	//the players color, whose turn it is
	ChessColor _currentTurnColor;

	//if an en passant move is possible, 
	//the square, where the opponent has to go to
	//will be stored here
	Square _enPassantSquare;

	//keeps track of the 50 moves rule
	uint16_t _halfMoveClock;

	//the move number. 
	//increases every time when black makes a move
	uint16_t _moveNumber;

	/*Move Generation*/
	bool destinationIsSameColor(Square start, Direction direction, ChessColor color) const;
	bool positionIsSameColor(Square pos, ChessColor color) const;
	
	void addIfDestinationIsValid(
		UniqueMoveList& moves,
		Square start,
		Direction dir) const;
	void addIfDestinationIsColor(
		UniqueMoveList& moves,
		Square start,
		Direction dir,
		ChessColor color) const;

	//Pseudo legal moves are moves that follow 
	//the move rules for the types.
	//They do not check if the king is in check
	UniqueMoveList getAllPseudoLegalMoves() const;

	void getPawnMoves(UniqueMoveList& moves) const;
	void getKnightMoves(UniqueMoveList& moves) const;
	void getBishopMoves(UniqueMoveList& moves) const;
	void getRookMoves(UniqueMoveList& moves) const;
	void getQueenMoves(UniqueMoveList& moves) const;
	void getKingMoves(UniqueMoveList& moves) const;

	void addPawnMove(
		UniqueMoveList& moves, Square start, Square dest) const;
	void getCastlingMoves(UniqueMoveList& moves) const;
	void getEnPassantMove(UniqueMoveList& moves) const;
	
	void addRayMoves(
		UniqueMoveList& moves,
		Square start,
		Direction directions[],
		int numberOfDirections) const;

	//square gets attacke by the opponent
	bool fieldIsUnderAttack(Square pos, BitBoard moveBB = BITBOARD_NONE) const;
	//checks if square gets attacked by sliding pieces
	bool fieldGetsAttackedBySlidingPiece(Square pos, BitBoard moveBB = BITBOARD_NONE) const;

	bool moveIsLegal(const std::unique_ptr<Move>& move) const;
	bool isCaptureMove(const std::unique_ptr<Move>& move) const;
	
	void udpateCastlingRightsAfterMove(Move& m);
	
	void updateEnPassantRightsAfterMove(Move& m);

	void update50MoveRule(Move& m);

	//checks if any side has enough material/pieces to win the game
	bool insufficientMaterialCheck() const;
	
	char getPieceCharAt(Square pos) const;

	friend bool operator ==(const ChessBoard& first, const ChessBoard& second);
	friend bool operator !=(const ChessBoard& first, const ChessBoard& second);
public:
	ChessBoard(std::string given_fen_code);

	std::string getString();
	std::string getFen();

	ChessColor getCurrentTurnColor() const;
	int getNumberOfMovesPlayed() const;
	
	//returns true if the king is in check
	bool isKingInCheck() const;

	//returns all moves of the color, whos turn it is
	UniqueMoveList getAllLegalMoves() const;
	UniqueMoveList getAllLegalCaptureMoves() const;

	//execute move
	void makeMove(Move& move);

	//copy board by value
	ChessBoard getCopyByValue() const;

	GameState getGameState() const;
	GameDurationState getGameDurationState() const;

	BoardRepresentation getBoardRepresentation() const;
};

bool operator ==(const ChessBoard& first, const ChessBoard& second);
bool operator !=(const ChessBoard& first, const ChessBoard& second);