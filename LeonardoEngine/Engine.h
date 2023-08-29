#pragma once

#include <vector>

#include "Types.h"
#include "Board.h"
#include "Move.h"

std::vector<Move> getMoves(Board &board);

std::vector<Move> getMoves(Board &board, int depth);


std::vector<Move> getMovesForPiece(const Board &board, Piece &piece);

std::vector<Move> getMovesForPawn(const Board &board, Piece &pawn);

std::vector<Move> getMovesForKnight(const Board &board, Piece &knight);

std::vector<Move> getMovesForBishop(const Board &board, Piece &bishop);

std::vector<Move> getMovesForRook(const Board &board, Piece &rook);

std::vector<Move> getMovesForQueen(const Board &board, Piece &queen);

std::vector<Move> getMovesForKing(const Board &board, Piece &king);

void addSlidingMovesInDirection(std::vector<Move>& moves, const Board &board, Piece &piece, direction dir);
void addMoveInDirection(std::vector<Move>& moves, const Board &board, Piece &piece, direction dir);