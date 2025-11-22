#ifndef AI_H
#define AI_H

#include "game.h"
#include "move.h"
#include <unordered_map>

class AI
{
public:
    static const int searchDepth = 5;
    static Move findBestMove(const Game& game, Chess::PieceColor aiColor);

    static int nodeCount;

private:
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor);
    static int quiescence(Game& state, int alpha, int beta, bool maxing, Chess::PieceColor aiColor);

    static int eval(const Game& board, Chess::PieceColor colorToMax);
    
    // [추가] move ordering helper
    static int scoreMove(const Game& game, const Move& move);

    static const int pawnPST[8][8];
    static const int bishopPST[8][8];
    static const int knightPST[8][8];
    static const int rookPST[8][8];
    static const int queenPST[8][8];
    static const int kingPST[8][8];

    struct TTEntry {
        enum Flag { EXACT, LOWER, UPPER };
        int depth;
        int score;
        Move bestMove;
        Flag flag;
    };
    static std::unordered_map<uint64_t, TTEntry> transpositionTable;
};

#endif // AI_H
