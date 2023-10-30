#pragma once

#include <vector>

#include "Types.h"
#include "Board.h"
#include "Move.h"

std::vector<Move> getMoves(Board &board);

int getNodesForDepth(Board& board, int depth);


std::vector<Move> getMovesForPiece(const Board &board, Piece &piece, uint8_t pieceIndex);

std::vector<Move> getMovesForPawn(const Board &board, Piece &pawn, uint8_t pieceIndex);

std::vector<Move> getMovesForKnight(const Board &board, Piece &knight, uint8_t pieceIndex);

std::vector<Move> getMovesForBishop(const Board &board, Piece &bishop, uint8_t pieceIndex);

std::vector<Move> getMovesForRook(const Board &board, Piece &rook, uint8_t pieceIndex);

std::vector<Move> getMovesForQueen(const Board &board, Piece &queen, uint8_t pieceIndex);

std::vector<Move> getMovesForKing(const Board &board, Piece &king, uint8_t pieceIndex);

void addSlidingMovesInDirection(std::vector<Move>& moves, const Board &board, Piece &piece, uint8_t pieceIndex, direction dir);
void addMoveInDirection(std::vector<Move>& moves, const Board &board, Piece &piece, uint8_t pieceIndex, direction dir);