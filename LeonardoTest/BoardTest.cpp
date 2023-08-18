#include <CppUnitTest.h>
#include <Engine.h>
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace BoardTest
{
	TEST_CLASS(BoardTest)
	{
	public:
		BoardTest() {}

		TEST_METHOD(FENConstructorTest)
		{
			std::string FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
			Board board1(FEN, WHITE, SQUARE_NONE, true, true, true, true);

			Assert::AreEqual(0xffffULL, board1.getWhitePieces());
			Assert::AreEqual(0xffff000000000000ULL, board1.getBlackPieces());

			Assert::AreEqual(0xff00000000ff00ULL, board1.getPawns());
			Assert::AreEqual(0x4200000000000042ULL, board1.getKnights());
			Assert::AreEqual(0x2400000000000024ULL, board1.getBishops());
			Assert::AreEqual(0x8100000000000081ULL, board1.getRooks());
			Assert::AreEqual(0x800000000000008ULL, board1.getQueens());
			Assert::AreEqual(0x1000000000000010ULL, board1.getKings());

			Assert::AreEqual(0ULL, board1.getEnPassantSquare());

			FEN = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R";
			Board board2(FEN, WHITE, SQUARE_NONE, true, true, true, true);

			Assert::AreEqual(0xffffULL, board1.getWhitePieces());
			Assert::AreEqual(0xffff000000000000ULL, board1.getBlackPieces());

			Assert::AreEqual(0xff00000000ff00ULL, board1.getPawns());
			Assert::AreEqual(0x4200000000000042ULL, board1.getKnights());
			Assert::AreEqual(0x2400000000000024ULL, board1.getBishops());
			Assert::AreEqual(0x8100000000000081ULL, board1.getRooks());
			Assert::AreEqual(0x800000000000008ULL, board1.getQueens());
			Assert::AreEqual(0x1000000000000010ULL, board1.getKings());

			Assert::AreEqual(0ULL, board1.getEnPassantSquare());
		}

		TEST_METHOD(squareIsNotAttackedTest)
		{
			Board board(0, 0, 0, 0, 0, 0, 0, 0);
			Assert::IsFalse(board.squareIsAttackedBy(A1, WHITE));
		}

		TEST_METHOD(squareIsAttackedByPawnTest)
		{
			bitboard pawns = 0x100000;
			bitboard whitePieces = 0x100000;
			Board board(pawns, 0, 0, 0, 0, 0, whitePieces, 0);

			Assert::IsTrue(board.squareIsAttackedBy(D4, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(E4, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(F4, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(D4, BLACK));
			Assert::IsFalse(board.squareIsAttackedBy(D2, WHITE));
		}

		TEST_METHOD(squareIsAttackedByKnightTest)
		{
			bitboard knights = 0x10000000;
			bitboard whitePieces = 0x10000000;
			Board board(0, knights, 0, 0, 0, 0, whitePieces, 0);

			Assert::IsTrue(board.squareIsAttackedBy(D2, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(E2, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(F2, WHITE));

			Assert::IsTrue(board.squareIsAttackedBy(D6, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(E6, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(F6, WHITE));

			Assert::IsTrue(board.squareIsAttackedBy(C3, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(C4, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(C5, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(D2, BLACK));
			Assert::IsFalse(board.squareIsAttackedBy(D4, WHITE));
		}

		TEST_METHOD(squareIsAttackedByKingTest)
		{
			bitboard kings = 0x1;
			bitboard whitePieces = 0x1;
			Board board(0, 0, 0, 0, 0, kings, whitePieces, 0);

			Assert::IsTrue(board.squareIsAttackedBy(A2, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(B2, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(B1, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(A2, BLACK));

			Assert::IsFalse(board.squareIsAttackedBy(A3, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(A8, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(H8, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(D1, WHITE));
		}

		TEST_METHOD(squareIsAttackedByBishopTest)
		{
			bitboard bishops = 0x100000;
			bitboard whitePieces = 0x100000;
			Board board(0, 0, bishops, 0, 0, 0, whitePieces, 0);

			Assert::IsTrue(board.squareIsAttackedBy(C1, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(D2, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(F4, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(H6, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(G1, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(C5, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(E2, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(A3, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(D1, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(C1, BLACK));
		}

		TEST_METHOD(squareIsAttackedByRookTest)
		{
			bitboard rooks = 0x100000;
			bitboard whitePieces = 0x100000;
			Board board(0, 0, 0, rooks, 0, 0, whitePieces, 0);

			Assert::IsTrue(board.squareIsAttackedBy(E1, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(E2, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(A3, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(E8, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(G3, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(D4, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(F5, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(C1, WHITE));
		}


		TEST_METHOD(squareIsGuardedByOwnPieceTest)
		{
			bitboard rooks = 0x1000100000;
			bitboard whitePieces = 0x100000;
			bitboard blackPieces = 0x1000000000;
			Board board(0, 0, 0, rooks, 0, 0, whitePieces, blackPieces);
			
			Assert::IsFalse(board.squareIsAttackedBy(E6, WHITE));
		}

		TEST_METHOD(squareIsGuardedByEnemyPieceTest)
		{
			bitboard rooks = 0x100000;
			bitboard pawns = 0x1000000000;
			bitboard whitePieces = 0x1000100000;
			Board board(pawns, 0, 0, rooks, 0, 0, whitePieces, 0);

			Assert::IsFalse(board.squareIsAttackedBy(E6, WHITE));
		}
	};
}
