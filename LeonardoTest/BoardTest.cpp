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
			Board board("8/8/8/8/8/4P3/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Assert::IsTrue(board.squareIsAttackedBy(D4, WHITE));
			Assert::IsFalse(board.squareIsAttackedBy(E4, WHITE));
			Assert::IsTrue(board.squareIsAttackedBy(F4, WHITE));

			Assert::IsFalse(board.squareIsAttackedBy(D4, BLACK));
			Assert::IsFalse(board.squareIsAttackedBy(D2, WHITE));
		}

		TEST_METHOD(squareIsAttackedByKnightTest)
		{
			Board board("8/8/8/8/4N3/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

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
			Board board("8/8/8/8/8/8/8/K7", WHITE, SQUARE_NONE, true, true, true, true);

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
			Board board("8/8/8/8/8/4B3/8/8", WHITE, SQUARE_NONE, true, true, true, true);

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
			Board board("8/8/8/8/8/4R3/8/8", WHITE, SQUARE_NONE, true, true, true, true);

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
			Board board("8/8/8/4r3/8/4R3/8/8", WHITE, SQUARE_NONE, true, true, true, true);
			
			Assert::IsFalse(board.squareIsAttackedBy(E6, WHITE));
		}

		TEST_METHOD(squareIsGuardedByEnemyPieceTest)
		{
			Board board("8/8/8/4P3/8/4R3/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Assert::IsFalse(board.squareIsAttackedBy(E6, WHITE));
		}

		TEST_METHOD(executeNormalMove)
		{
			Board board("8/8/8/8/8/8/3P4/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.type = PAWN;
			pawn.position = D2;

			std::vector<Piece> whitePieces = { pawn };
			board.setWhitePiecesList(whitePieces);

			Move move = Move(&pawn, 0, D2, D4);

			board.executeMove(move);
			std::string expected = "8/8/8/8/3P4/8/8/8";

			Assert::AreEqual((int)D4, (int)board.getWhitePiecesList().at(0).position);
			Assert::AreEqual(expected, board.getFEN());
		}

		TEST_METHOD(executeCaptureMove)
		{
			Board board("8/8/8/8/8/4p3/3P4/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.type = PAWN;
			pawn.position = D2;

			Piece enemyPawn;
			enemyPawn.color = BLACK;
			enemyPawn.type = PAWN;
			enemyPawn.position = E3;

			std::vector<Piece> whitePieces = { pawn };
			board.setWhitePiecesList(whitePieces);

			Move move = Move(&pawn, 0, D2, E3);

			board.executeMove(move);
			std::string expected = "8/8/8/8/8/4P3/8/8";

			Assert::AreEqual((int)E3, (int)board.getWhitePiecesList().at(0).position);
			Assert::IsFalse(board.getBlackPieces());
			Assert::IsTrue(board.getBlackPiecesList().empty());
			Assert::AreEqual(expected, board.getFEN());
		}

		TEST_METHOD(executeEnPassantMove)
		{
			Board board("8/8/8/3Pp3/8/8/8/8", WHITE, E6, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.type = PAWN;
			pawn.position = D5;

			Piece enemyPawn;
			enemyPawn.color = BLACK;
			enemyPawn.type = PAWN;
			enemyPawn.position = E5;

			std::vector<Piece> whitePieces = { pawn };
			board.setWhitePiecesList(whitePieces);

			Move move = Move(&pawn, 0, D5, E6, EN_PASSANT);

			board.executeMove(move);
			std::string expected = "8/8/4P3/8/8/8/8/8";

			Assert::AreEqual((int)E6, (int)board.getWhitePiecesList().at(0).position);
			Assert::IsFalse(board.getBlackPieces());
			Assert::IsTrue(board.getBlackPiecesList().empty());
			Assert::AreEqual(expected, board.getFEN());
		}

		TEST_METHOD(executePromotionMove)
		{
			Board board("8/3P4/8/8/8/8/8/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.type = PAWN;
			pawn.position = D7;

			std::vector<Piece> whitePieces = { pawn };
			board.setWhitePiecesList(whitePieces);

			Move move = Move(&pawn, 0, D7, D8, QUEEN);

			board.executeMove(move);
			std::string expected = "3Q4/8/8/8/8/8/8/8";

			Assert::AreEqual((int)D8, (int)board.getWhitePiecesList().at(0).position);
			Assert::AreEqual((int)QUEEN, (int)board.getWhitePiecesList().at(0).type);
			Assert::AreEqual(expected, board.getFEN());
		}

		TEST_METHOD(executeLeftCastlingMove)
		{
			Board board("8/8/8/8/8/8/8/R3K2R", WHITE, SQUARE_NONE, true, true, true, true);

			Piece king;
			king.color = WHITE;
			king.type = KING;
			king.position = E1;

			Piece leftRook;
			leftRook.color = WHITE;
			leftRook.type = ROOK;
			leftRook.position = A1;

			std::vector<Piece> whitePieces = { king, leftRook };
			board.setWhitePiecesList(whitePieces);

			Move move = Move(&king, 0, E1, C1, CASTLE_LEFT);

			board.executeMove(move);
			std::string expected = "8/8/8/8/8/8/8/2KR3R";

			Assert::AreEqual((int)C1, (int)board.getWhitePiecesList().at(0).position);
			Assert::AreEqual((int)D1, (int)board.getWhitePiecesList().at(1).position);
			Assert::AreEqual(expected, board.getFEN());
		}

		TEST_METHOD(executeRightCastlingMove)
		{
			Board board("r3k2r/8/8/8/8/8/8/8", BLACK, SQUARE_NONE, true, true, true, true);

			Piece king;
			king.color = BLACK;
			king.type = KING;
			king.position = E8;

			Piece rightRook;
			rightRook.color = BLACK;
			rightRook.type = ROOK;
			rightRook.position = H8;

			std::vector<Piece> blackPieces = { king, rightRook };
			board.setBlackPiecesList(blackPieces);

			Move move = Move(&king, 0, E8, G8, CASTLE_RIGHT);

			board.executeMove(move);
			std::string expected = "r4rk1/8/8/8/8/8/8/8";

			Assert::AreEqual((int)G8, (int)board.getBlackPiecesList().at(0).position);
			Assert::AreEqual((int)F8, (int)board.getBlackPiecesList().at(1).position);
			Assert::AreEqual(expected, board.getFEN());
		}

		TEST_METHOD(executeMoveMakesCastlingUnavailable)
		{
			Board board("8/8/8/8/r7/8/8/R3K2R", WHITE, SQUARE_NONE, true, true, true, true);

			Piece rook;
			rook.color = BLACK;
			rook.type = ROOK;
			rook.position = A4;

			Move move = Move(&rook, 0, A4, A1);

			board.executeMove(move);

			Assert::IsFalse(board.getWhiteLeftCastleAvailable());
		}
		
		TEST_METHOD(executeMoveMakesEnPassantAvailable)
		{
			Board board("8/8/8/8/8/8/3P4/8", WHITE, SQUARE_NONE, true, true, true, true);

			Piece pawn;
			pawn.color = WHITE;
			pawn.type = PAWN;
			pawn.position = D2;

			std::vector<Piece> whitePieces = { pawn };
			board.setWhitePiecesList(whitePieces);

			Move move = Move(&pawn, 0, D2, D4);

			board.executeMove(move);
			std::string FEN = "8/8/8/8/3P4/8/8/8";
			bitboard enPassant = 0x80000;

			Assert::AreEqual((int)D4, (int)board.getWhitePiecesList().at(0).position);
			Assert::AreEqual(FEN, board.getFEN());
			Assert::AreEqual(enPassant, board.getEnPassantSquare());
		}
	};
}
