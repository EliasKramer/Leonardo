#include "CppUnitTest.h"
#include "../LeonardoAi/leonardo_util.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace LeonardoAiTest
{
	TEST_CLASS(leonardo_ai_test)
	{
	public:		
		TEST_METHOD(board_to_matrix_converting)
		{
			ChessBoard w_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
			ChessBoard b_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");

			matrix w_matrix(leonardo_util::get_input_format());
			matrix b_matrix(leonardo_util::get_input_format());

			//leonardo_util::set_matrix_from_chessboard(w_board, w_matrix);
			//leonardo_util::set_matrix_from_chessboard(b_board, b_matrix);

			Assert::IsTrue(matrix::are_equal(w_matrix, b_matrix));
		}
	};
}
