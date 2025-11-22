#include "ai.h"
#include "piece.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <QElapsedTimer>
#include <QDebug>
#include <QString>

int AI::nodeCount = 0;
std::unordered_map<uint64_t, AI::TTEntry> AI::transpositionTable;

// Piece-square tables tuned on a simple centipawn scale
const int AI::pawnPST[8][8] = {
    {0,   0,   0,   0,   0,   0,   0,   0},
    {50, 50,  50,  50,  50,  50,  50,  50},
    {10, 10,  20,  30,  30,  20,  10,  10},
    {5,   5,  10,  25,  25,  10,   5,   5},
    {0,   0,   0,  20,  20,   0,   0,   0},
    {5,  -5, -10,   0,   0, -10,  -5,   5},
    {5,  10,  10, -20, -20,  10,  10,   5},
    {0,   0,   0,   0,   0,   0,   0,   0}
};

const int AI::knightPST[8][8] = {
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20,   0,   0,   0,   0, -20, -40},
    {-30,   0,  10,  15,  15,  10,   0, -30},
    {-30,   5,  15,  20,  20,  15,   5, -30},
    {-30,   0,  15,  20,  20,  15,   0, -30},
    {-30,   5,  10,  15,  15,  10,   5, -30},
    {-40, -20,   0,   5,   5,   0, -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50}
};

const int AI::bishopPST[8][8] = {
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10,   5,   0,   0,   0,   0,   5, -10},
    {-10,  10,  10,  10,  10,  10,  10, -10},
    {-10,   0,  10,  10,  10,  10,   0, -10},
    {-10,   5,   5,  10,  10,   5,   5, -10},
    {-10,   0,   5,  10,  10,   5,   0, -10},
    {-10,   0,   0,   0,   0,   0,   0, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20}
};

const int AI::rookPST[8][8] = {
    {0,   0,   0,   0,   0,   0,   0,   0},
    {5,  10,  10,  10,  10,  10,  10,   5},
    {-5,  0,   0,   0,   0,   0,   0,  -5},
    {-5,  0,   0,   0,   0,   0,   0,  -5},
    {-5,  0,   0,   0,   0,   0,   0,  -5},
    {-5,  0,   0,   0,   0,   0,   0,  -5},
    {-5,  0,   0,   0,   0,   0,   0,  -5},
    {0,   0,   0,   5,   5,   0,   0,   0}
};

const int AI::queenPST[8][8] = {
    {-20, -10, -10,  -5,  -5, -10, -10, -20},
    {-10,   0,   0,   0,   0,   0,   0, -10},
    {-10,   0,   5,   5,   5,   5,   0, -10},
    { -5,   0,   5,   5,   5,   5,   0,  -5},
    {  0,   0,   5,   5,   5,   5,   0,  -5},
    {-10,   5,   5,   5,   5,   5,   0, -10},
    {-10,   0,   5,   0,   0,   0,   0, -10},
    {-20, -10, -10,  -5,  -5, -10, -10, -20}
};

const int AI::kingPST[8][8] = {
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    { 20,  20,   0,   0,   0,   0,  20,  20},
    { 20,  30,  10,   0,   0,  10,  30,  20}
};

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

struct MoveScore {
    Move move;
    int score;
};

static inline bool sameMove(const Move& a, const Move& b)
{
    return a.fromX == b.fromX && a.fromY == b.fromY &&
           a.toX == b.toX && a.toY == b.toY &&
           a.promotionType == b.promotionType && a.moveType == b.moveType;
}

static QString moveToString(const Move& m)
{
    QChar fromFile('a' + m.fromX);
    QChar toFile('a' + m.toX);
    int fromRank = 8 - m.fromY;
    int toRank = 8 - m.toY;
    return QString("%1%2-%3%4").arg(fromFile).arg(fromRank).arg(toFile).arg(toRank);
}

int AI::scoreMove(const Game& game, const Move& move)
{
    Piece* attacker = game.getPiece(move.fromX, move.fromY);
    Piece* target = game.getPiece(move.toX, move.toY);
    if (move.moveType == Move::EN_PASSANT && attacker != nullptr) {
        int capturedPawnY = (attacker->getColor() == Chess::WHITE) ? move.toY + 1 : move.toY - 1;
        target = game.getPiece(move.toX, capturedPawnY);
    }

    int score = 0;
    if (target && attacker)
    {
        score += 10 * target->getValue() - attacker->getValue();
    }
    if (move.promotionType != Chess::EMPTY) {
        score += 800 + move.promotionType * 10;
    }
    if (move.moveType == Move::CASTLE_KS || move.moveType == Move::CASTLE_QS) {
        score += 50;
    }
    return score;
}

int AI::eval(const Game& game, Chess::PieceColor colorToMax)
{
    int whiteScore = 0;
    int blackScore = 0;
    int totalMaterial = 0;
    int whitePawnFiles[8] = {0};
    int blackPawnFiles[8] = {0};
    int whiteBishops = 0;
    int blackBishops = 0;

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            Piece* p = game.m_board[x][y].get();
            if (p) {
                if (p->getType() != Chess::KING) {
                    totalMaterial += p->getValue();
                }
                if (p->getType() == Chess::PAWN) {
                    if (p->getColor() == Chess::WHITE) whitePawnFiles[x]++;
                    else blackPawnFiles[x]++;
                } else if (p->getType() == Chess::BISHOP) {
                    if (p->getColor() == Chess::WHITE) whiteBishops++; else blackBishops++;
                }
            }
        }
    }

    bool endgame = totalMaterial <= 2400;

    for (int x = 0; x < 8; ++x) {
        for (int y = 0; y < 8; ++y) {
            Piece* p = game.m_board[x][y].get();
            if (!p) continue;

            int pstY = (p->getColor() == Chess::WHITE) ? y : 7 - y;
            int pieceScore = p->getValue();

            switch (p->getType())
            {
            case Chess::PAWN: pieceScore += pawnPST[pstY][x]; break;
            case Chess::BISHOP: pieceScore += bishopPST[pstY][x]; break;
            case Chess::KNIGHT: pieceScore += knightPST[pstY][x]; break;
            case Chess::ROOK: pieceScore += rookPST[pstY][x]; break;
            case Chess::QUEEN: pieceScore += queenPST[pstY][x]; break;
            case Chess::KING:
                pieceScore += endgame ? king_endgame_pst[pstY][x] : kingPST[pstY][x];
                if (!endgame) {
                    bool castled = (p->getColor() == Chess::WHITE) ?
                                   ((x == 6 || x == 2) && y == 7) :
                                   ((x == 6 || x == 2) && y == 0);
                    if (castled) pieceScore += 40;
                    else if (x >= 2 && x <= 5 && y == ((p->getColor() == Chess::WHITE) ? 7 : 0)) pieceScore += 5;
                    else pieceScore -= 20;
                }
                break;
            default: break;
            }

            // Encourage central control
            if (x >= 2 && x <= 5 && y >= 2 && y <= 5) {
                int centerBonus = 0;
                if (p->getType() == Chess::PAWN) centerBonus = 6;
                else if (p->getType() == Chess::KNIGHT || p->getType() == Chess::BISHOP) centerBonus = 12;
                else if (p->getType() == Chess::QUEEN) centerBonus = 6;
                pieceScore += centerBonus;
            }

            if (p->getColor() == Chess::WHITE)
                whiteScore += pieceScore;
            else
                blackScore += pieceScore;
        }
    }

    // Pawn structure penalties
    for (int file = 0; file < 8; ++file) {
        if (whitePawnFiles[file] > 1) whiteScore -= 12 * (whitePawnFiles[file] - 1);
        if (blackPawnFiles[file] > 1) blackScore -= 12 * (blackPawnFiles[file] - 1);

        if (whitePawnFiles[file] > 0) {
            bool left = (file > 0) && whitePawnFiles[file - 1] > 0;
            bool right = (file < 7) && whitePawnFiles[file + 1] > 0;
            if (!left && !right) whiteScore -= 10;
        }
        if (blackPawnFiles[file] > 0) {
            bool left = (file > 0) && blackPawnFiles[file - 1] > 0;
            bool right = (file < 7) && blackPawnFiles[file + 1] > 0;
            if (!left && !right) blackScore -= 10;
        }
    }

    // Bishop pair bonus
    if (whiteBishops >= 2) whiteScore += 30;
    if (blackBishops >= 2) blackScore += 30;

    return (colorToMax == Chess::WHITE) ? (whiteScore - blackScore) : (blackScore - whiteScore);
}

int AI::quiescence(Game& state, int alpha, int beta, bool maxing, Chess::PieceColor aiColor)
{
    nodeCount++;

    int standPat = eval(state, aiColor);
    if (maxing) {
        if (standPat >= beta) return standPat;
        alpha = std::max(alpha, standPat);
    } else {
        if (standPat <= alpha) return standPat;
        beta = std::min(beta, standPat);
    }

    std::vector<Move> moves = state.generateMoves(state.getCurrentTurn());
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a) > scoreMove(state, b);
    });

    if (maxing) {
        int best = standPat;
        for (const auto& move : moves) {
            Piece* target = state.getPiece(move.toX, move.toY);
            bool isCapture = (target != nullptr) || move.moveType == Move::EN_PASSANT || move.promotionType != Chess::EMPTY;
            if (!isCapture) continue;

            UndoInfo undo = state.makeMove(move);
            int score = quiescence(state, alpha, beta, false, aiColor);
            state.unmakeMove(move, undo);

            best = std::max(best, score);
            alpha = std::max(alpha, score);
            if (beta <= alpha) break;
        }
        return best;
    } else {
        int best = standPat;
        for (const auto& move : moves) {
            Piece* target = state.getPiece(move.toX, move.toY);
            bool isCapture = (target != nullptr) || move.moveType == Move::EN_PASSANT || move.promotionType != Chess::EMPTY;
            if (!isCapture) continue;

            UndoInfo undo = state.makeMove(move);
            int score = quiescence(state, alpha, beta, true, aiColor);
            state.unmakeMove(move, undo);

            best = std::min(best, score);
            beta = std::min(beta, score);
            if (beta <= alpha) break;
        }
        return best;
    }
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor)
{
    nodeCount++;

    Game::GameState gameState = state.getGameState();
    if (gameState != Game::IN_PROGRESS)
    {
        if (gameState == Game::CHECKMATE) {
            return maxing ? (-100000 - depth) : (100000 + depth);
        }
        return 0;
    }

    if(depth == 0)
    {
        return quiescence(state, alpha, beta, maxing, aiColor);
    }

    const uint64_t hash = state.currentHash;
    int originalAlpha = alpha;
    int originalBeta = beta;

    auto ttIt = transpositionTable.find(hash);
    if (ttIt != transpositionTable.end() && ttIt->second.depth >= depth) {
        const TTEntry& entry = ttIt->second;
        if (entry.flag == TTEntry::EXACT) {
            return entry.score;
        } else if (entry.flag == TTEntry::LOWER) {
            alpha = std::max(alpha, entry.score);
        } else if (entry.flag == TTEntry::UPPER) {
            beta = std::min(beta, entry.score);
        }
        if (alpha >= beta) {
            return entry.score;
        }
    }

    std::vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());
    Move ttBest;
    bool hasTTBest = (ttIt != transpositionTable.end());
    if (hasTTBest) ttBest = ttIt->second.bestMove;

    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b) {
        bool aBest = hasTTBest && sameMove(a, ttBest);
        bool bBest = hasTTBest && sameMove(b, ttBest);
        if (aBest != bBest) return aBest;
        return scoreMove(state, a) > scoreMove(state, b);
    });

    Move bestMoveLocal;
    int bestScore = maxing ? std::numeric_limits<int>::min() / 2 : std::numeric_limits<int>::max() / 2;

    for(const auto& move : allMoves)
    {
        UndoInfo undo = state.makeMove(move);
        int evalScore = alphabeta(state, depth - 1, alpha, beta, !maxing, aiColor);
        state.unmakeMove(move, undo);

        if(maxing)
        {
            if (evalScore > bestScore) {
                bestScore = evalScore;
                bestMoveLocal = move;
            }
            alpha = std::max(alpha, evalScore);
        }
        else
        {
            if (evalScore < bestScore) {
                bestScore = evalScore;
                bestMoveLocal = move;
            }
            beta = std::min(beta, evalScore);
        }

        if(beta <= alpha) break;
    }

    TTEntry entry;
    entry.depth = depth;
    entry.score = bestScore;
    entry.bestMove = bestMoveLocal;
    if (bestScore <= originalAlpha) entry.flag = TTEntry::UPPER;
    else if (bestScore >= originalBeta) entry.flag = TTEntry::LOWER;
    else entry.flag = TTEntry::EXACT;
    transpositionTable[hash] = entry;

    return bestScore;
}

Move AI::findBestMove(const Game& game, Chess::PieceColor aiColor)
{
    if(game.getCurrentTurn() != aiColor) return Move();

    QElapsedTimer timer;
    timer.start();
    nodeCount = 0;
    transpositionTable.clear();

    Game rootGame = game; 
    std::vector<Move> rootMoves = rootGame.generateMoves(aiColor);
    
    if(rootMoves.empty()) return Move();
    
    std::sort(rootMoves.begin(), rootMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(rootGame, a) > scoreMove(rootGame, b);
    });

    std::vector<MoveScore> scoredMoves;
    for (const auto& m : rootMoves) scoredMoves.push_back({m, std::numeric_limits<int>::min() / 2});

    Move bestMoveSoFar = scoredMoves[0].move;

    for (int currentDepth = 1; currentDepth <= searchDepth; ++currentDepth)
    {
        int alpha = -200000;
        int beta = 200000;
        
        for (size_t i = 0; i < scoredMoves.size(); ++i)
        {
            Move move = scoredMoves[i].move;
            UndoInfo undo = rootGame.makeMove(move);

            int score = alphabeta(rootGame, currentDepth - 1, alpha, beta, false, aiColor);
            
            rootGame.unmakeMove(move, undo);

            alpha = std::max(alpha, score);
            scoredMoves[i].score = score;
        }

        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
            return a.score > b.score;
        });

        bestMoveSoFar = scoredMoves[0].move;
        qDebug() << "[AI] depth" << currentDepth << "best" << scoredMoves[0].score << "move" << moveToString(bestMoveSoFar);
    }

    qDebug() << "========================================";
    qDebug() << "AI Iterative Depth:" << searchDepth; 
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; 
    qDebug() << "Nodes Visited:" << nodeCount;
    qDebug() << "Best Move Score:" << scoredMoves[0].score;
    qDebug() << "PV start move:" << moveToString(bestMoveSoFar);
    qDebug() << "========================================";

    return bestMoveSoFar;
}
