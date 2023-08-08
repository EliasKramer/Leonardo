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

		std::vector<piece> getPiecesOfColor(color color);
	public:
		Board(bitboard pawns, bitboard knights, bitboard books, bitboard rooks, bitboard queens, bitboard kings, bitboard whitePieces, bitboard blackPieces);
		pieceType getType(bitboard pieceBB);

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
};

