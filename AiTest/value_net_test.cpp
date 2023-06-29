#include "CppUnitTest.h"
#include "../MockChessEngine/ChessBoard.h"
#include "../LeonardoAi/leonardo_util.hpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AiTest
{
	TEST_CLASS(value_net_test)
	{
	public:
		TEST_METHOD(fools_mate_white)
		{
			//checkmate for white
			ChessBoard board("rnb1kbnr/pppp1ppp/4p3/8/6Pq/5P2/PPPPP2P/RNBQKBNR w KQkq - 1 3");
			matrix w_value(leonardo_util::get_prediction_output_format());
			matrix b_value(leonardo_util::get_prediction_output_format());

			leonardo_util::set_prediction_output(w_value, board, White);
			leonardo_util::set_prediction_output(b_value, board, Black);

			matrix win_matrix(leonardo_util::get_prediction_output_format());
			win_matrix.set_at_flat_host(0, 1);
			win_matrix.set_at_flat_host(1, 0);
			matrix lose_matrix(leonardo_util::get_prediction_output_format());
			lose_matrix.set_at_flat_host(0, 0);
			lose_matrix.set_at_flat_host(1, 1);

			Assert::IsTrue(matrix::are_equal(w_value, lose_matrix));
			Assert::IsTrue(matrix::are_equal(b_value, win_matrix));

			Assert::AreEqual(-1.0f, leonardo_util::get_prediction_output(w_value));
			Assert::AreEqual(1.0f, leonardo_util::get_prediction_output(b_value));
		}
		TEST_METHOD(fools_mate_black)
		{
			//checkmate for white
			ChessBoard board("rnbqkbnr/ppppp2p/5p2/6pQ/8/P3P3/1PPP1PPP/RNB1KBNR b KQkq - 1 3");
			matrix w_value(leonardo_util::get_prediction_output_format());
			matrix b_value(leonardo_util::get_prediction_output_format());

			leonardo_util::set_prediction_output(w_value, board, White);
			leonardo_util::set_prediction_output(b_value, board, Black);

			matrix win_matrix(leonardo_util::get_prediction_output_format());
			win_matrix.set_at_flat_host(0, 1);
			win_matrix.set_at_flat_host(1, 0);
			matrix lose_matrix(leonardo_util::get_prediction_output_format());
			lose_matrix.set_at_flat_host(0, 0);
			lose_matrix.set_at_flat_host(1, 1);

			Assert::IsTrue(matrix::are_equal(w_value, win_matrix));
			Assert::IsTrue(matrix::are_equal(b_value, lose_matrix));

			Assert::AreEqual(1.0f, leonardo_util::get_prediction_output(w_value));
			Assert::AreEqual(-1.0f, leonardo_util::get_prediction_output(b_value));
		}
	};
}