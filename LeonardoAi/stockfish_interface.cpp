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
    Stockfish::Search::clear();
    Stockfish::Eval::NNUE::init();
}

static double to_cp(Stockfish::Value v) { return double(v) / Stockfish::UCI::NormalizeToPawnValue; }

float stockfish_interface::eval(const std::string& fen, int depth) {

    Stockfish::StateListPtr states(new std::deque<Stockfish::StateInfo>(1));
    Stockfish::Position p;
    p.set(fen.c_str(), false, &states->back(), Stockfish::Threads.main());

    Stockfish::Search::LimitsType limits;
    limits.depth = depth;

    Stockfish::Threads.start_thinking(p, states, limits, false);

    Stockfish::Threads.main()->wait_for_search_finished();

    int cp_score = Stockfish::Threads.main()->rootMoves[0].score;

    float evaluation = static_cast<float>(cp_score) / 100.0;

    return evaluation;
}