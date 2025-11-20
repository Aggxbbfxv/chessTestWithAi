#include "ai.h"

#include <QElapsedTimer>
#include <QDebug>

#include <algorithm>
#include <random>

int AI::nodeCount = 0;
const Piece::PieceColor AI::aiColor = Piece::BLACK;
std::unordered_map<uint64_t, AI::TTEntry> AI::transpositionTable;
bool AI::zobristInitialized = false;
uint64_t AI::zobristPiece[2][6][8][8];
uint64_t AI::zobristTurn = 0;
std::vector<std::vector<Move>> AI::killerMoves;
int AI::historyHeuristic[8][8][8][8] = {};

namespace
{
    constexpr int INF = 30000;

    bool isCapture(const Game& state, const Move& move)
    {
        return state.getPiece(move.toX, move.toY) != nullptr;
    }
}

// Piece-Square Tables
const int pawnPST[8][8] = {
    {9000,  9000,   9000,   9000,   9000,   9000,   9000,   9000},
    {200,   200,    200,    200,    200,    200,    200,    200},
    {100,   100,    100,    100,    100,    100,    100,    100},
    {40,    40,     90,     100,    100,    90,     40,     40},
    {20,    20,     20,     100,    150,    20,     20,     20},
    {2,     4,      0,      15,     4,      0,      4,      2},
    {-10,   -10,    -10,    -20,    -35,    -10,    -10,    -10},
    {0,     0,      0,      0,      0,      0,      0,      0}
};

const int knightPST[8][8] = {
    {-20, -80, -60, -60, -60, -60, -80, -20},
    {-80, -40,   0,   0,   0,   0, -40, -80},
    {-60,   0,  20,  30,  30,  20,   0, -60},
    {-60,  10,  30,  40,  40,  30,  10, -60},
    {-60,   0,  30,  40,  40,  30,   0, -60},
    {-60,  10,  20,  30,  30,  30,   1, -60},
    {-80, -40,   0,  10,  10,   0,  -4, -80},
    {-20, -80, -60, -60, -60, -60, -80, -20},
};

const int bishopPST[8][8] = {
    {-40, -20, -20, -20, -20, -20, -20, -40},
    {-20,   0,   0,   0,   0,   0,   0, -20},
    {-20,   0,  10,  20,  20,  10,   0, -20},
    {-20,   0,  20,  20,  20,  20,   0, -20},
    {-20,   0,  20,  20,  20,  20,   0, -20},
    {-20,   0,  10,  20,  20,  10,   0, -20},
    {-20,   0,   0,   0,   0,   0,   0, -20},
    {-40, -20, -20, -20, -20, -20, -20, -40}
};

const int rookPST[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,   0},
    {10, 20, 20, 20, 20, 20, 20,  10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-10,  0,  0,  0,  0,  0,  0, -10},
    {-30, 30, 40, 10, 10,  0,  0, -30}
};

const int queenPST[8][8] = {
    {-40, -20, -20, -10, -10, -20, -20, -40},
    {-20,   0,   0,   0,   0,   0,   0, -20},
    {-20,   0,  10,  10,  10,  10,   0, -20},
    {-10,   0,  10,  10,  10,  10,   0, -10},
    {0,   0,  10,  10,  10,  10,   0, -10},
    {-20,  10,  10,  10,  10,  10,   0, -20},
    {-20,   0,  10,   0,   0,   0,   0, -20},
    {-40, -20, -20, -10, -10, -20, -20, -40}
};

const int kingPST[8][8] = {
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
    int totalScore = 0;

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.getPiece(x,y);
            if(p == nullptr) continue;

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
    return totalScore;
}

int AI::scoreMove(const Game& state, const Move& move, Piece::PieceColor moverColor)
{
    const Piece* movingPiece = state.getPiece(move.fromX, move.fromY);
    const Piece* targetPiece = state.getPiece(move.toX, move.toY);

    int captureScore = 0;
    if (targetPiece)
    {
        captureScore = (targetPiece->getValue() * 10);
        if (movingPiece)
        {
            captureScore -= movingPiece->getValue();
        }
    }

    int centerDistance = (3 - std::abs(3 - move.toX)) + (3 - std::abs(3 - move.toY));

    int pawnPush = 0;
    if (movingPiece && movingPiece->getType() == Piece::PAWN)
    {
        pawnPush = (moverColor == Piece::WHITE) ? (move.fromY - move.toY) : (move.toY - move.fromY);
    }

    int historyScore = historyHeuristic[move.fromX][move.fromY][move.toX][move.toY];

    return captureScore * 100 + historyScore + centerDistance * 5 + pawnPush;
}

void AI::initZobrist()
{
    if (zobristInitialized) return;

    std::mt19937_64 rng(42); // deterministic for reproducibility
    std::uniform_int_distribution<uint64_t> dist;

    for (int color = 0; color < 2; ++color)
    {
        for (int piece = 0; piece < 6; ++piece)
        {
            for (int x = 0; x < 8; ++x)
            {
                for (int y = 0; y < 8; ++y)
                {
                    zobristPiece[color][piece][x][y] = dist(rng);
                }
            }
        }
    }

    zobristTurn = dist(rng);
    zobristInitialized = true;
}

uint64_t AI::computeHash(const Game& state)
{
    initZobrist();
    uint64_t h = 0;

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            Piece* p = state.getPiece(x, y);
            if (!p) continue;

            int colorIndex = (p->getColor() == Piece::WHITE) ? 0 : 1;
            int pieceIndex = static_cast<int>(p->getType()) - 1; // skip EMPTY
            if (pieceIndex >= 0 && pieceIndex < 6)
            {
                h ^= zobristPiece[colorIndex][pieceIndex][x][y];
            }
        }
    }

    if (state.getCurrentTurn() == Piece::WHITE)
    {
        h ^= zobristTurn;
    }

    return h;
}

void AI::storeKiller(int depth, const Move& move)
{
    if (depth >= static_cast<int>(killerMoves.size())) return;
    auto& killers = killerMoves[depth];
    if (killers.size() < 2)
    {
        killers.push_back(move);
        return;
    }

    if (!(killers[0].fromX == move.fromX && killers[0].fromY == move.fromY &&
          killers[0].toX == move.toX && killers[0].toY == move.toY))
    {
        killers[1] = killers[0];
        killers[0] = move;
    }
}

int AI::quiescence(Game& state, int alpha, int beta)
{
    bool maximizing = state.getCurrentTurn() == aiColor;
    int standPat = eval(state);

    if (maximizing)
    {
        if (standPat >= beta) return standPat;
        if (standPat > alpha) alpha = standPat;
    }
    else
    {
        if (standPat <= alpha) return standPat;
        if (standPat < beta) beta = standPat;
    }

    std::vector<Move> moves = state.generateMoves(state.getCurrentTurn());
    moves.erase(std::remove_if(moves.begin(), moves.end(), [&](const Move& m){ return !isCapture(state, m); }), moves.end());

    for (const auto& move : moves)
    {
        Piece* captured = state.makeMove(move);
        int score = quiescence(state, alpha, beta);
        state.unmakeMove(move, captured);

        if (maximizing)
        {
            if (score > alpha) alpha = score;
            if (alpha >= beta) return alpha;
        }
        else
        {
            if (score < beta) beta = score;
            if (beta <= alpha) return beta;
        }
    }
    return maximizing ? alpha : beta;
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maximizingPlayer)
{
    nodeCount++;

    uint64_t hash = computeHash(state);
    auto ttIt = transpositionTable.find(hash);
    if (ttIt != transpositionTable.end() && ttIt->second.depth >= depth)
    {
        const TTEntry& entry = ttIt->second;
        if (entry.type == EntryType::EXACT)
            return entry.value;
        else if (entry.type == EntryType::LOWER && entry.value > alpha)
            alpha = entry.value;
        else if (entry.type == EntryType::UPPER && entry.value < beta)
            beta = entry.value;
        if (alpha >= beta)
            return entry.value;
    }

    if (depth == 0 || state.isGameOver())
    {
        int val = quiescence(state, alpha, beta);
        return val;
    }

    std::vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

    auto orderMoves = [&](std::vector<Move>& moves, Piece::PieceColor moverColor)
    {
        std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b)
        {
            int killerBonusA = 0, killerBonusB = 0;
            if (depth < static_cast<int>(killerMoves.size()))
            {
                auto& killers = killerMoves[depth];
                if (!killers.empty() && a.fromX == killers[0].fromX && a.fromY == killers[0].fromY && a.toX == killers[0].toX && a.toY == killers[0].toY)
                    killerBonusA = 5000;
                if (killers.size() > 1 && a.fromX == killers[1].fromX && a.fromY == killers[1].fromY && a.toX == killers[1].toX && a.toY == killers[1].toY)
                    killerBonusA = 4000;

                if (!killers.empty() && b.fromX == killers[0].fromX && b.fromY == killers[0].fromY && b.toX == killers[0].toX && b.toY == killers[0].toY)
                    killerBonusB = 5000;
                if (killers.size() > 1 && b.fromX == killers[1].fromX && b.fromY == killers[1].fromY && b.toX == killers[1].toX && b.toY == killers[1].toY)
                    killerBonusB = 4000;
            }

            int scoreA = scoreMove(state, a, moverColor) + killerBonusA;
            int scoreB = scoreMove(state, b, moverColor) + killerBonusB;
            return scoreA > scoreB;
        });
    };

    Piece::PieceColor moverColor = state.getCurrentTurn();
    orderMoves(allMoves, moverColor);

    Move bestLocalMove;
    int originalAlpha = alpha;
    int originalBeta = beta;

    if (maximizingPlayer)
    {
        int value = -INF;
        for (const auto& move : allMoves)
        {
            Piece* captured = state.makeMove(move);
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false);
            state.unmakeMove(move, captured);

            if (evalScore > value)
            {
                value = evalScore;
                bestLocalMove = move;
            }
            if (evalScore > alpha) alpha = evalScore;
            if (beta <= alpha)
            {
                if (!isCapture(state, move))
                {
                    storeKiller(depth, move);
                    historyHeuristic[move.fromX][move.fromY][move.toX][move.toY] += depth * depth;
                }
                break;
            }
        }

        TTEntry entry;
        entry.depth = depth;
        entry.value = value;
        if (value <= originalAlpha) entry.type = EntryType::UPPER;
        else if (value >= originalBeta) entry.type = EntryType::LOWER;
        else entry.type = EntryType::EXACT;
        entry.bestMove = bestLocalMove;
        transpositionTable[hash] = entry;
        return value;
    }
    else
    {
        int value = INF;
        for (const auto& move : allMoves)
        {
            Piece* captured = state.makeMove(move);
            int evalScore = alphabeta(state, depth - 1, alpha, beta, true);
            state.unmakeMove(move, captured);

            if (evalScore < value)
            {
                value = evalScore;
                bestLocalMove = move;
            }
            if (evalScore < beta) beta = evalScore;
            if (beta <= alpha)
            {
                if (!isCapture(state, move))
                {
                    storeKiller(depth, move);
                    historyHeuristic[move.fromX][move.fromY][move.toX][move.toY] += depth * depth;
                }
                break;
            }
        }

        TTEntry entry;
        entry.depth = depth;
        entry.value = value;
        if (value <= originalAlpha) entry.type = EntryType::UPPER;
        else if (value >= originalBeta) entry.type = EntryType::LOWER;
        else entry.type = EntryType::EXACT;
        entry.bestMove = bestLocalMove;
        transpositionTable[hash] = entry;
        return value;
    }
}

Move AI::findBestMove(const Game& game)
{
    if(game.getCurrentTurn() != aiColor)
    {
        return Move();
    }

    Move bestMove;

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;
    transpositionTable.clear();

    killerMoves.assign(searchDepth + 2, {});
    std::fill(&historyHeuristic[0][0][0][0], &historyHeuristic[0][0][0][0] + 8*8*8*8, 0);

    Game rootGame = game;
    std::vector<Move> allMoves = rootGame.generateMoves(aiColor);
    if(allMoves.empty())
    {
        return Move();
    }

    for (int depth = 1; depth <= searchDepth; ++depth)
    {
        std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b)
        {
            return scoreMove(rootGame, a, aiColor) > scoreMove(rootGame, b, aiColor);
        });

        int currentBestEval = -INF;
        Move currentBestMove;

        for(const auto& move : allMoves)
        {
            Piece* captured = rootGame.makeMove(move);
            int evalScore = alphabeta(rootGame, depth - 1, -INF, INF, false);
            rootGame.unmakeMove(move, captured);

            if(evalScore > currentBestEval)
            {
                currentBestEval = evalScore;
                currentBestMove = move;
            }
        }

        bestMove = currentBestMove;

        // Principal Variation Ordering: move best to front
        auto it = std::find_if(allMoves.begin(), allMoves.end(), [&](const Move& m)
        {
            return m.fromX == bestMove.fromX && m.fromY == bestMove.fromY && m.toX == bestMove.toX && m.toY == bestMove.toY;
        });
        if (it != allMoves.end())
        {
            std::iter_swap(allMoves.begin(), it);
        }
    }

    qDebug() << "========================================";
    qDebug() << "AI Search Depth:" << searchDepth;
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms";
    qDebug() << "Nodes Visited:" << nodeCount;
    qDebug() << "========================================";

    return bestMove;
}