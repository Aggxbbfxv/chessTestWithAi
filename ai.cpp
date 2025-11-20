#include "ai.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

int AI::nodeCount = 0; 

const Piece::PieceColor aiColor = Piece::BLACK; //ai ������ ����!

const int AI::pawnPST[8][8] = { //pst ���̺� �����°ǵ� ���� �̷��� �Ǿ��ִ� ������ ��Ű�� ó���� ������ٰ� ����Ʈ���ʹ� �����Ƽ� �����ϴϱ� �̹� �������ִ������
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

const int AI::kingPST[8][8] = { //ŷ�� ���߿� �Ĺ����� ���̺� ���� ����Ű��ƿ�
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
    int totalScore = 0; //�� ��������

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.getPiece(x,y); //���带 ��ȸ�ϸ鼭 �⹰�� ������ �����Ϳ�

            if(p != nullptr) //��ǥ�� �⹰�� �ִ� ���
            {
                int pieceScore = 0;

                pieceScore += p->getValue(); //�⹰�� ������ ���ϰ���

                int pstY = (p->getColor() == Piece::WHITE) ? (7 - y) : y; //��� ���п� ���� y��ǥ ����!

                switch (p->getType()) //�̰� ����鼭 �����Ѱǵ��� PST ���̺��� �ִ��� �״�� ������ ��, �� �̷������� ǥ�������� y, x�� �ǳ׿� ����
                { //�ƹ�ư ������ ��ġ�� ���� ������ �� ���ϰԵ���
                case Piece::PAWN: pieceScore += pawnPST[pstY][x]; break;
                case Piece::BISHOP: pieceScore += bishopPST[pstY][x]; break;
                case Piece::KNIGHT: pieceScore += knightPST[pstY][x]; break;
                case Piece::ROOK: pieceScore += rookPST[pstY][x]; break;
                case Piece::QUEEN: pieceScore += queenPST[pstY][x]; break;
                case Piece::KING: pieceScore += kingPST[pstY][x]; break;
                default: break;
                }

                if(p->getColor() == aiColor) //�ΰ������̶� ���� ���� ��� ���ϰ�
                {
                    totalScore += pieceScore;
                }
                else //�ٸ��� ������ ����
                {
                    totalScore -= pieceScore;
                }
            }
        }
    }
    return totalScore; //�׷��� ���Ѱ� ��ȯ�ؿ�
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing) //���Ⱑ ���ĺ�Ÿ ����ġ��!
{// ai.h�κп��� ���� ���������ؿ��
    nodeCount++;

    if(depth == 0 || state.isGameOver()) //���̰� 0�̰ų� �� �̻� ������ �⹰�� ���� ���
    {
        return eval(state); //��ȯ�� �ؿ�
    }

    vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

    if(maxing) //�ִ�ȭ�� �ؾ��ϴ� ���
    {
        int maxEval = -20000; //�ϴ� �ΰ����� ���������� ���� ���Ѵ뿴���� �׳� �̷��� �صѰԿ�

        for(int i = 0; i < allMoves.size(); ++i) //��� �����ϼ��ִ� �� ��� �ȿ���
        {
            Move move = allMoves[i]; //���߿� �ϳ��� ����Ϳ�

            Piece* captured = state.makeMove(move);
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false); //���⼭ ��������� ȣ���� �ؿ� maxing�� true�� false�Ŀ� ���� �� �ΰ����ɿ��� �ô� �� Ʈ�����ݾƿ� �����ư��� ����

            state.unmakeMove(move, captured);

            maxEval = max(maxEval, evalScore);
            alpha = max(alpha, evalScore);

            if(beta <= alpha)
            {
                break; //����ġ��;
            }
        }
        return maxEval;
    }
    else //���⵵ ���������� �̷������
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
    if(game.getCurrentTurn() != aiColor) // ai�� �ƴҶ� �׳� �����ؿ�
    {
        return Move();
    }

    Move bestMove;

    int maxEval = -20000;

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;

    Game rootGame = game; //���� �������� ������ �ǵ��� �ʱ����� ���纻 ����
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
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; // �ɸ� �ð� (�и���)
    qDebug() << "Nodes Visited:" << nodeCount;            // �湮�� ��� ��
    qDebug() << "========================================";

    return bestMove;
}
