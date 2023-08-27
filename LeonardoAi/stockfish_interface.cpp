#include "stockfish_interface.hpp"

#include "../StockFish/bitboard.h"
#include "../StockFish/endgame.h"
#include "../StockFish/position.h"
#include "../StockFish/psqt.h"
#include "../StockFish/search.h"
#include "../StockFish/syzygy/tbprobe.h"
#include "../StockFish/thread.h"
#include "../StockFish/tt.h"
#include "../StockFish/uci.h"
#include "../StockFish/evaluate.h"

//using namespace Stockfish;

void stockfish_interface::init() {
    Stockfish::Options["Threads"] = 4;
    Stockfish::UCI::init(Stockfish::Options);
    Stockfish::Tune::init();
    Stockfish::PSQT::init();
    Stockfish::Bitboards::init();
    Stockfish::Position::init();
    Stockfish::Bitbases::init();
    Stockfish::Endgames::init();
    Stockfish::Threads.set(size_t(Stockfish::Options["Threads"]));
    Stockfish::Search::clear(); // After threads are up
    Stockfish::Eval::NNUE::init();
}

static double to_cp(Stockfish::Value v) { return double(v) / Stockfish::UCI::NormalizeToPawnValue; }

float stockfish_interface::eval(const std::string& fen, int depth) {

    Stockfish::StateListPtr states(new std::deque<Stockfish::StateInfo>(1));
    Stockfish::Position p;
    p.set(fen.c_str(), false, &states->back(), Stockfish::Threads.main());

    //Stockfish::Eval::NNUE::verify();

   
    // return to_cp(Stockfish::Eval::evaluate(p));

    // Set search depth
    Stockfish::Search::LimitsType limits;
    limits.depth = depth;

    // Start search
    Stockfish::Threads.start_thinking(p, states, limits, false);

    // Wait for the search to finish
    //Stockfish::Threads.main()->wait_for_search_finished();
    //wait 5s 
    //std::this_thread::sleep_for(std::chrono::seconds(5));

    Stockfish::Threads.main()->wait_for_search_finished();

    // Get the evaluation score from the best move
    int cp_score = Stockfish::Threads.main()->rootMoves[0].score;

    // Convert the centipawn score to a float value if needed
    float evaluation = static_cast<float>(cp_score) / 100.0;  // Assuming centipawn score is in hundredths

    return evaluation;
}