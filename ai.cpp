#include "ai.h"
#include "piece.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

int AI::nodeCount = 0;



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
    {-60, -80, -80, -20, -20, -80, -80, -60},
    {-60, -80, -80, -20, -20, -80, -80, -60},
    {-60, -80, -80, -20, -20, -80, -80, -60},
    {-60, -80, -80, -20, -20, -80, -80, -60},
    {-40, -60, -60, -80, -80, -60, -60, -40},
    {-20, -40, -40, -40, -40, -40, -40, -20},
    {40,  40,   0,   0,   0,   0,  40,  40},
    {40,  60,  20,   0,   0,  20,  60,  40}
};

// 엔드게임용 킹 PST (중앙으로 이동 장려)
const int king_endgame_pst[8][8] = {
    {-80, -60, -40, -20, -20, -40, -60, -80},
    {-60, -40, -20,   0,   0, -20, -40, -60},
    {-40, -20,  20,  40,  40,  20, -20, -40},
    {-20,   0,  40,  60,  60,  40,   0, -20},
    {-20,   0,  40,  60,  60,  40,   0, -20},
    {-40, -20,  20,  40,  40,  20, -20, -40},
    {-60, -40, -20,   0,   0, -20, -40, -60},
    {-80, -60, -40, -20, -20, -40, -60, -80}
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

int AI::eval(const Game& game, Chess::PieceColor colorToMax)
{
    int totalScore = 0;
    int numMajorPieces = 0; // 퀸, 룩 카운트

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.m_board[x][y].get();

            if(p != nullptr)
            {
                if (p->getType() == Chess::QUEEN || p->getType() == Chess::ROOK) {
                    numMajorPieces++;
                }

                int pieceScore = p->getValue();
                int pstY = (p->getColor() == Chess::WHITE) ? y : (7 - y);

                switch (p->getType())
                {
                case Chess::PAWN: pieceScore += pawnPST[pstY][x]; break;
                case Chess::BISHOP: pieceScore += bishopPST[pstY][x]; break;
                case Chess::KNIGHT: pieceScore += knightPST[pstY][x]; break;
                case Chess::ROOK: pieceScore += rookPST[pstY][x]; break;
                case Chess::QUEEN: pieceScore += queenPST[pstY][x]; break;
                case Chess::KING:
                    // 게임 단계에 따라 다른 PST 적용
                    if (numMajorPieces <= 3) { // 엔드게임으로 간주 (퀸 하나 또는 룩 두개 이하)
                        pieceScore += king_endgame_pst[pstY][x];
                    } else { // 미들게임
                        pieceScore += kingPST[pstY][x];
                    }
                    break;
                default: break;
                }

                if(p->getColor() == colorToMax)
                    totalScore += pieceScore;
                else
                    totalScore -= pieceScore;
            }
        }
    }
    return totalScore;
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor)
{
    nodeCount++;

    // 게임이 끝났는지 먼저 확인 (체크메이트, 스테일메이트, 무승부)
    Game::GameState gameState = state.getGameState();
    if (gameState != Game::IN_PROGRESS)
    {
        if (gameState == Game::CHECKMATE) {
            // 현재 턴의 플레이어가 체크메이트 당한 것임.
            // 만약 현재 턴이 AI(maxing) 차례인데 체크메이트라면 AI가 진 것이므로 최악의 점수.
            // 반대라면 AI가 이긴 것이므로 최고의 점수.
            // depth를 더해주는 이유: 더 빨리 이기는 수를 선호하게 만들기 위함.
            return maxing ? (-100000 - depth) : (100000 + depth);
        }
        // 스테일메이트 또는 다른 무승부 조건
        return 0;
    }

    // 깊이 제한에 도달하면 평가 함수 호출
    if(depth == 0)
    {
        return eval(state, aiColor);
    }

    std::vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

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
            
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false, aiColor);

            state.unmakeMove(move, undo);

            maxEval = std::max(maxEval, evalScore);
            alpha = std::max(alpha, evalScore);

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

            int evalScore = alphabeta(state, depth - 1, alpha, beta, true, aiColor);

            state.unmakeMove(move, undo);

            minEval = std::min(minEval, evalScore);
            beta = std::min(beta, evalScore);

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

Move AI::findBestMove(const Game& game, Chess::PieceColor aiColor)
{
    if(game.getCurrentTurn() != aiColor) return Move();

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;

    Game rootGame = game; 
    std::vector<Move> rootMoves = rootGame.generateMoves(aiColor);
    
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
            int score = alphabeta(rootGame, currentDepth - 1, alpha, beta, false, aiColor);
            
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
