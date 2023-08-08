#pragma once

#include "Types.h"
#include <vector>
class Board
{
	private:
		map pawns;
		map knights;
		map bishops;
		map rooks;
		map queens;
		map kings;

		map whitePieces;
		map blackPieces;

		std::vector<piece> whitePiecesList;
		std::vector<piece> blackPiecesList;

		std::vector<piece> getPiecesOfColor(color color);
	public:
		Board(map pawns, map knights, map books, map rooks, map queens, map kings, map whitePieces, map blackPieces);
		pieceType getType(map pieceBB);

		map getPawns();
		map getKnights();
		map getBishops();
		map getRooks();
		map getQueens();
		map getKings();

		map getWhitePieces();
		map getBlackPieces();
		map getAllPieces();

		std::vector<piece> getWhitePiecesList();
		std::vector<piece> getBlackPiecesList();
};

