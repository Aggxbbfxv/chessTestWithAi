#ifndef AI_H
#define AI_H

#include "game.h"
#include "move.h"
#include <vector>

// Transposition Table Entry
struct TTEntry {
    uint64_t hash;
    int depth;
    int score;
    enum { EXACT, LOWER_BOUND, UPPER_BOUND } flag;
    // Move bestMove; // For now, we omit storing the best move to keep it simple
};

class AI
{
public:
    static const int searchDepth = 6;
    static Move findBestMove(const Game& game, Chess::PieceColor aiColor);

    static int nodeCount;
    static void clearTranspositionTable();

private:
    // Search
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor);
    static int quiescenceSearch(Game& state, int alpha, int beta, Chess::PieceColor aiColor);

    // Evaluation
    static int eval(const Game& board, Chess::PieceColor colorToMax);
    static bool hasCastled(const Game& game, Chess::PieceColor color);

    // Move Ordering
    static int scoreMove(const Game& game, const Move& move);

    // Transposition Table
    static std::vector<TTEntry> transpositionTable;
    static const int ttSize = 1048576; // 2^20 entries, about 16MB
    static TTEntry* probeTT(uint64_t hash);
    static void storeTT(uint64_t hash, int depth, int score, TTEntry::flag_type flag);

    // Piece-Square Tables
    static const int pawnPST[8][8];
    static const int bishopPST[8][8];
    static const int knightPST[8][8];
    static const int rookPST[8][8];
    static const int queenPST[8][8];
    static const int kingPST[8][8];
};

#endif // AI_H
