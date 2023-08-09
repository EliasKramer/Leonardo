#pragma once

#include "Types.h"
class Move
{
	private:
		square from;
		square to;
		pieceType promotion = NONE;
	public:
		Move(square from, square to);
		Move(square from, pieceType promotion);
		square getFrom();
		square getTo();
		pieceType getPromotion();

		bool operator==(const Move& other) const;
};
