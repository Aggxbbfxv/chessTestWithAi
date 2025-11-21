#ifndef AI_H
#define AI_H

#include "game.h"
#include "move.h"

class AI
{
public:
    static const int searchDepth = 4; // [조정됨] 깊이 7은 일반적인 엔진에서 최적화 없이 매우 느립니다. 테스트를 위해 4~5 정도로 낮추고, 최적화 후 올리는 것을 권장합니다.
    static Move findBestMove(const Game& game);

    static int nodeCount;

private:
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maxing);

    static int eval(const Game& board);
    
    // [추가] 수의 가치를 매겨 정렬하기 위한 헬퍼 함수
    static int scoreMove(const Game& game, const Move& move);

    static const int pawnPST[8][8];
    static const int bishopPST[8][8];
    static const int knightPST[8][8];
    static const int rookPST[8][8];
    static const int queenPST[8][8];
    static const int kingPST[8][8];
};

#endif // AI_H
