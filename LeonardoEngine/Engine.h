#pragma once

#include <vector>

#include "Types.h"
#include "Board.h"
#include "Move.h"

std::vector<Move> getMoves(Board board, color color);

std::vector<Move> getMoves(Board board, color color, int depth);


std::vector<Move> getMovesForPiece(Board board, Piece &piece);

std::vector<Move> getMovesForPawn(Board board, Piece &pawn);

std::vector<Move> getMovesForKnight(Board board, Piece &knight);

std::vector<Move> getMovesForBishop(Board board, Piece &bishop);

std::vector<Move> getMovesForRook(Board board, Piece &rook);

std::vector<Move> getMovesForQueen(Board board, Piece &queen);

std::vector<Move> getMovesForKing(Board board, Piece &king);

void addSlidingMovesInDirection(std::vector<Move>& moves, Board board, Piece &piece, direction dir);
void addMoveInDirection(std::vector<Move>& moves, Board board, Piece &piece, direction dir);