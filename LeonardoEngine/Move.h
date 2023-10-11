#pragma once
#include "Types.h"

enum moveType : uint8_t {
	NORMAL,
	PROMOTION,
	CASTLE_LEFT,
	CASTLE_RIGHT,
	EN_PASSANT
};

class Move
{
	private:
		uint8_t pieceIndex;
		square from;
		square to;
		moveType type = NORMAL;
		pieceType promotion = PIECE_NONE;
	public:
		Move(Piece *piece, uint8_t pieceIndex, square from, square to);
		Move(Piece *piece, uint8_t pieceIndex, square from, square to, moveType type);
		Move(Piece *piece, uint8_t pieceIndex, square from, square to, pieceType promotion);
		uint8_t getPieceIndex();
		square getFrom();
		square getTo();
		pieceType getPromotion();
		moveType getType();

		bool operator==(const Move& other) const;
};
