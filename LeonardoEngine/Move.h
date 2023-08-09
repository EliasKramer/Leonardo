#pragma once
#include "Types.h"

enum moveType : uint8_t {
	NORMAL,
	PROMOTION,
	CASTLE,
	EN_PASSANT
};

class Move
{
	private:
		square from;
		square to;
		pieceType promotion = NONE;
		moveType type = NORMAL;
	public:
		Move(square from, square to);
		Move(square from, square to, moveType type);
		Move(square from, pieceType promotion);
		square getFrom();
		square getTo();
		pieceType getPromotion();

		bool operator==(const Move& other) const;
};
