#include "Move.h"

Move::Move(square from, square to)
{
	this->from = from;
	this->to = to;
}

Move::Move(square from, pieceType promotion)
{
	this->from = from;
	this->to = from;
	this->promotion = promotion;
}

square Move::getFrom()
{
	return from;
}

square Move::getTo()
{
	return to;
}

pieceType Move::getPromotion()
{
	return promotion;
}