#ifndef AI_H
#define AI_H

#include "game.h"
#include "move.h"

class AI
{
public:
    static const int searchDepth = 7;
    static Move findBestMove(const Game& game);

    static int nodeCount;

private:
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maxing);

    static int eval(const Game& board);

    static const int pawnPST[8][8];
    static const int bishopPST[8][8];
    static const int knightPST[8][8];
    static const int rookPST[8][8];
    static const int queenPST[8][8];
    static const int kingPST[8][8];
};

#endif // AI_H
