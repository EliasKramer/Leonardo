#pragma once

#include <vector>

#include "Types.h"
#include "Board.h"
#include "Move.h"

std::vector<Move> getMoves(Board board, color color);

std::vector<Move> getMoves(Board board, color color, int depth);


std::vector<Move> getMovesForPiece(Board board, piece piece);

std::vector<Move> getMovesForPawn(Board board, piece pawn);

std::vector<Move> getMovesForKnight(Board board, piece knight);

std::vector<Move> getMovesForBishop(Board board, piece bishop);

std::vector<Move> getMovesForRook(Board board, piece rook);

std::vector<Move> getMovesForQueen(Board board, piece queen);

std::vector<Move> getMovesForKing(Board board, piece king);

void addSlidingMovesInDirection(std::vector<Move>& moves, Board board, piece piece, direction dir);
void addMoveInDirection(std::vector<Move>& moves, Board board, piece piece, direction dir);