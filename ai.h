#ifndef AI_H
#define AI_H

#include "game.h"
#include "move.h"
#include <unordered_map>

// Transposition Table Entry
struct TTEntry {
    uint64_t hash;
    int depth;
    int score;
    enum Flag { EXACT, LOWER_BOUND, UPPER_BOUND } flag;
    Move bestMove;
};

class AI
{
public:
    static const int searchDepth = 5;
    static Move findBestMove(const Game& game, Chess::PieceColor aiColor);

    static int nodeCount;

private:
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor, int ply);
    static int quiescence(Game& state, int alpha, int beta, bool maxing, Chess::PieceColor aiColor);

    static int eval(const Game& board, Chess::PieceColor colorToMax);
    
    static int scoreMove(const Game& game, const Move& move, const Move& hashMove);

    static const int pawnPST[8][8];
    static const int bishopPST[8][8];
    static const int knightPST[8][8];
    static const int rookPST[8][8];
    static const int queenPST[8][8];
    static const int kingPST[8][8];

    // Transposition table
    static std::unordered_map<uint64_t, TTEntry> transpositionTable;
    static const int TT_SIZE = 1000000;
};

#endif // AI_H
