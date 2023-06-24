#include "CppUnitTest.h"
#include "../MockChessEngine/ChessBoard.h"
#include "../LeonardoAi/leonardo_util.hpp"
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace AiTest
{
	TEST_CLASS(move_value)
	{
	public:
		TEST_METHOD(set_one_move)
		{
			ChessBoard board(STARTING_FEN);
			Move move(E2, E4);
			matrix m(leonardo_util::get_policy_output_format());
			m.set_all(0);
			leonardo_util::set_move_value(move, m, 1, White);

			Assert::AreEqual(1.0f, leonardo_util::get_move_value(move, m, White));
		}
		TEST_METHOD(set_two_moves)
		{
			ChessBoard board(STARTING_FEN);
			Move move1(E2, E4);
			Move move2(E3, E5);
			matrix m(leonardo_util::get_policy_output_format());
			m.set_all(0);
			leonardo_util::set_move_value(move1, m, 12, White);
			leonardo_util::set_move_value(move2, m, 2, White);

			Assert::AreEqual(12.0f, leonardo_util::get_move_value(move1, m, White));
			Assert::AreEqual(2.0f, leonardo_util::get_move_value(move2, m, White));
		}
		TEST_METHOD(set_two_moves_with_same_start)
		{
			ChessBoard board(STARTING_FEN);
			Move move1(E2, E4);
			Move move2(E2, E5);
			matrix m(leonardo_util::get_policy_output_format());
			m.set_all(0);
			leonardo_util::set_move_value(move1, m, 5, White);
			leonardo_util::set_move_value(move2, m, 4, White);

			Assert::AreEqual(5.0f, leonardo_util::get_move_value(move1, m, White));
			Assert::AreEqual(4.0f, leonardo_util::get_move_value(move2, m, White));
		}
		TEST_METHOD(set_two_moves_with_same_destination)
		{
			ChessBoard board(STARTING_FEN);
			Move move1(A3, E5);
			Move move2(C2, E5);
			matrix m(leonardo_util::get_policy_output_format());
			m.set_all(0);
			leonardo_util::set_move_value(move1, m, 5, White);
			leonardo_util::set_move_value(move2, m, 4, White);

			Assert::AreEqual(5.0f, leonardo_util::get_move_value(move1, m, White));
			Assert::AreEqual(4.0f, leonardo_util::get_move_value(move2, m, White));
		}
		TEST_METHOD(set_start_pos_without_exception)
		{
			ChessBoard board(STARTING_FEN);
			auto moves = board.getAllLegalMoves();
			matrix m(leonardo_util::get_policy_output_format());
			m.set_all(0);
			for (int i = 0; i < moves.size(); i++)
			{
				leonardo_util::set_move_value(*moves[i].get(), m, i, White);
			}
		}
	};
}