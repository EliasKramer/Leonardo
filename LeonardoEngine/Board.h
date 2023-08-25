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

		std::vector<Piece> whitePiecesList;
		std::vector<Piece> blackPiecesList;
		square whiteKingSquare = SQUARE_NONE;
		square blackKingSquare = SQUARE_NONE;

		Color turnColor = WHITE;

		bitboard enPassantSquare = 0;

		bool whiteLeftCastleAvailable = true;
		bool whiteRightCastleAvailable = true;
		bool blackLeftCastleAvailable = true;
		bool blackRightCastleAvailable = true;

		std::vector<Piece> getPiecesOfColor(Color color);
		bool checkDirectionForAttack(square square, direction dir, bitboard enemySlidingPieces);
		void movePieceFromList(Color color, square from, square to);
		void removePieceFromList(Color color, square square);
	public:
		Board(bitboard pawns, bitboard knights, bitboard bishops, bitboard rooks, bitboard queens, bitboard kings, bitboard whitePieces, bitboard blackPieces);
		Board(std::string FEN, Color turnColor, square enPassantSquare, bool whiteLeftCastle, bool whiteRightCastle, bool blackLeftCastle, bool blackRightCastle);
		pieceType getType(bitboard pieceBB);
		std::string getFEN();

		bool squareIsAttackedBy(square square, Color color);

		void executeMove(Move move);

		bool isMoveStrictlyLegal(Move move);

		bitboard getPawns();
		bitboard getKnights();
		bitboard getBishops();
		bitboard getRooks();
		bitboard getQueens();
		bitboard getKings();

		bitboard getWhitePieces();
		bitboard getBlackPieces();
		bitboard getAllPieces();

		std::vector<Piece> getWhitePiecesList();
		std::vector<Piece> getBlackPiecesList();

		Color getTurnColor();

		bitboard getEnPassantSquare();

		bool getWhiteLeftCastleAvailable();
		bool getWhiteRightCastleAvailable();
		bool getBlackLeftCastleAvailable();
		bool getBlackRightCastleAvailable();
		void whiteCastle();
		void blackCastle();
};

