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
			bitboard pawns = 0x10000;
			bitboard whitePieces = 0x10000;
			bitboard blackPieces = 0;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, blackPieces);

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
			bitboard pawns = 0x100;
			bitboard whitePieces = 0x100;
			bitboard blackPieces = 0;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, blackPieces);

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
			bitboard pawns = 0x5020000;
			bitboard whitePieces = 0x20000;
			bitboard blackPieces = 0x5000000;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, blackPieces);

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
			bitboard pawns = 0x100000000000000;
			bitboard whitePieces = 0x100000000000000;
			bitboard blackPieces = 0;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, blackPieces);

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
			bitboard pawns = 0x300000000;
			bitboard whitePieces = 0x100000000;
			bitboard blackPieces = 0x200000000;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, blackPieces);

			bitboard enPassantPosition = 0x20000000000;
			//board.setEnPassantSquare(enPassantPosition);

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
			bitboard knights = 0x8000000;
			bitboard whitePieces = 0x8000000;
			bitboard blackPieces = 0;
			Board board(0, knights, 0, 0, 0, 0, whitePieces, blackPieces);

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
			bitboard rooks = 0x8000000;
			bitboard whitePieces = 0x8000000;
			bitboard blackPieces = 0;
			Board board(0, 0, 0, rooks, 0, 0, whitePieces, blackPieces);

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
			bitboard rooks = 0x109;
			bitboard whitePieces = 0x1;
			bitboard blackPieces = 0x108;
			Board board(0, 0, 0, rooks, 0, 0, whitePieces, blackPieces);

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
			bitboard rooks = 0x109;
			bitboard whitePieces = 0x109;
			bitboard blackPieces = 0;
			Board board(0, 0, 0, rooks, 0, 0, whitePieces, blackPieces);

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
			bitboard bishops = 0x8000000;
			bitboard whitePieces = 0x8000000;
			bitboard blackPieces = 0;
			Board board(0, 0, bishops, 0, 0, 0, whitePieces, blackPieces);

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
			bitboard bishops = 0x8000001;
			bitboard whitePieces = 0x1;
			bitboard blackPieces = 0x8000000;
			Board board(0, 0, bishops, 0, 0, 0, whitePieces, blackPieces);

			piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = ROOK;

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
			bitboard bishops = 0x8000001;
			bitboard whitePieces = 0x8000001;
			bitboard blackPieces = 0;
			Board board(0, 0, bishops, 0, 0, 0, whitePieces, blackPieces);

			piece bishop;
			bishop.color = WHITE;
			bishop.position = A1;
			bishop.type = ROOK;

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
			bitboard kings = 0x200;
			bitboard whitePieces = 0x200;
			bitboard blackPieces = 0;
			Board board(0, 0, 0, 0, 0, kings, whitePieces, blackPieces);
			board.whiteCastle();

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
			bitboard kings = 0x10;
			bitboard rooks = 0x81;
			bitboard whitePieces = 0x91;
			bitboard blackPieces = 0;
			Board board1(0, 0, 0, rooks, 0, kings, whitePieces, blackPieces);

			piece king;
			king.color = WHITE;
			king.position = E1;
			king.type = KING;

			Move castlingLeft1(KING, E1, C1, CASTLE_LEFT);
			Move castlingRight1(KING, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual1 = getMovesForKing(board1, king);

			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingLeft1) != actual1.end());
			Assert::IsTrue(std::find(actual1.begin(), actual1.end(), castlingRight1) != actual1.end());

			kings = 0x10;
			rooks = 0x100000081;
			bitboard bishops = 0x20;
			whitePieces = 0xb1;
			blackPieces = 0x100000000;
			Board board2(0, 0, bishops, rooks, 0, kings, whitePieces, blackPieces);

			Move castlingLeft2(KING, E1, C1, CASTLE_LEFT);
			Move castlingRight2(KING, E1, G1, CASTLE_RIGHT);

			std::vector<Move> actual2 = getMovesForKing(board2, king);

			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingLeft2) == actual2.end());
			Assert::IsTrue(std::find(actual2.begin(), actual2.end(), castlingRight2) == actual2.end());
		}

		/*TEST_METHOD(getPseudoMovesForPawnTest)
		{
			bitboard pawns = 0x800003010000400;
			bitboard whitePieces = 0x800001010000400;
			bitboard blackPieces = 0x2000000000;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, blackPieces);

			std::vector<Move> moves = getMoves(board, WHITE);

			Assert::AreEqual(9, (int)moves.size());
		}*/

		/*TEST_METHOD(getPseudoMovesForRookPseudoMovesTest)
		{
			bitboard pawns = 0x800002010000400;
			bitboard rooks = 0x1000000000;
			bitboard whitePieces = 0x800001010000400;
			bitboard blackPieces = 0x2000000000;
			Board board(pawns, 0, rooks, 0, 0, 0, whitePieces, blackPieces);

			std::vector<Move> moves = getMoves(board, WHITE);
		}*/
	};
}