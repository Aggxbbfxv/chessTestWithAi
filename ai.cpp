#include "ai.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

int AI::nodeCount = 0;

const Piece::PieceColor aiColor = Piece::BLACK; //ai 설정은 검정!

// ... (PST 테이블 변수는 그대로 유지됩니다) ...
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

int AI::scoreMove(const Game& game, const Move& move)
{
    // MVV-LVA: Most Valuable Victim - Least Valuable Aggressor
    Piece* target = game.getPiece(move.toX, move.toY);
    int score = 0;

    if (target != nullptr)
    {
        Piece* attacker = game.getPiece(move.fromX, move.fromY);
        if (attacker != nullptr) {
             score = 10 * target->getValue() - attacker->getValue();
        }
    }
    return score;
}

int AI::eval(const Game& game)
{
    int totalScore = 0;
    // [최적화] getPiece 함수 호출 대신 m_board 직접 접근 (friend class 사용)
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.m_board[x][y];

            if(p != nullptr)
            {
                int pieceScore = p->getValue();
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

    // [최적화] Null Move Pruning (널 무브 가지치기)
    // 조건: 깊이가 충분하고(>=3), 현재 체크 상태가 아닐 때
    // "내가 한 수를 쉬어도 유리하다면(beta cut), 이 가지는 더 볼 필요가 없다"
    if (depth >= 3 && !state.isCheck(state.getCurrentTurn())) 
    {
        state.switchTurn(); // 턴 넘김 (Null Move)
        
        // R=2 (깊이를 2만큼 더 줄여서 탐색)
        // Minimax 구조이므로 maxing이면 minning 호출, minning이면 maxing 호출
        int score;
        if (maxing) 
            score = alphabeta(state, depth - 1 - 2, alpha, beta, false); 
        else 
            score = alphabeta(state, depth - 1 - 2, alpha, beta, true);
            
        state.switchTurn(); // 턴 복구

        if (maxing) {
            if (score >= beta) return beta; // Cut-off
        } else {
            if (score <= alpha) return alpha; // Cut-off
        }
    }

    vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

    // Move Ordering: MVV-LVA로 정렬
    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a) > scoreMove(state, b);
    });

    if(maxing)
    {
        int maxEval = -200000;
        for(const auto& move : allMoves)
        {
            UndoInfo undo = state.makeMove(move);
            
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false);

            state.unmakeMove(move, undo);

            maxEval = max(maxEval, evalScore);
            alpha = max(alpha, evalScore);

            if(beta <= alpha) break;
        }
        return maxEval;
    }
    else
    {
        int minEval = 200000;
        for(const auto& move : allMoves)
        {
            UndoInfo undo = state.makeMove(move);

            int evalScore = alphabeta(state, depth - 1, alpha, beta, true);

            state.unmakeMove(move, undo);

            minEval = min(minEval, evalScore);
            beta = min(beta, evalScore);

            if(beta <= alpha) break;
        }
        return minEval;
    }
}

// 루트 노드에서의 수와 점수를 저장하기 위한 구조체
struct MoveScore {
    Move move;
    int score;
};

Move AI::findBestMove(const Game& game)
{
    if(game.getCurrentTurn() != aiColor) return Move();

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;

    Game rootGame = game; 
    vector<Move> rootMoves = rootGame.generateMoves(aiColor);
    
    if(rootMoves.empty()) return Move();
    
    // 초기 정렬 (MVV-LVA)
    std::sort(rootMoves.begin(), rootMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(rootGame, a) > scoreMove(rootGame, b);
    });

    // 각 수의 평가 점수를 저장할 벡터
    std::vector<MoveScore> scoredMoves;
    for (const auto& m : rootMoves) scoredMoves.push_back({m, -200000});

    Move bestMoveSoFar;

    for (int currentDepth = 1; currentDepth <= searchDepth; ++currentDepth)
    {
        int alpha = -200000;
        int beta = 200000;
        
        // 이번 깊이(currentDepth)에서 모든 루트 수 탐색
        for (size_t i = 0; i < scoredMoves.size(); ++i)
        {
            Move move = scoredMoves[i].move;
            UndoInfo undo = rootGame.makeMove(move);

            // 재귀 호출 (AI가 수를 뒀으므로 다음은 상대방 턴, 즉 min-node)
            int score = alphabeta(rootGame, currentDepth - 1, alpha, beta, false);
            
            rootGame.unmakeMove(move, undo);

            // 루트 노드는 max-node 이므로, alpha 값을 업데이트
            alpha = std::max(alpha, score);

            // 점수 기록
            scoredMoves[i].score = score;
        }

        // [핵심] 점수 높은 순으로 정렬 (다음 깊이 탐색 시 좋은 수를 먼저 보기 위함)
        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
            return a.score > b.score;
        });

        bestMoveSoFar = scoredMoves[0].move;

        // (선택 사항) 시간이 너무 오래 걸리면 중단하는 로직을 여기에 추가 가능
        // if (timer.elapsed() > 5000) { 
        //     qDebug() << "Time limit exceeded, breaking at depth " << currentDepth;
        //     break; 
        // }
    }

    qDebug() << "========================================";
    qDebug() << "AI Iterative Depth:" << searchDepth; 
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; 
    qDebug() << "Nodes Visited:" << nodeCount;
    qDebug() << "Best Move Score:" << scoredMoves[0].score;
    qDebug() << "========================================";

    return bestMoveSoFar;
}