/*
#include "CppUnitTest.h"
#include "../MockChessEngine/ChessBoard.h"
#include "../LeonardoAi/leonardo_util.hpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AiTest
{
	TEST_CLASS(move_flipper_test)
	{
	public:
		void test_move(
			const Move& white_move,
			const Move& black_move
		)
		{
			matrix w_move(leonardo_util::get_policy_output_format());
			w_move.set_all(0);
			leonardo_util::set_move_value(white_move, w_move, 1, White);
			matrix b_move(leonardo_util::get_policy_output_format());
			b_move.set_all(0);
			leonardo_util::set_move_value(black_move, b_move, 1, Black);

			Assert::IsTrue(matrix::are_equal(w_move, b_move));
		}
		TEST_METHOD(move_1)
		{
			test_move(Move(C2, C4), Move(C7, C5));
		}
		TEST_METHOD(move_2)
		{
			test_move(Move(G1, F3), Move(G8, F6));
		}
		TEST_METHOD(move_3)
		{
			test_move(Move(E1, A5), Move(E8, A4));
		}
	};
}*/