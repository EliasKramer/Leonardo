#include <CppUnitTest.h>
#include <vector>
#include <algorithm>

#include <Engine.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EngineTest
{
	TEST_CLASS(EngineTest)
	{
	public:

		TEST_METHOD(getRegularPawnMoveTest)
		{
			Board board("8/8/8/8/8/P7/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece pawn;
			pawn.color = WHITE;
			pawn.position = A3;
			pawn.type = PAWN;

			Move regularMove(PAWN, A3, A4);

			std::vector<Move> moves = getMovesForPawn(board, pawn);

			Assert::IsTrue(regularMove == moves.at(0));
		}

		TEST_METHOD(getStartPositionPawnMovesTest)
		{
			Board board("8/8/8/8/8/8/P7/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece pawn;
			pawn.color = WHITE;
			pawn.position = A2;
			pawn.type = PAWN;

			Move singleMove(PAWN, A2, A3);
			Move doubleMove(PAWN, A2, A4);
			std::vector<Move> expected = { singleMove, doubleMove };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCapturePawnMovesTest)
		{
			Board board("8/8/8/8/p1p5/1P6/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece pawn;
			pawn.color = WHITE;
			pawn.position = B3;
			pawn.type = PAWN;

			Move leftCapture(PAWN, B3, A4);
			Move regularMove(PAWN, B3, B4);
			Move rightCapture(PAWN, B3, C4);
			std::vector<Move> expected = { leftCapture, regularMove, rightCapture };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getPromotionPawnMovesTest)
		{
			Board board("P7/8/8/8/8/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece pawn;
			pawn.color = WHITE;
			pawn.position = A8;
			pawn.type = PAWN;

			Move promoteN(A8, KNIGHT);
			Move promoteB(A8, BISHOP);
			Move promoteR(A8, ROOK);
			Move promoteQ(A8, QUEEN);
			std::vector<Move> expected = { promoteN, promoteB, promoteR, promoteQ };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getEnPassantPawnMoveTest)
		{
			Board board("8/8/8/Pp6/8/8/8/8", WHITE, B6, true, true, true, true);

			piece pawn;
			pawn.color = WHITE;
			pawn.position = A5;
			pawn.type = PAWN;

			Move regular(PAWN, A5, A6);
			Move enPassant(PAWN, A5, B6, EN_PASSANT);
			std::vector<Move> expected = { regular, enPassant };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getKnightMovesTest)
		{
			Board board("8/8/8/8/3N4/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece knight;
			knight.color = WHITE;
			knight.position = D4;
			knight.type = KNIGHT;

			Move upUpRight(KNIGHT, D4, E6);
			Move rightRightUp(KNIGHT, D4, F5);
			Move rightRightDown(KNIGHT, D4, F3);
			Move downDownRight(KNIGHT, D4, E2);
			Move downDownLeft(KNIGHT, D4, C2);
			Move leftLeftDown(KNIGHT, D4, B3);
			Move leftLeftUp(KNIGHT, D4, B5);
			Move upUpLeft(KNIGHT, D4, C6);
			std::vector<Move> expected = { upUpRight, rightRightUp, rightRightDown, downDownRight, downDownLeft, leftLeftDown, leftLeftUp, upUpLeft };

			std::vector<Move> actual = getMovesForKnight(board, knight);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getRegularRookMovesTest)
		{
			Board board("8/8/8/8/3R4/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece rook;
			rook.color = WHITE;
			rook.position = D4;
			rook.type = ROOK;

			std::vector<Move> expected;
			for (int i = 1; i < 5; i++) {
				Move moveRank(ROOK, D4, (square)(D4 + i * RIGHT));
				expected.push_back(moveRank);
				Move moveFile(ROOK, D4, (square)(D4 + i * UP));
				expected.push_back(moveFile);
			}

			for (int i = 1; i < 4; i++) {
				Move moveRank(ROOK, D4, (square)(D4 + i * LEFT));
				expected.push_back(moveRank);
				Move moveFile(ROOK, D4, (square)(D4 + i * DOWN));
				expected.push_back(moveFile);
			}

			std::vector<Move> actual = getMovesForRook(board, rook);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCaptureRookMovesTest)
		{
			Board board("8/8/8/8/8/8/r7/R2r4", WHITE, SQUARE_NONE, true, true, true, true);

			piece rook;
			rook.color = WHITE;
			rook.position = A1;
			rook.type = ROOK;

			std::vector<Move> expected;
			Move moveFile(ROOK, A1, (square)(A1 + UP));
			expected.push_back(moveFile);
			for (int i = 1; i < 4; i++) {
				Move moveRank(ROOK, A1, (square)(A1 + i * RIGHT));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForRook(board, rook);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCollisionRookMovesTest)
		{
			Board board("8/8/8/8/8/8/R7/R2R4", WHITE, SQUARE_NONE, true, true, true, true);

			piece rook;
			rook.color = WHITE;
			rook.position = A1;
			rook.type = ROOK;

			std::vector<Move> expected;
			for (int i = 1; i < 3; i++) {
				Move moveRank(ROOK, A1, (square)(A1 + i * RIGHT));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForRook(board, rook);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getRegularBishopMovesTest) 
		{
			Board board("8/8/8/8/3B4/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			piece bishop;
			bishop.color = WHITE;
			bishop.position = D4;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 5; i++) {
				Move moveRightUp(BISHOP, D4, (square)(D4 + i * RIGHT_UP));
				expected.push_back(moveRightUp);
			}

			for (int i = 1; i < 4; i++) {
				Move moveLeftUp(BISHOP, D4, (square)(D4 + i * LEFT_UP));
				expected.push_back(moveLeftUp);
				Move moveLeftDown(BISHOP, D4, (square)(D4 + i * LEFT_DOWN));
				expected.push_back(moveLeftDown);
				Move moveRightDown(BISHOP, D4, (square)(D4 + i * RIGHT_DOWN));
				expected.push_back(moveRightDown);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCaptureBishopMovesTest)
		{
			Board board("8/8/8/8/3b4/8/8/B7", WHITE, SQUARE_NONE, true, true, true, true);

			piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 4; i++) {
				Move moveRank(BISHOP, A1, (square)(A1 + i * RIGHT_UP));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCollisionBishopMovesTest)
		{
			Board board("8/8/8/8/3B4/8/8/B7", WHITE, SQUARE_NONE, true, true, true, true);

			piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 3; i++) {
				Move moveRank(BISHOP, A1, (square)(A1 + i * RIGHT_UP));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getKingMovesTest)
		{
			Board board("8/8/8/8/8/8/1K6/8", WHITE, SQUARE_NONE, false, false, true, true);

			piece king;
			king.color = WHITE;
			king.position = B2;
			king.type = KING;

			Move up(KING, B2, B3);
			Move rightUp(KING, B2, C3);
			Move right(KING, B2, C2);
			Move rightDown(KING, B2, C1);
			Move down(KING, B2, B1);
			Move leftDown(KING, B2, A1);
			Move left(KING, B2, A2);
			Move leftUp(KING, B2, A3);
			std::vector<Move> expected = { up, rightUp, right, rightDown, down, leftDown, left, leftUp };

			std::vector<Move> actual = getMovesForKing(board, king);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCastlingMovesTest)
		{
			Board board1("8/8/8/8/8/8/8/R3K2R", WHITE, SQUARE_NONE, true, true, true, true);

			piece king;
			king.color = WHITE;
			king.position = E1;
			king.type = KING;

			Move castlingLeft1(KING, E1, C1, CASTLE_LEFT);
			Move castlingRight1(KING, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual1 = getMovesForKing(board1, king);

			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingLeft1) != actual1.end());
			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingRight1) != actual1.end());

			Board board2("8/8/8/r7/8/8/8/R3KB1R", WHITE, SQUARE_NONE, true, true, true, true);

			Move castlingLeft2(KING, E1, C1, CASTLE_LEFT);
			Move castlingRight2(KING, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual2 = getMovesForKing(board2, king);

			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingLeft2) == actual2.end());
			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingRight2) == actual2.end());
		}
	};
}