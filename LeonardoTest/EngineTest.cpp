#include <CppUnitTest.h>
#include <vector>
#include <algorithm>

#include <Engine.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace EngineTest
{
	TEST_CLASS(MoveGenerationTest)
	{
	public:
		TEST_METHOD(getMovesStartPosition)
		{
			std::string startPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
			Board board(startPositionFEN, WHITE, SQUARE_NONE, true, true, true, true);

			std::vector<Move> moves = getMoves(board);

			Assert::AreEqual(20, (int)moves.size());
		}

		TEST_METHOD(getNodesDepthTwo)
		{
			std::string startPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
			Board board(startPositionFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesForDepth(board, 2);

			Assert::AreEqual(400, nodes);
		}

		TEST_METHOD(getNodesDepthThree)
		{
			std::string startPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
			Board board(startPositionFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesForDepth(board, 3);

			Assert::AreEqual(8902, nodes);
		}

		TEST_METHOD(getNodesDepthFour)
		{
			std::string startPositionFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
			Board board(startPositionFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesForDepth(board, 4);

			Assert::AreEqual(197281, nodes);
		}

		TEST_METHOD(getMovesKiwipete)
		{
			std::string kiwipeteFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board(kiwipeteFEN, WHITE, SQUARE_NONE, true, true, true, true);

			std::vector<Move> moves = getMoves(board);

			Assert::AreEqual(48, (int)moves.size());
		}

		TEST_METHOD(getNodesDepthTwoKiwipete)
		{
			std::string kiwipeteFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board(kiwipeteFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesForDepth(board, 2);

			Assert::AreEqual(2039, nodes);
		}

		TEST_METHOD(getNodesOfTypeCastleDepthTwoKiwipete)
		{
			std::string kiwipeteFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board(kiwipeteFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesOfTypeForDepth(board, 2, CASTLE_LEFT) + getNodesOfTypeForDepth(board, 2, CASTLE_RIGHT);

			Assert::AreEqual(91, nodes);
		}

		TEST_METHOD(getNodesDepthThreeKiwipete)
		{
			std::string kiwipeteFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board(kiwipeteFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesForDepth(board, 3);

			Assert::AreEqual(97862, nodes);
		}

		TEST_METHOD(getNodesDepthFourKiwipete)
		{
			std::string kiwipeteFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board(kiwipeteFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesForDepth(board, 4);

			Assert::AreEqual(4085603, nodes);
		}

		TEST_METHOD(getNodesOfTypeCastleDepthThreeKiwipete)
		{
			std::string kiwipeteFEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board(kiwipeteFEN, WHITE, SQUARE_NONE, true, true, true, true);

			int nodes = getNodesOfTypeForDepth(board, 3, CASTLE_LEFT) + getNodesOfTypeForDepth(board, 3, CASTLE_RIGHT);

			Assert::AreEqual(3162, nodes);
		}
		
		TEST_METHOD(getNodesOfTypeCastleDepthOneTestPositionThree) 
		{
			std::string testPositionThreeFEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8";
			Board board(testPositionThreeFEN, WHITE, SQUARE_NONE, false, false, false, false);

			int nodes = getNodesOfTypeForDepth(board, 1, CASTLE_LEFT) + getNodesOfTypeForDepth(board, 1, CASTLE_RIGHT);

			Assert::AreEqual(0, nodes);
		}

		TEST_METHOD(getNodesDepthTwoTestPositionThree)
		{
			std::string testPositionThreeFEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8";
			Board board(testPositionThreeFEN, WHITE, SQUARE_NONE, false, false, false, false);

			int nodes = getNodesForDepth(board, 2);

			Assert::AreEqual(191, nodes);
		}


		TEST_METHOD(getNodesDepthThreeTestPositionThree)
		{
			std::string testPositionThreeFEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8";
			Board board(testPositionThreeFEN, WHITE, SQUARE_NONE, false, false, false, false);

			int nodes = getNodesForDepth(board, 3);

			Assert::AreEqual(2812, nodes);
		}

		TEST_METHOD(getNodesForDepthFourTestPositionThree) 
		{
			std::string testPositionThreeFEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8";
			Board board(testPositionThreeFEN, WHITE, SQUARE_NONE, false, false, false, false);

			int nodes = getNodesForDepth(board, 4);

			Assert::AreEqual(43238, nodes);
		}

		TEST_METHOD(getNodesForDepthFiveTestPositionThree)
		{
			std::string testPositionThreeFEN = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8";
			Board board(testPositionThreeFEN, WHITE, SQUARE_NONE, false, false, false, false);

			int nodes = getNodesForDepth(board, 5);

			Assert::AreEqual(674624, nodes);
		}

		TEST_METHOD(getNodesForDepthTwoTestPositionFour)
		{
			std::string testPositionFourFEN = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1";
			Board board(testPositionFourFEN, WHITE, SQUARE_NONE, false, false, true, true);

			int nodes = getNodesForDepth(board, 2);

			Assert::AreEqual(264, nodes);
		}

		TEST_METHOD(getNodesForDepthThreeTestPositionFour)
		{
			std::string testPositionFourFEN = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1";
			Board board(testPositionFourFEN, WHITE, SQUARE_NONE, false, false, true, true);

			int nodes = getNodesForDepth(board, 3);

			Assert::AreEqual(9467, nodes);
		}

		TEST_METHOD(getNodesForDepthFourTestPositionFour)
		{
			std::string testPositionFourFEN = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1";
			Board board(testPositionFourFEN, WHITE, SQUARE_NONE, false, false, true, true);

			int nodes = getNodesForDepth(board, 4);

			Assert::AreEqual(422333, nodes);
		}

		TEST_METHOD(getNodesDepthFourPositionFive) 
		{
			std::string testPositionFiveFEN = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R";
			Board board(testPositionFiveFEN, WHITE, SQUARE_NONE, true, true, false, false);

			int nodes = getNodesForDepth(board, 4);

			Assert::AreEqual(2103487, nodes);
		}

		TEST_METHOD(getNodesDepthFourPositionSix)
		{
			std::string testPositionSixFEN = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1";
			Board board(testPositionSixFEN, WHITE, SQUARE_NONE, false, false, false, false);

			int nodes = getNodesForDepth(board, 4);

			Assert::AreEqual(3894594, nodes);
		}
	};

	TEST_CLASS(EngineTest)
	{
	public:

		TEST_METHOD(getRegularPawnMoveTest)
		{
			Board board("8/8/8/8/8/P7/8/K7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A3;
			pawn.type = PAWN;

			Move regularMove(&pawn, 0, A3, A4);

			std::vector<Move> moves = getMovesForPawn(board, pawn, 0);

			Assert::IsTrue(regularMove == moves.at(0));
		}

		TEST_METHOD(getStartPositionPawnMovesTest)
		{
			Board board("8/8/8/8/8/8/P7/K7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A2;
			pawn.type = PAWN;

			Move singleMove(&pawn, 0, A2, A3);
			Move doubleMove(&pawn, 0, A2, A4);
			std::vector<Move> expected = { singleMove, doubleMove };

			std::vector<Move> actual = getMovesForPawn(board, pawn, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCapturePawnMovesTest)
		{
			Board board("8/8/8/8/p1p5/1P6/8/K7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = B3;
			pawn.type = PAWN;

			Move leftCapture(&pawn, 0, B3, A4);
			Move regularMove(&pawn, 0, B3, B4);
			Move rightCapture(&pawn, 0, B3, C4);
			std::vector<Move> expected = { leftCapture, regularMove, rightCapture };

			std::vector<Move> actual = getMovesForPawn(board, pawn, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getPromotionPawnMovesTest)
		{
			Board board("8/P7/8/8/8/8/8/K7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A7;
			pawn.type = PAWN;

			Move promoteN(&pawn, 0, A7, A8, KNIGHT);
			Move promoteB(&pawn, 0, A7, A8, BISHOP);
			Move promoteR(&pawn, 0, A7, A8, ROOK);
			Move promoteQ(&pawn, 0, A7, A8, QUEEN);
			std::vector<Move> expected = { promoteN, promoteB, promoteR, promoteQ };

			std::vector<Move> actual = getMovesForPawn(board, pawn, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getEnPassantPawnMoveTest)
		{
			Board board("8/8/8/Pp6/8/8/8/K7", WHITE, B6, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.position = A5;
			pawn.type = PAWN;

			Move regular(&pawn, 0, A5, A6);
			Move enPassant(&pawn, 0, A5, B6, EN_PASSANT);
			std::vector<Move> expected = { regular, enPassant };

			std::vector<Move> actual = getMovesForPawn(board, pawn, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getKnightMovesTest)
		{
			Board board("8/8/8/8/3N4/8/8/K7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece knight;
			knight.color = WHITE;
			knight.position = D4;
			knight.type = KNIGHT;

			Move upUpRight(&knight, 0, D4, E6);
			Move rightRightUp(&knight, 0, D4, F5);
			Move rightRightDown(&knight, 0, D4, F3);
			Move downDownRight(&knight, 0, D4, E2);
			Move downDownLeft(&knight, 0, D4, C2);
			Move leftLeftDown(&knight, 0, D4, B3);
			Move leftLeftUp(&knight, 0, D4, B5);
			Move upUpLeft(&knight, 0, D4, C6);
			std::vector<Move> expected = { upUpRight, rightRightUp, rightRightDown, downDownRight, downDownLeft, leftLeftDown, leftLeftUp, upUpLeft };

			std::vector<Move> actual = getMovesForKnight(board, knight, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getRegularRookMovesTest)
		{
			Board board("8/8/8/8/3R4/8/8/K7", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = WHITE;
			rook.position = D4;
			rook.type = ROOK;

			std::vector<Move> expected;
			for (int i = 1; i < 5; i++) {
				Move moveRank(&rook, 0, D4, (square)(D4 + i * RIGHT));
				expected.push_back(moveRank);
				Move moveFile(&rook, 0, D4, (square)(D4 + i * UP));
				expected.push_back(moveFile);
			}

			for (int i = 1; i < 4; i++) {
				Move moveRank(&rook, 0, D4, (square)(D4 + i * LEFT));
				expected.push_back(moveRank);
				Move moveFile(&rook, 0, D4, (square)(D4 + i * DOWN));
				expected.push_back(moveFile);
			}

			std::vector<Move> actual = getMovesForRook(board, rook, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCaptureRookMovesTest)
		{
			Board board("7K/8/8/8/8/8/r7/R2r4", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = WHITE;
			rook.position = A1;
			rook.type = ROOK;

			std::vector<Move> expected;
			Move moveFile(&rook, 0, A1, (square)(A1 + UP));
			expected.push_back(moveFile);
			for (int i = 1; i < 4; i++) {
				Move moveRank(&rook, 0, A1, (square)(A1 + i * RIGHT));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForRook(board, rook, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCollisionRookMovesTest)
		{
			Board board("8/8/8/8/8/8/R7/R2R3K", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = WHITE;
			rook.position = A1;
			rook.type = ROOK;

			std::vector<Move> expected;
			for (int i = 1; i < 3; i++) {
				Move moveRank(&rook, 0, A1, (square)(A1 + i * RIGHT));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForRook(board, rook, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getRegularBishopMovesTest) 
		{
			Board board("8/8/8/8/3B4/8/8/7K", WHITE, SQUARE_NONE, true, true, true, true);

			Piece bishop;
			bishop.color = WHITE;
			bishop.position = D4;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 5; i++) {
				Move moveRightUp(&bishop, 0, D4, (square)(D4 + i * RIGHT_UP));
				expected.push_back(moveRightUp);
			}

			for (int i = 1; i < 4; i++) {
				Move moveLeftUp(&bishop, 0, D4, (square)(D4 + i * LEFT_UP));
				expected.push_back(moveLeftUp);
				Move moveLeftDown(&bishop, 0, D4, (square)(D4 + i * LEFT_DOWN));
				expected.push_back(moveLeftDown);
				Move moveRightDown(&bishop, 0, D4, (square)(D4 + i * RIGHT_DOWN));
				expected.push_back(moveRightDown);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCaptureBishopMovesTest)
		{
			Board board("8/8/8/8/3b4/8/8/B6K", WHITE, SQUARE_NONE, true, true, true, true);

			Piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 4; i++) {
				Move moveRank(&bishop, 0, A1, (square)(A1 + i * RIGHT_UP));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop, 0);

			Assert::AreEqual(expected.size(), actual.size());
			Assert::IsTrue(std::is_permutation(expected.begin(), expected.end(), actual.begin()));
		}

		TEST_METHOD(getCollisionBishopMovesTest)
		{
			Board board("8/8/8/8/3B4/8/8/B6K", WHITE, SQUARE_NONE, true, true, true, true);

			Piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = BISHOP;

			std::vector<Move> expected;
			for (int i = 1; i < 3; i++) {
				Move moveRank(&bishop, 0, A1, (square)(A1 + i * RIGHT_UP));
				expected.push_back(moveRank);
			}

			std::vector<Move> actual = getMovesForBishop(board, bishop, 0);

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

			Move up(&king, 0, B2, B3);
			Move rightUp(&king, 0, B2, C3);
			Move right(&king, 0, B2, C2);
			Move rightDown(&king, 0, B2, C1);
			Move down(&king, 0, B2, B1);
			Move leftDown(&king, 0, B2, A1);
			Move left(&king, 0, B2, A2);
			Move leftUp(&king, 0, B2, A3);
			std::vector<Move> expected = { up, rightUp, right, rightDown, down, leftDown, left, leftUp };

			std::vector<Move> actual = getMovesForKing(board, king, 0);

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

			Move castlingLeft1(&king, 0, E1, C1, CASTLE_LEFT);
			Move castlingRight1(&king, 0, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual1 = getMovesForKing(board1, king, 0);

			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingLeft1) != actual1.end());
			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingRight1) != actual1.end());

			Board board2("8/8/8/2r5/8/8/8/R3KB1R", WHITE, SQUARE_NONE, true, true, true, true);

			Move castlingLeft2(&king, 0, E1, C1, CASTLE_LEFT);
			Move castlingRight2(&king, 0, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual2 = getMovesForKing(board2, king, 0);

			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingLeft2) == actual2.end());
			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingRight2) == actual2.end());
		}
	};
}