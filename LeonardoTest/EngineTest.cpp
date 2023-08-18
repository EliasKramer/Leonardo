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

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A3;
			pawn.type = PAWN;

			Move regularMove(&pawn, A3, A4);

			std::vector<Move> moves = getMovesForPawn(board, pawn);

			Assert::IsTrue(regularMove == moves.at(0));
		}

		TEST_METHOD(getStartPositionPawnMovesTest)
		{
			Board board("8/8/8/8/8/8/P7/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A2;
			pawn.type = PAWN;

			Move singleMove(&pawn, A2, A3);
			Move doubleMove(&pawn, A2, A4);
			std::vector<Move> expected = { singleMove, doubleMove };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCapturePawnMovesTest)
		{
			Board board("8/8/8/8/p1p5/1P6/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = B3;
			pawn.type = PAWN;

			Move leftCapture(&pawn, B3, A4);
			Move regularMove(&pawn, B3, B4);
			Move rightCapture(&pawn, B3, C4);
			std::vector<Move> expected = { leftCapture, regularMove, rightCapture };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getPromotionPawnMovesTest)
		{
			Board board("P7/8/8/8/8/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A8;
			pawn.type = PAWN;

			Move promoteN(&pawn, A8, KNIGHT);
			Move promoteB(&pawn, A8, BISHOP);
			Move promoteR(&pawn, A8, ROOK);
			Move promoteQ(&pawn, A8, QUEEN);
			std::vector<Move> expected = { promoteN, promoteB, promoteR, promoteQ };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getEnPassantPawnMoveTest)
		{
			Board board("8/8/8/Pp6/8/8/8/8", WHITE, B6, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A5;
			pawn.type = PAWN;

			Move regular(&pawn, A5, A6);
			Move enPassant(&pawn, A5, B6, EN_PASSANT);
			std::vector<Move> expected = { regular, enPassant };

			std::vector<Move> actual = getMovesForPawn(board, pawn);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getKnightMovesTest)
		{
			Board board("8/8/8/8/3N4/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece knight;
			knight.color = WHITE;
			knight.position = D4;
			knight.type = KNIGHT;

			Move upUpRight(&knight, D4, E6);
			Move rightRightUp(&knight, D4, F5);
			Move rightRightDown(&knight, D4, F3);
			Move downDownRight(&knight, D4, E2);
			Move downDownLeft(&knight, D4, C2);
			Move leftLeftDown(&knight, D4, B3);
			Move leftLeftUp(&knight, D4, B5);
			Move upUpLeft(&knight, D4, C6);
			std::vector<Move> expected = { upUpRight, rightRightUp, rightRightDown, downDownRight, downDownLeft, leftLeftDown, leftLeftUp, upUpLeft };

			std::vector<Move> actual = getMovesForKnight(board, knight);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getRegularRookMovesTest)
		{
			Board board("8/8/8/8/3R4/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = WHITE;
			rook.position = D4;
			rook.type = ROOK;

			std::vector<Move> expected;
			for (int i = 1; i < 5; i++) {
				Move moveRank(&rook, D4, (square)(D4 + i * RIGHT));
				expected.push_back(moveRank);
				Move moveFile(&rook, D4, (square)(D4 + i * UP));
				expected.push_back(moveFile);
			}

			for (int i = 1; i < 4; i++) {
				Move moveRank(&rook, D4, (square)(D4 + i * LEFT));
				expected.push_back(moveRank);
				Move moveFile(&rook, D4, (square)(D4 + i * DOWN));
				expected.push_back(moveFile);
			}

			std::vector<Move> actual = getMovesForRook(board, rook);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCaptureRookMovesTest)
		{
			Board board("8/8/8/8/8/8/r7/R2r4", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = WHITE;
			rook.position = A1;
			rook.type = ROOK;

			std::vector<Move> expected;
			Move moveFile(&rook, A1, (square)(A1 + UP));
			expected.push_back(moveFile);
			for (int i = 1; i < 4; i++) {
				Move moveRank(&rook, A1, (square)(A1 + i * RIGHT));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForRook(board, rook);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCollisionRookMovesTest)
		{
			Board board("8/8/8/8/8/8/R7/R2R4", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = WHITE;
			rook.position = A1;
			rook.type = ROOK;

			std::vector<Move> expected;
			for (int i = 1; i < 3; i++) {
				Move moveRank(&rook, A1, (square)(A1 + i * RIGHT));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForRook(board, rook);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getRegularBishopMovesTest) 
		{
			Board board("8/8/8/8/3B4/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece bishop;
			bishop.color = WHITE;
			bishop.position = D4;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 5; i++) {
				Move moveRightUp(&bishop, D4, (square)(D4 + i * RIGHT_UP));
				expected.push_back(moveRightUp);
			}

			for (int i = 1; i < 4; i++) {
				Move moveLeftUp(&bishop, D4, (square)(D4 + i * LEFT_UP));
				expected.push_back(moveLeftUp);
				Move moveLeftDown(&bishop, D4, (square)(D4 + i * LEFT_DOWN));
				expected.push_back(moveLeftDown);
				Move moveRightDown(&bishop, D4, (square)(D4 + i * RIGHT_DOWN));
				expected.push_back(moveRightDown);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCaptureBishopMovesTest)
		{
			Board board("8/8/8/8/3b4/8/8/B7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 4; i++) {
				Move moveRank(&bishop, A1, (square)(A1 + i * RIGHT_UP));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCollisionBishopMovesTest)
		{
			Board board("8/8/8/8/3B4/8/8/B7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 3; i++) {
				Move moveRank(&bishop, A1, (square)(A1 + i * RIGHT_UP));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getKingMovesTest)
		{
			Board board("8/8/8/8/8/8/1K6/8", WHITE, SQUARE_NONE, false, false, true, true);

			Piece king;
			king.color = WHITE;
			king.position = B2;
			king.type = KING;

			Move up(&king, B2, B3);
			Move rightUp(&king, B2, C3);
			Move right(&king, B2, C2);
			Move rightDown(&king, B2, C1);
			Move down(&king, B2, B1);
			Move leftDown(&king, B2, A1);
			Move left(&king, B2, A2);
			Move leftUp(&king, B2, A3);
			std::vector<Move> expected = { up, rightUp, right, rightDown, down, leftDown, left, leftUp };

			std::vector<Move> actual = getMovesForKing(board, king);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCastlingMovesTest)
		{
			Board board1("8/8/8/8/8/8/8/R3K2R", WHITE, SQUARE_NONE, true, true, true, true);

			Piece king;
			king.color = WHITE;
			king.position = E1;
			king.type = KING;

			Move castlingLeft1(&king, E1, C1, CASTLE_LEFT);
			Move castlingRight1(&king, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual1 = getMovesForKing(board1, king);

			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingLeft1) != actual1.end());
			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingRight1) != actual1.end());

			Board board2("8/8/8/r7/8/8/8/R3KB1R", WHITE, SQUARE_NONE, true, true, true, true);

			Move castlingLeft2(&king, E1, C1, CASTLE_LEFT);
			Move castlingRight2(&king, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual2 = getMovesForKing(board2, king);

			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingLeft2) == actual2.end());
			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingRight2) == actual2.end());
		}
	};
}