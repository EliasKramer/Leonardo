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
    throw std::runtime_error("Cannot call Player Base method");
    return -1;
}