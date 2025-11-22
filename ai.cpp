#include "ai.h"
#include "piece.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

// --- Static Member Definitions ---
int AI::nodeCount = 0;
std::vector<TTEntry> AI::transpositionTable;

// --- Evaluation Constants ---
const int BISHOP_PAIR_BONUS = 50;
const int CASTLING_BONUS = 60; // Slightly increased bonus

// --- Piece-Square Tables (Corrected) ---
const int AI::pawnPST[8][8] = {
    {0,   0,   0,   0,   0,   0,   0,   0}, // Rank 1 (not reachable for pawns)
    {5,  10,  10, -20, -20,  10,  10,   5}, // Rank 2
    {5,  -5, -10,   0,   0, -10,  -5,   5}, // Rank 3
    {0,   0,   0,   20,  20,   0,   0,   0}, // Rank 4
    {5,   5,   10,  25,  25,  10,  5,   5}, // Rank 5
    {10,  10,  20,  30,  30,  20,  10,  10}, // Rank 6
    {50,  50,  50,  50,  50,  50,  50,  50}, // Rank 7
    {800, 800, 800, 800, 800, 800, 800, 800}  // Rank 8 (Promotion)
};
const int AI::knightPST[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50},
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};
const int AI::bishopPST[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};
const int AI::rookPST[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,  0},
    {5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {0,  0,  0,  5,  5,  0,  0,  0}
};
const int AI::queenPST[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    {-5,  0,  5,  5,  5,  5,  0, -5},
    {0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};
// King PST now defined from White's perspective (rank 1 = index 0)
const int AI::kingPST[8][8] = {
    {20, 30, 10,  0,  0, 10, 30, 20}, // Rank 1 (castled)
    {20, 20,  0,  0,  0,  0, 20, 20}, // Rank 2
    {-10,-20,-20,-20,-20,-20,-20,-10},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30}
};
const int king_endgame_pst[8][8] = {
    {-50,-30,-30,-30,-30,-30,-30,-50},
    {-30,-30,  0,  0,  0,  0,-30,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 30, 40, 40, 30,-10,-30},
    {-30,-10, 20, 30, 30, 20,-10,-30},
    {-30,-20,-10,  0,  0,-10,-20,-30},
    {-50,-40,-30,-20,-20,-30,-40,-50}
};

// --- Transposition Table Methods ---
void AI::clearTranspositionTable() {
    transpositionTable.assign(ttSize, {});
}

TTEntry* AI::probeTT(uint64_t hash) {
    TTEntry* entry = &transpositionTable[hash & (ttSize - 1)];
    if (entry->hash == hash) {
        return entry;
    }
    return nullptr;
}

void AI::storeTT(uint64_t hash, int depth, int score, TTEntry::flag_type flag) {
    TTEntry* entry = &transpositionTable[hash & (ttSize - 1)];
    // Always replace scheme. Could be improved with depth-preferred replacement.
    entry->hash = hash;
    entry->score = score;
    entry->flag = flag;
    entry->depth = depth;
}

// --- Evaluation Methods ---
bool AI::hasCastled(const Game& game, Chess::PieceColor color)
{
    // Heuristic: Check if the king is on a known castled square.
    // This is not perfectly robust but sufficient for an eval bonus.
    if (color == Chess::WHITE) {
        // Initial pos y=7. Castled y=7, x=2 or x=6.
        return game.m_wKingY == 7 && (game.m_wKingX == 2 || game.m_wKingX == 6);
    } else { // BLACK
        // Initial pos y=0. Castled y=0, x=2 or x=6.
        return game.m_bKingY == 0 && (game.m_bKingX == 2 || game.m_bKingX == 6);
    }
}

int AI::scoreMove(const Game& game, const Move& move)
{
    // MVV-LVA: Most Valuable Victim - Least Valuable Aggressor
    Piece* target = game.getPiece(move.toX, move.toY);
    if (target) {
        Piece* attacker = game.getPiece(move.fromX, move.fromY);
        return 10 * target->getValue() - attacker->getValue();
    }
    return 0;
}

int AI::eval(const Game& game, Chess::PieceColor colorToMax)
{
    int totalScore = 0;
    int whiteBishops = 0;
    int blackBishops = 0;
    int numMajorPieces = 0;

    for(int y = 0; y < 8; ++y) {
        for(int x = 0; x < 8; ++x) {
            Piece* p = game.m_board[x][y].get(); // Corrected indexing
            if(p) {
                if (p->getType() == Chess::QUEEN || p->getType() == Chess::ROOK) numMajorPieces++;
                if (p->getType() == Chess::BISHOP) {
                    if (p->getColor() == Chess::WHITE) whiteBishops++; else blackBishops++;
                }

                int pieceScore = p->getValue();
                // PSTs are defined from white's perspective (rank 1 = index 0).
                // White piece at y=7 (rank 1) should access index 0. Formula: 7-y.
                // Black piece at y=0 (rank 8) should access index 0. Formula: y.
                int pst_y = (p->getColor() == Chess::WHITE) ? (7-y) : y;

                switch (p->getType()) {
                    case Chess::PAWN:   pieceScore += pawnPST[pst_y][x]; break;
                    case Chess::BISHOP: pieceScore += bishopPST[pst_y][x]; break;
                    case Chess::KNIGHT: pieceScore += knightPST[pst_y][x]; break;
                    case Chess::ROOK:   pieceScore += rookPST[pst_y][x]; break;
                    case Chess::QUEEN:  pieceScore += queenPST[pst_y][x]; break;
                    case Chess::KING:
                        if (numMajorPieces <= 3) pieceScore += king_endgame_pst[pst_y][x];
                        else pieceScore += kingPST[pst_y][x];
                        break;
                    default: break;
                }
                totalScore += (p->getColor() == colorToMax) ? pieceScore : -pieceScore;
            }
        }
    }

    if (whiteBishops >= 2) totalScore += (colorToMax == Chess::WHITE ? BISHOP_PAIR_BONUS : -BISHOP_PAIR_BONUS);
    if (blackBishops >= 2) totalScore += (colorToMax == Chess::BLACK ? BISHOP_PAIR_BONUS : -BISHOP_PAIR_BONUS);
    if (hasCastled(game, Chess::WHITE)) totalScore += (colorToMax == Chess::WHITE ? CASTLING_BONUS : -CASTLING_BONUS);
    if (hasCastled(game, Chess::BLACK)) totalScore += (colorToMax == Chess::BLACK ? CASTLING_BONUS : -CASTLING_BONUS);

    return totalScore;
}

// --- Search Methods ---

int AI::quiescenceSearch(Game& state, int alpha, int beta, Chess::PieceColor aiColor)
{
    nodeCount++;
    uint64_t hash = state.currentHash;
    TTEntry* ttEntry = probeTT(hash);
    if (ttEntry) {
        // For quiescence, we only care about exact matches or bounds that cause a cutoff.
        if (ttEntry->flag == TTEntry::EXACT) return ttEntry->score;
        if (ttEntry->flag == TTEntry::LOWER_BOUND && ttEntry->score >= beta) return beta;
        if (ttEntry->flag == TTEntry::UPPER_BOUND && ttEntry->score <= alpha) return alpha;
    }

    int stand_pat = eval(state, aiColor);
    bool isMaxing = (state.getCurrentTurn() == aiColor);

    if (isMaxing) {
        if (stand_pat >= beta) return beta;
        alpha = std::max(alpha, stand_pat);
    } else {
        if (stand_pat <= alpha) return alpha;
        beta = std::min(beta, stand_pat);
    }

    std::vector<Move> moves = state.generateMoves(state.getCurrentTurn());
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a) > scoreMove(state, b);
    });

    for (const auto& move : moves) {
        if (state.getPiece(move.toX, move.toY) == nullptr && !move.isEnPassant()) continue;

        UndoInfo undo = state.makeMove(move);
        int score = quiescenceSearch(state, alpha, beta, aiColor);
        state.unmakeMove(move, undo);

        if (isMaxing) {
            alpha = std::max(alpha, score);
            if (alpha >= beta) { storeTT(hash, 0, beta, TTEntry::LOWER_BOUND); return beta; }
        } else {
            beta = std::min(beta, score);
            if (beta <= alpha) { storeTT(hash, 0, alpha, TTEntry::UPPER_BOUND); return alpha; }
        }
    }

    int finalScore = isMaxing ? alpha : beta;
    storeTT(hash, 0, finalScore, TTEntry::EXACT);
    return finalScore;
}


int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor)
{
    nodeCount++;
    uint64_t hash = state.currentHash;

    TTEntry* ttEntry = probeTT(hash);
    if (ttEntry && ttEntry->depth >= depth) {
        if (ttEntry->flag == TTEntry::EXACT) return ttEntry->score;
        if (ttEntry->flag == TTEntry::LOWER_BOUND) alpha = std::max(alpha, ttEntry->score);
        if (ttEntry->flag == TTEntry::UPPER_BOUND) beta = std::min(beta, ttEntry->score);
        if (alpha >= beta) return ttEntry->score;
    }

    Game::GameState gameState = state.getGameState();
    if (gameState != Game::IN_PROGRESS) {
        if (gameState == Game::CHECKMATE) return maxing ? (-100000 - depth) : (100000 + depth);
        return 0; // Draw
    }

    if (depth == 0) {
        return quiescenceSearch(state, alpha, beta, aiColor);
    }

    std::vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());
    if (allMoves.empty()) return 0; // Stalemate

    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a) > scoreMove(state, b);
    });

    int bestScore = maxing ? -200000 : 200000;
    TTEntry::flag_type flag = maxing ? TTEntry::UPPER_BOUND : TTEntry::LOWER_BOUND;

    for(const auto& move : allMoves) {
        UndoInfo undo = state.makeMove(move);
        int evalScore = alphabeta(state, depth - 1, alpha, beta, !maxing, aiColor);
        state.unmakeMove(move, undo);

        if (maxing) {
            if (evalScore > bestScore) {
                bestScore = evalScore;
                if (bestScore > alpha) {
                    alpha = bestScore;
                    flag = TTEntry::EXACT;
                }
            }
            if (alpha >= beta) { storeTT(hash, depth, beta, TTEntry::LOWER_BOUND); return beta; }
        } else {
            if (evalScore < bestScore) {
                bestScore = evalScore;
                if (bestScore < beta) {
                    beta = bestScore;
                    flag = TTEntry::EXACT;
                }
            }
            if (beta <= alpha) { storeTT(hash, depth, alpha, TTEntry::UPPER_BOUND); return alpha; }
        }
    }

    storeTT(hash, depth, bestScore, flag);
    return bestScore;
}

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
    clearTranspositionTable();

    Game rootGame = game;
    std::vector<Move> rootMoves = rootGame.generateMoves(aiColor);
    if(rootMoves.empty()) return Move();

    std::sort(rootMoves.begin(), rootMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(rootGame, a) > scoreMove(rootGame, b);
    });

    std::vector<MoveScore> scoredMoves;
    for (const auto& m : rootMoves) scoredMoves.push_back({m, -200000});

    Move bestMoveSoFar;

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

            if (timer.hasExpired(10000)) { // 10 second timeout
                 qDebug() << "Timeout at depth " << currentDepth;
                 break;
            }
            
            scoredMoves[i].score = score;
            
            // For the first move, the score becomes the lower bound (alpha) for subsequent searches.
            if (i == 0) {
                alpha = score;
            } else {
                alpha = std::max(alpha, score);
            }
        }

        // Sort moves based on the score from the current search depth
        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
            return a.score > b.score;
        });

        bestMoveSoFar = scoredMoves[0].move;
        qDebug() << "Depth" << currentDepth << "Best Move:" << QString::fromStdString(bestMoveSoFar.toSmithNotation()) << "Score:" << scoredMoves[0].score;


        if (timer.hasExpired(10000) || scoredMoves[0].score >= 100000) { // Stop if timeout or checkmate found
            break;
        }
    }

    qDebug() << "========================================";
    qDebug() << "AI Final Depth:" << searchDepth;
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms";
    qDebug() << "Nodes Visited:" << nodeCount;
    qDebug() << "Best Move Score:" << scoredMoves[0].score;
    qDebug() << "AI Moved:" << QString::fromStdString(bestMoveSoFar.toSmithNotation());
    qDebug() << "========================================";

    return bestMoveSoFar;
}
