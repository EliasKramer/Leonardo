#pragma once
#include "BitBoard.h"
#include "DataAndTypes.h"
#include <vector>
#include <map>
#include <string>

const Direction KNIGHT_DIRECTIONS[8] = {
	NORTH_NORTH_EAST,
	EAST_NORTH_EAST,
	EAST_SOUTH_EAST,
	SOUTH_SOUTH_EAST,
	SOUTH_SOUTH_WEST,
	WEST_SOUTH_WEST,
	WEST_NORTH_WEST,
	NORTH_NORTH_WEST
};

const Direction ALL_SLIDING_DIRECTIONS[8] = {
	NORTH,
	EAST,
	SOUTH,
	WEST,
	NORTH_EAST,
	SOUTH_EAST,
	SOUTH_WEST,
	NORTH_WEST
};

const Direction ROOK_DIRECTIONS[4] = {
	NORTH,
	EAST,
	SOUTH,
	WEST
};

const Direction BISHOP_DIRECTIONS[4] = {
	NORTH_EAST,
	SOUTH_EAST,
	SOUTH_WEST,
	NORTH_WEST
};

const Direction PAWN_ATTACK_DIRECTION[2][2]{
	{NORTH_EAST, NORTH_WEST},
	{SOUTH_EAST, SOUTH_WEST}
};

const BitBoard BACK_RANK[2] = { RANK_1, RANK_8 };

//first dimension is color, second is castling type and 
//third is the list of squares, that have to be not attacked when castling
extern const std::array<std::array<std::array<Square, 3>, 2>, 2> SQUARES_FOR_KING_CASTLING;

//the first dimension is color, second is castling type
//third on the first idx is start and second idx is destination
extern const std::array<std::array<std::array<Square, 2>, 2>, 2> SQUARES_FOR_ROOK_CASTLING;

extern const std::array<BitBoard, 64> KNIGHT_ATTACK_BB;

extern const std::array<std::array<BitBoard, 64>, 2> PAWN_ATTACK_BB;

extern const std::array<BitBoard, 64> KING_ATTACKS_BB;

//size of 65 because SQUARE_NONE has also a string representation 
extern const std::array<std::string, 65> SQUARE_STRING;

const BitBoard BACKRANK[2] = { RANK_1, RANK_8 };
const BitBoard CASTLINGSIDE[2] = { FILE_H, FILE_A };
const BitBoard KINGFILE = FILE_E;
const BitBoard SQUARES_EFFECTED_BY_CASTLING_BB =
BB_SQUARE[A1] | BB_SQUARE[E1] | BB_SQUARE[H1] |
BB_SQUARE[A8] | BB_SQUARE[E8] | BB_SQUARE[H8];

const char PIECETYPE_CHAR[NUMBER_OF_DIFFERENT_PIECE_TYPES] = { 'P', 'N', 'B', 'R', 'Q', 'K' };

const std::string COLOR_STRING[2] = { "White", "Black" };

const int PAWN_VALUE = 100;
const int KNIGHT_VALUE = 320;
const int BISHOP_VALUE = 330;
const int ROOK_VALUE = 500;
const int QUEEN_VALUE = 900;

const int PIECETYPE_VALUE[5] = {
	PAWN_VALUE,
	KNIGHT_VALUE,
	BISHOP_VALUE,
	ROOK_VALUE,
	QUEEN_VALUE
};

//https://www.chessprogramming.org/Simplified_Evaluation_Function
//Some pieces are more worth on different fields.
//add this value to the piece value
//the king is excepted here, because it differs from early to endgame.
//first dimension is color, second is piece type, third is square
extern const int POSITION_VALUE[2][NUMBER_OF_DIFFERENT_PIECE_TYPES - 1][64];

const int DIFFERENT_GAME_DURATION_STATES = 2;
enum GameDurationState {
	MidGame,
	EndGame
};

//first dimension is the color, the second the current game state duration and the third is the value of the square
extern const int POSITION_VALUE_KING[DIFFERENT_CHESS_COLORS][DIFFERENT_GAME_DURATION_STATES][64];

//there are some issues with int_min or max when multiplying by -1
//therfore these are only high values and not max values
const int WHITE_WIN_EVAL_VALUE = 1000000000;
const int BLACK_WIN_EVAL_VALUE = -1000000000;

const int GAME_STATE_EVALUATION[5] = {
	WHITE_WIN_EVAL_VALUE, //white won 
	BLACK_WIN_EVAL_VALUE, //black won
	0, //stalemate
	0, //draw
	-1 //ongoing
};

const std::string GAME_STATE_STRING[5] = {
	"white wins",
	"black wins",
	"stalemate",
	"draw",
	"ongoing"
};