#include "Player.h"

Player::Player(std::string name)
    :_name(name)
{}

std::string Player::getName()
{
    return _name;
}

int Player::getMove(const ChessBoard& board, const UniqueMoveList& moves)
{
    throw std::exception("Cannot call Player Base method");
}