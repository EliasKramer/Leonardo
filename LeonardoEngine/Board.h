#pragma once

#include "Types.h"
#include "Move.h"
#include <vector>
class Board
{
	private:
		bitboard pawns;
		bitboard knights;
		bitboard bishops;
		bitboard rooks;
		bitboard queens;
		bitboard kings;

		bitboard whitePieces;
		bitboard blackPieces;

		std::vector<piece> whitePiecesList;
		std::vector<piece> blackPiecesList;

		color turnColor = WHITE;

		bitboard enPassantSquare = 0;

		bool whiteLeftCastleAvailable = true;
		bool whiteRightCastleAvailable = true;
		bool blackLeftCastleAvailable = true;
		bool blackRightCastleAvailable = true;

		std::vector<piece> getPiecesOfColor(color color);
		bool checkDirectionForAttack(square square, direction dir, bitboard EnemySlidingPieces);
	public:
		Board(bitboard pawns, bitboard knights, bitboard bishops, bitboard rooks, bitboard queens, bitboard kings, bitboard whitePieces, bitboard blackPieces);
		pieceType getType(bitboard pieceBB);

		bool squareIsAttackedBy(square square, color color);

		void executeMove(Move move);

		bitboard getPawns();
		bitboard getKnights();
		bitboard getBishops();
		bitboard getRooks();
		bitboard getQueens();
		bitboard getKings();

		bitboard getWhitePieces();
		bitboard getBlackPieces();
		bitboard getAllPieces();

		std::vector<piece> getWhitePiecesList();
		std::vector<piece> getBlackPiecesList();

		color getTurnColor();

		bitboard getEnPassantSquare();

		bool getWhiteLeftCastleAvailable();
		bool getWhiteRightCastleAvailable();
		bool getBlackLeftCastleAvailable();
		bool getBlackRightCastleAvailable();
		void whiteCastle();
		void blackCastle();
};

