#include "Constants.h"

const std::array<std::array<std::array<Square, 3>, 2>, 2> SQUARES_FOR_KING_CASTLING =
[]()->std::array<std::array<std::array<Square, 3>, 2>, 2> {

	std::array<std::array<std::array<Square, 3>, 2>, 2> result = {};

	result[White][CastleShort][0] = E1; //start 
	result[White][CastleShort][1] = F1; //rook dest - not used
	result[White][CastleShort][2] = G1; //end

	result[White][CastleLong][0] = E1; //start
	result[White][CastleLong][1] = D1; //rook dest - not used
	result[White][CastleLong][2] = C1; //end

	result[Black][CastleShort][0] = E8; //start
	result[Black][CastleShort][1] = F8; //rook dest - not used
	result[Black][CastleShort][2] = G8; //end

	result[Black][CastleLong][0] = E8; //start
	result[Black][CastleLong][1] = D8; //rook dest - not used
	result[Black][CastleLong][2] = C8; //end

	return result;
}();

const std::array<std::array<std::array<Square, 2>, 2>, 2> SQUARES_FOR_ROOK_CASTLING =
[]()->std::array<std::array<std::array<Square, 2>, 2>, 2> {
	std::array<std::array<std::array<Square, 2>, 2>, 2> result = {};

	result[White][CastleShort][0] = H1; //start
	result[White][CastleShort][1] = F1; //end

	result[White][CastleLong][0] = A1; //start
	result[White][CastleLong][1] = D1; //end

	result[Black][CastleShort][0] = H8; //start
	result[Black][CastleShort][1] = F8; //end

	result[Black][CastleLong][0] = A8; //start
	result[Black][CastleLong][1] = D8; //end

	return result;
}();

const std::array<BitBoard, 64> KNIGHT_ATTACK_BB = []()->std::array<BitBoard, 64> {
	std::array<BitBoard, 64> result = {};

	for (int i = A1; i <= H8; i++)
	{
		BitBoard currBitBoard = BITBOARD_NONE;

		Square currSquare = (Square)i;
		for (int j = 0; j < 8; j++)
		{
			Direction dir = KNIGHT_DIRECTIONS[j];
			if (destinationIsOnBoard(currSquare, dir))
			{
				currBitBoard |= BB_SQUARE[i + dir];
			}
		}

		result[i] = currBitBoard;
	}

	return result;
}();

const std::array<std::array<BitBoard, 64>, 2> PAWN_ATTACK_BB =
[]()->std::array<std::array<BitBoard, 64>, 2> {

	std::array<std::array<BitBoard, 64>, 2> result = {};

	for (int colorIdx = White; colorIdx <= Black; colorIdx++)
	{
		for (int i = A1; i <= H8; i++)
		{
			Square currSquare = (Square)i;
			for (int j = 0; j < 2; j++)
			{
				Direction dir = PAWN_ATTACK_DIRECTION[colorIdx][j];
				if (destinationIsOnBoard(currSquare, dir))
				{
					result[colorIdx][i] |= BB_SQUARE[i + dir];
				}
			}
		}
	}

	return result;
}();

const std::array<BitBoard, 64> KING_ATTACKS_BB = []()->std::array<BitBoard, 64> {
	std::array<BitBoard, 64> result = {};

	for (int i = A1; i <= H8; i++)
	{
		BitBoard currBitBoard = BITBOARD_NONE;

		Square currSquare = (Square)i;
		for (int j = 0; j < 8; j++)
		{
			Direction dir = ALL_SLIDING_DIRECTIONS[j];
			if (destinationIsOnBoard(currSquare, dir))
			{
				currBitBoard |= BB_SQUARE[i + dir];
			}
		}
		result[i] = currBitBoard;
	}
	return result;
}();

const std::array<std::string, 65> SQUARE_STRING = []()->std::array<std::string, 65> {
	std::array<std::string, 65> result = {};

	for (int i = A1; i <= H8; i++)
	{
		Square currSquare = (Square)i;
		char file = 'a' + (i % 8);
		char rank = '1' + (i / 8);
		result[i] = file;
		result[i] += rank;
	}

	//representation for SQUARE_NONE
	result[64] = "-";

	return result;
}();

const int POSITION_VALUE[2][NUMBER_OF_DIFFERENT_PIECE_TYPES - 1][64]
{
	//it seems to be flipped, but look at with indexes
	//index 0 is a1 - index 63 is h8
	//so the first row represents actually the last board

	//white
	{
		//Pawn
		{0, 0, 0, 0, 0, 0, 0, 0,
		5, 10, 10,-20,-20, 10, 10, 5,
		5, -5,-10, 0, 0,-10, -5, 5,
		0, 0, 0,    35, 35, 0, 0, 0,
		5, 5, 10,   36, 36, 10, 5, 5,
		10, 10, 20, 37, 37, 20, 10, 10,
		50, 50, 50, 50, 50, 50, 50, 50,
		0, 0, 0, 0, 0, 0, 0, 0},

		//Knight
		{-50,-40,-30,-30,-30,-30,-40,-50,
		-40,-20, 0, 5, 5, 0,-20,-40,
		-30, 5, 10, 15, 15, 10, 5,-30,
		-30, 0, 15, 20, 20, 15, 0,-30,
		-30, 5, 15, 20, 20, 15, 5,-30,
		-30, 0, 10, 15, 15, 10, 0,-30,
		-40,-20, 0, 0, 0, 0,-20,-40,
		-50,-40,-30,-30,-30,-30,-40,-50},

		//Bishop
		{-20,-10,-10,-10,-10,-10,-10,-20,
		-10, 5, 0, 0, 0, 0, 5,-10,
		-10, 10, 10, 10, 10, 10, 10,-10,
		-10, 0, 10, 10, 10, 10, 0,-10,
		-10, 5, 10, 10, 10, 10, 5,-10,
		-10, 0, 5, 10, 10, 5, 0,-10,
		-10, 0, 0, 0, 0, 0, 0,-10,
		-20,-10,-10,-10,-10,-10,-10,-20},

		//Rook
		{0,0, 0, 5, 5, 0, 0, 0,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		-5, 0, 0, 0, 0, 0, 0, -5,
		5, 10, 10, 10, 10, 10, 10, 5,
		0, 0, 0, 0, 0, 0, 0, 0},

		//Queen
		{-20,-10,-10, -5, -5,-10,-10,-20,
		-10, 0, 5, 0, 0, 0, 0,-10,
		-10, 5, 5, 5, 5, 5, 0,-10,
		0, 0, 5, 5, 5, 5, 0, -5,
		-5, 0, 5, 5, 5, 5, 0, -5,
		-10, 0, 5, 5, 5, 5, 0,-10,
		-10, 0, 0, 0, 0, 0, 0,-10,
		-20,-10,-10, -5, -5,-10,-10,-20}
	 },
	//black
	{
		//Pawn
		{0, 0, 0, 0, 0, 0, 0, 0,
		50, 50, 50, 50, 50, 50, 50, 50,
		10, 10, 20, 37, 37, 20, 10, 10,
		 5, 5, 10, 36, 36, 10, 5, 5,
		 0, 0, 0,  35, 35, 0, 0, 0,
		 5, -5,-10, 0, 0,-10, -5, 5,
		 5, 10, 10,-20,-20, 10, 10, 5,
		 0, 0, 0, 0, 0, 0, 0, 0},

		 //Knight
		 {-50,-40,-30,-30,-30,-30,-40,-50,
		 -40,-20, 0, 0, 0, 0,-20,-40,
		 -30, 0, 10, 15, 15, 10, 0,-30,
		 -30, 5, 15, 20, 20, 15, 5,-30,
		 -30, 0, 15, 20, 20, 15, 0,-30,
		 -30, 5, 10, 15, 15, 10, 5,-30,
		 -40,-20, 0, 5, 5, 0,-20,-40,
		 -50,-40,-30,-30,-30,-30,-40,-50},

		 //Bishop
		 {-20,-10,-10,-10,-10,-10,-10,-20,
		 -10, 0, 0, 0, 0, 0, 0,-10,
		 -10, 0, 5, 10, 10, 5, 0,-10,
		 -10, 5, 5, 10, 10, 5, 5,-10,
		 -10, 0, 10, 10, 10, 10, 0,-10,
		 -10, 10, 10, 10, 10, 10, 10,-10,
		 -10, 5, 0, 0, 0, 0, 5,-10,
		 -20,-10,-10,-10,-10,-10,-10,-20},

		 //Rook
		 {0, 0, 0, 0, 0, 0, 0, 0,
		 5, 10, 10, 10, 10, 10, 10, 5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 -5, 0, 0, 0, 0, 0, 0, -5,
		 0, 0, 0, 5, 5, 0, 0, 0},

		 //Queen
		 {-20,-10,-10, -5, -5,-10,-10,-20,
		 -10, 0, 0, 0, 0, 0, 0,-10,
		 -10, 0, 5, 5, 5, 5, 0,-10,
		  -5, 0, 5, 5, 5, 5, 0, -5,
		   0, 0, 5, 5, 5, 5, 0, -5,
		 -10, 5, 5, 5, 5, 5, 0,-10,
		 -10, 0, 5, 0, 0, 0, 0,-10,
		 -20,-10,-10, -5, -5,-10,-10,-20}
	}
};

const int POSITION_VALUE_KING[DIFFERENT_CHESS_COLORS][DIFFERENT_GAME_DURATION_STATES][64] =
{
	//white	
	{
		//midgame
		{20, 30, 10, 0, 0, 10, 30, 20,
		20, 20, 0, 0, 0, 0, 20, 20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30},

		{20, 30, 10, 0, 0, 10, 30, 20,
		20, 20, 0, 0, 0, 0, 20, 20,
		-10, -20, -20, -20, -20, -20, -20, -10,
		-20, -30, -30, -40, -40, -30, -30, -20,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30,
		-30, -40, -40, -50, -50, -40, -40, -30},

		//endgame
	/*
		{-50, -30, -30, -30, -30, -30, -30, -50,
		-30, -30, 0, 0, 0, 0, -30, -30,
		-30, -10, 20, 30, 30, 20, -10, -30,
		-30, -10, 30, 40, 40, 30, -10, -30,
		-30, -10, 30, 40, 40, 30, -10, -30,
		-30, -10, 20, 30, 30, 20, -10, -30,
		-30, -20, -10, 0, 0, -10, -20, -30,
		-50, -40, -30, -20, -20, -30, -40, -50}
		*/
	},
	//black
	{
		//midgame
		{-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-20,-30,-30,-40,-40,-30,-30,-20,
		-10,-20,-20,-20,-20,-20,-20,-10,
		20, 20, 0, 0, 0, 0, 20, 20,
		20, 30, 10, 0, 0, 10, 30, 20},
		
	{-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-30,-40,-40,-50,-50,-40,-40,-30,
		-20,-30,-30,-40,-40,-30,-30,-20,
		-10,-20,-20,-20,-20,-20,-20,-10,
		20, 20, 0, 0, 0, 0, 20, 20,
		20, 30, 10, 0, 0, 10, 30, 20},
		//endgame
	/*
		{-50,-40,-30,-20,-20,-30,-40,-50,
		-30,-20,-10, 0, 0,-10,-20,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 30, 40, 40, 30,-10,-30,
		-30,-10, 20, 30, 30, 20,-10,-30,
		-30,-30, 0, 0, 0, 0,-30,-30,
		-50,-30,-30,-30,-30,-30,-30,-50}
	*/
	}
};