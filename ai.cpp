#include "ai.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

int AI::nodeCount = 0;

const Piece::PieceColor aiColor = Piece::BLACK; //ai 설정은 검정!

// ... (PST 테이블 변수들은 기존과 동일하므로 생략하지 않고 그대로 둡니다. 코드 적용시 위쪽 원본 코드의 배열들을 그대로 사용하세요.) ...
const int AI::pawnPST[8][8] = {
    {9000,  9000,   9000,   9000,   9000,   9000,   9000,   9000},
    {200,   200,    200,    200,    200,    200,    200,    200},
    {100,   100,    100,    100,    100,    100,    100,    100},
    {40,    40,     90,     100,    100,    90,     40,     40},
    {20,    20,     20,     100,    150,    20,     20,     20},
    {2,     4,      0,      15,     4,      0,      4,      2},
    {-10,   -10,    -10,    -20,    -35,    -10,    -10,    -10},
    {0,     0,      0,      0,      0,      0,      0,      0}
};

const int AI::knightPST[8][8] = {
    {-20, -80, -60, -60, -60, -60, -80, -20},
    {-80, -40,   0,   0,   0,   0, -40, -80},
    {-60,   0,  20,  30,  30,  20,   0, -60},
    {-60,  10,  30,  40,  40,  30,  10, -60},
    {-60,   0,  30,  40,  40,  30,   0, -60},
    {-60,  10,  20,  30,  30,  30,   1, -60},
    {-80, -40,   0,  10,  10,   0,  -4, -80},
    {-20, -80, -60, -60, -60, -60, -80, -20},
};

const int AI::bishopPST[8][8] = {
    {-40, -20, -20, -20, -20, -20, -20, -40},
    {-20,   0,   0,   0,   0,   0,   0, -20},
    {-20,   0,  10,  20,  20,  10,   0, -20},
    {-20,  10,  10,  20,  20,  10,  10, -20},
    {-20,   0,  20,  20,  20,  20,   0, -20},
    {-20,  20,  20,  20,  20,  20,  20, -20},
    {-20,  10,   0,   0,   0,   0,  10, -20},
    {-40, -20, -20, -20, -20, -20, -20, -40}
};

const int AI::rookPST[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,   0},
    {10, 20, 20, 20, 20, 20, 20,  10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-30, 30, 40, 10, 10,  0,  0, -30}
};

const int AI::queenPST[8][8] = {
    {-40, -20, -20, -10, -10, -20, -20, -40},
    {-20,   0,   0,   0,   0,   0,   0, -20},
    {-20,   0,  10,  10,  10,  10,   0, -20},
    {-10,   0,  10,  10,  10,  10,   0, -10},
    {0,   0,  10,  10,  10,  10,   0, -10},
    {-20,  10,  10,  10,  10,  10,   0, -20},
    {-20,   0,  10,   0,   0,   0,   0, -20},
    {-40, -20, -20, -10, -10, -20, -20, -40}
};

const int AI::kingPST[8][8] = {
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-40, -60, -60, -8, -80, -60, -60, -40},
    {-20, -40, -40, -40,-40, -40, -40, -20},
    {40,  40,   0,   0,  0,   0,  40,  40},
    {40,  60,  20,   0,  0,  20,  60,  40}
};

// [최적화] 수 정렬을 위한 점수 계산 함수 (MVV-LVA)
// 잡히는 기물(Victim)이 비쌀수록, 잡는 기물(Attacker)이 쌀수록 높은 점수를 줍니다.
int AI::scoreMove(const Game& game, const Move& move)
{
    Piece* target = game.getPiece(move.toX, move.toY);
    int score = 0;

    if (target != nullptr) // 캡처인 경우
    {
        Piece* attacker = game.getPiece(move.fromX, move.fromY);
        // 예: 폰(100)으로 퀸(900)을 잡으면: 10 * 900 - 100 = 8900 (아주 높은 우선순위)
        // 예: 퀸(900)으로 폰(100)을 잡으면: 10 * 100 - 900 = 100 (낮은 우선순위)
        if (attacker != nullptr) {
             score = 10 * target->getValue() - attacker->getValue();
        }
    }
    return score;
}

int AI::eval(const Game& game)
{
    int totalScore = 0;

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.getPiece(x,y);

            if(p != nullptr)
            {
                int pieceScore = 0;
                pieceScore += p->getValue();

                int pstY = (p->getColor() == Piece::WHITE) ? (7 - y) : y;

                switch (p->getType())
                {
                case Piece::PAWN: pieceScore += pawnPST[pstY][x]; break;
                case Piece::BISHOP: pieceScore += bishopPST[pstY][x]; break;
                case Piece::KNIGHT: pieceScore += knightPST[pstY][x]; break;
                case Piece::ROOK: pieceScore += rookPST[pstY][x]; break;
                case Piece::QUEEN: pieceScore += queenPST[pstY][x]; break;
                case Piece::KING: pieceScore += kingPST[pstY][x]; break;
                default: break;
                }

                if(p->getColor() == aiColor)
                    totalScore += pieceScore;
                else
                    totalScore -= pieceScore;
            }
        }
    }
    return totalScore;
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing)
{
    nodeCount++;

    if(depth == 0 || state.isGameOver())
    {
        return eval(state);
    }

    vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

    // [최적화 핵심] Move Ordering
    // 생성된 수들을 '점수가 높은 순서'로 정렬합니다.
    // 이렇게 하면 좋은 수를 먼저 탐색하게 되어, 베타 컷오프가 훨씬 빈번하게 발생합니다.
    // maxing이든 minning이든 "현재 턴을 잡은 쪽에서 좋은 수"를 먼저 봐야 합니다.
    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a) > scoreMove(state, b);
    });

    if(maxing)
    {
        int maxEval = -200000; // 안전한 무한대 값

        for(int i = 0; i < allMoves.size(); ++i)
        {
            Move move = allMoves[i];
            Piece* captured = state.makeMove(move);
            
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false);

            state.unmakeMove(move, captured);

            maxEval = max(maxEval, evalScore);
            alpha = max(alpha, evalScore);

            if(beta <= alpha)
            {
                break; // 가지치기
            }
        }
        return maxEval;
    }
    else
    {
        int minEval = 200000;

        for(int i = 0; i < allMoves.size(); ++i)
        {
            Move move = allMoves[i];
            Piece* captured = state.makeMove(move);

            int evalScore = alphabeta(state, depth - 1, alpha, beta, true);

            state.unmakeMove(move, captured);

            minEval = min(minEval, evalScore);
            beta = min(beta, evalScore);

            if(beta <= alpha)
            {
                break; // 가지치기
            }
        }
        return minEval;
    }
}

Move AI::findBestMove(const Game& game)
{
    if(game.getCurrentTurn() != aiColor)
    {
        return Move();
    }

    Move bestMove;
    int maxEval = -200000;

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;

    // 원본 보존을 위해 복사
    Game rootGame = game; 
    vector<Move> allMoves = rootGame.generateMoves(aiColor);

    if(allMoves.empty())
    {
        return Move();
    }

    // [최적화] 루트 노드에서도 정렬 수행
    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(rootGame, a) > scoreMove(rootGame, b);
    });

    for(int i = 0; i < allMoves.size(); ++i)
    {
        Move move = allMoves[i];

        Piece* captured = rootGame.makeMove(move);

        // searchDepth를 사용하여 재귀 호출
        int evalScore = alphabeta(rootGame, searchDepth - 1, -200000, 200000, false);

        rootGame.unmakeMove(move, captured);

        if(evalScore > maxEval)
        {
            maxEval = evalScore;
            bestMove = move;
        }
    }

    qDebug() << "========================================";
    qDebug() << "AI Search Depth:" << searchDepth;
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; 
    qDebug() << "Nodes Visited:" << nodeCount;
    qDebug() << "Best Move Score:" << maxEval;
    qDebug() << "========================================";

    return bestMove;
}