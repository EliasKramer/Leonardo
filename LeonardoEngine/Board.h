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
		bool checkDirectionForAttack(square square, direction dir, bitboard enemySlidingPieces) const;
		void changeBoardPositionInList(std::vector<Piece> *list_p, square from, square to);
		void removePieceFromList(std::vector<Piece> *list_p, square square);
	public:
		Board(bitboard pawns, bitboard knights, bitboard bishops, bitboard rooks, bitboard queens, bitboard kings, bitboard whitePieces, bitboard blackPieces);
		Board(std::string FEN, Color turnColor, square enPassantSquare, bool whiteLeftCastle, bool whiteRightCastle, bool blackLeftCastle, bool blackRightCastle);
		pieceType getType(bitboard pieceBB);
		std::string getFEN();

		bool squareIsAttackedBy(square square, Color color) const;

		void executeMove(Move move);

		bool isMoveStrictlyLegal(Move move);

		bitboard getPawns() const;
		bitboard getKnights() const;
		bitboard getBishops() const;
		bitboard getRooks() const;
		bitboard getQueens() const;
		bitboard getKings() const;

		bitboard getWhitePieces() const;
		bitboard getBlackPieces() const;
		bitboard getAllPieces() const;

		std::vector<Piece> &getWhitePiecesList();
		std::vector<Piece> &getBlackPiecesList();
		void setWhitePiecesList(std::vector<Piece> &list);
		void setBlackPiecesList(std::vector<Piece> &list);

		Color getTurnColor() const;

		bitboard getEnPassantSquare() const;

		bool getWhiteLeftCastleAvailable() const;
		bool getWhiteRightCastleAvailable() const;
		bool getBlackLeftCastleAvailable() const;
		bool getBlackRightCastleAvailable() const;
		void whiteCastle();
		void blackCastle();
};

