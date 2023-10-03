/*
#include "CppUnitTest.h"
#include "../MockChessEngine/ChessBoard.h"
#include "../LeonardoAi/leonardo_util.hpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AiTest
{
	TEST_CLASS(board_flipper_test)
	{
	public:
		void test_board(
			const std::string& first_fen,
			const std::string& second_fen)
		{
			ChessBoard w_board(first_fen);
			ChessBoard b_board(second_fen);

			matrix w_matrix(leonardo_util::get_input_format());
			matrix b_matrix(leonardo_util::get_input_format());

			leonardo_util::set_matrix_from_chessboard(w_board, w_matrix);
			leonardo_util::set_matrix_from_chessboard(b_board, b_matrix);

			Assert::IsTrue(matrix::are_equal(w_matrix, b_matrix));
		}

		TEST_METHOD(start_position)
		{
			test_board(
				"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
				"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1"
			);
		}
		TEST_METHOD(moved_position)
		{
			test_board(
				"rnbqkbnr/ppppppp1/8/7p/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
				"rnbqkbnr/pppppppp/8/8/7P/8/PPPPPPP1/RNBQKBNR b KQkq - 0 1"
			);
		}
		TEST_METHOD(asymmetric_position)
		{
			test_board(
				"rnbqkbnr/pp1p1ppp/8/2p1p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 1",
				"rnbqkbnr/pppp1ppp/8/4p3/2P1P3/8/PP1P1PPP/RNBQKBNR b KQkq - 0 1"
			);
		}
	};
}
*/