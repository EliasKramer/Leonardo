#pragma once

#include "Types.h"
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

		bitboard enPassant = 0;

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
		void switchTurnColor();

		bitboard getEnPassant();
		void setEnPassant(bitboard position);

		bool getWhiteLeftCastleAvailable();
		bool getWhiteRightCastleAvailable();
		bool getBlackLeftCastleAvailable();
		bool getBlackRightCastleAvailable();
		void whiteCastle();
		void blackCastle();
};

