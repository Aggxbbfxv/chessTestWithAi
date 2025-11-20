#include "ai.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

int AI::nodeCount = 0;

const Piece::PieceColor aiColor = Piece::BLACK; //ai 설정은 검정!

const int AI::pawnPST[8][8] = { //pst 테이블 가져온건데 폰만 이렇게 되어있는 이유는 탭키로 처음에 맞출려다가 나이트부터는 귀찮아서 복붙하니까 이미 맞춰져있더라고요
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

const int AI::kingPST[8][8] = { //킹은 나중에 후반전용 테이블 따로 만들거같아요
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-60, -80, -80, -2, -20, -80, -80, -60},
    {-40, -60, -60, -8, -80, -60, -60, -40},
    {-20, -40, -40, -40,-40, -40, -40, -20},
    {40,  40,   0,   0,  0,   0,  40,  40},
    {40,  60,  20,   0,  0,  20,  60,  40}
};

int AI::eval(const Game& game)
{
    int totalScore = 0; //총 점수에요

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.getPiece(x,y); //보드를 순회하면서 기물들 정보를 가져와요

            if(p != nullptr) //좌표에 기물이 있는 경우
            {
                int pieceScore = 0;

                pieceScore += p->getValue(); //기물의 점수를 더하고요

                int pstY = (p->getColor() == Piece::WHITE) ? (7 - y) : y; //흑백 구분에 따라 y좌표 반전!

                switch (p->getType()) //이거 만들면서 생각한건데요 PST 테이블에 있던거 그대로 박으면 몇, 몇 이런식으로 표현했을때 y, x가 되네요 허허
                { //아무튼 보드의 위치에 따라 점수를 다 더하게되죠
                case Piece::PAWN: pieceScore += pawnPST[pstY][x]; break;
                case Piece::BISHOP: pieceScore += bishopPST[pstY][x]; break;
                case Piece::KNIGHT: pieceScore += knightPST[pstY][x]; break;
                case Piece::ROOK: pieceScore += rookPST[pstY][x]; break;
                case Piece::QUEEN: pieceScore += queenPST[pstY][x]; break;
                case Piece::KING: pieceScore += kingPST[pstY][x]; break;
                default: break;
                }

                if(p->getColor() == aiColor) //인공지능이랑 같은 색인 경우 더하고
                {
                    totalScore += pieceScore;
                }
                else //다르면 점수를 깎죠
                {
                    totalScore -= pieceScore;
                }
            }
        }
    }
    return totalScore; //그렇게 더한걸 반환해요
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing) //여기가 알파베타 가지치기!
{// ai.h부분에서 깊이 조절가능해요ㅛ
    nodeCount++;

    if(depth == 0 || state.isGameOver()) //깊이가 0이거나 더 이상 움직일 기물이 없는 경우
    {
        return eval(state); //반환을 해요
    }

    vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

    if(maxing) //최대화를 해야하는 경우
    {
        int maxEval = -20000; //일단 인공지능 수업에서는 음의 무한대였지만 그냥 이렇게 해둘게요

        for(int i = 0; i < allMoves.size(); ++i) //모든 움직일수있는 그 경우 안에서
        {
            Move move = allMoves[i]; //그중에 하나를 들고와요

            Piece* captured = state.makeMove(move);
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false); //여기서 재귀적으로 호출을 해요 maxing이 true냐 false냐에 따라 그 인공지능에서 봤던 그 트리있잖아요 번갈아가게 되죠

            state.unmakeMove(move, captured);

            maxEval = max(maxEval, evalScore);
            alpha = max(alpha, evalScore);

            if(beta <= alpha)
            {
                break; //가지치ㄱ;
            }
        }
        return maxEval;
    }
    else //여기도 마찬가지로 이루어지죠
    {
        int minEval = 20000;

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
                break;
            }
        }
        return minEval;
    }
}

Move AI::findBestMove(const Game& game)
{
    if(game.getCurrentTurn() != aiColor) // ai가 아닐때 그냥 무시해요
    {
        return Move();
    }

    Move bestMove;

    int maxEval = -20000;

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;

    Game rootGame = game; //구조 변경으로 원본을 건들지 않기위해 복사본 생성
    vector<Move> allMoves = rootGame.generateMoves(aiColor);

    if(allMoves.empty())
    {
        return Move();
    }

    for(int i = 0; i < allMoves.size(); ++i)
    {
        Move move = allMoves[i];

        Piece* captured = rootGame.makeMove(move);

        int evalScore = alphabeta(rootGame, searchDepth - 1, -20000, 20000, false);

        rootGame.unmakeMove(move, captured);

        if(evalScore > maxEval)
        {
            maxEval = evalScore;
            bestMove = move;
        }
    }

    qDebug() << "========================================";
    qDebug() << "AI Search Depth:" << searchDepth;
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; // 걸린 시간 (밀리초)
    qDebug() << "Nodes Visited:" << nodeCount;            // 방문한 노드 수
    qDebug() << "========================================";

    return bestMove;
}
