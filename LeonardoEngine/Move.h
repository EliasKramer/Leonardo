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
		pieceType piece;
		square from;
		square to;
		moveType type = NORMAL;
	public:
		Move(pieceType piece, square from, square to);
		Move(pieceType piece, square from, square to, moveType type);
		Move(square from, pieceType promotion);
		pieceType getPieceType();
		square getFrom();
		square getTo();
		moveType getType();

		bool operator==(const Move& other) const;
};
