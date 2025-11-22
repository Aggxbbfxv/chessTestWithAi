#include "ai.h"
#include "piece.h"
#include <vector>
#include <algorithm>
#include <QElapsedTimer>
#include <QDebug>

int AI::nodeCount = 0;
std::unordered_map<uint64_t, TTEntry> AI::transpositionTable;

// FIXED: Pawn PST - rows represent ranks 1-8 (index 0 = rank 1, index 7 = rank 8)
const int AI::pawnPST[8][8] = {
    {0,     0,      0,      0,      0,      0,      0,      0},      // Rank 1 (back rank)
    {50,    50,     50,     50,     50,     50,     50,     50},     // Rank 2
    {10,    10,     20,     30,     30,     20,     10,     10},     // Rank 3
    {5,     5,      10,     25,     25,     10,     5,      5},      // Rank 4
    {0,     0,      0,      20,     20,     0,      0,      0},      // Rank 5
    {5,     -5,     -10,    0,      0,      -10,    -5,     5},      // Rank 6
    {5,     10,     10,     -20,    -20,    10,     10,     5},      // Rank 7
    {0,     0,      0,      0,      0,      0,      0,      0}       // Rank 8 (promotion rank)
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
    {-10,   0,   0,   0,   0,   0,   0, -10},
    {-10,   0,   5,  10,  10,   5,   0, -10},
    {-10,   5,   5,  10,  10,   5,   5, -10},
    {-10,   0,  10,  10,  10,  10,   0, -10},
    {-10,  10,  10,  10,  10,  10,  10, -10},
    {-10,   5,   0,   0,   0,   0,   5, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20}
};

const int AI::rookPST[8][8] = {
    {0,  0,  0,  0,  0,  0,  0,   0},
    {5, 10, 10, 10, 10, 10, 10,   5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {0,  0,  0,  5,  5,  0,  0,  0}
};

const int AI::queenPST[8][8] = {
    {-20, -10, -10, -5, -5, -10, -10, -20},
    {-10,   0,   0,  0,  0,   0,   0, -10},
    {-10,   0,   5,  5,  5,   5,   0, -10},
    {-5,    0,   5,  5,  5,   5,   0, -5},
    {0,     0,   5,  5,  5,   5,   0, -5},
    {-10,   5,   5,  5,  5,   5,   0, -10},
    {-10,   0,   5,  0,  0,   0,   0, -10},
    {-20, -10, -10, -5, -5, -10, -10, -20}
};

const int AI::kingPST[8][8] = {
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    {20,  20,   0,   0,   0,   0,  20,  20},
    {20,  30,  10,   0,   0,  10,  30,  20}
};

const int king_endgame_pst[8][8] = {
    {-50, -40, -30, -20, -20, -30, -40, -50},
    {-30, -20, -10,   0,   0, -10, -20, -30},
    {-30, -10,  20,  30,  30,  20, -10, -30},
    {-30, -10,  30,  40,  40,  30, -10, -30},
    {-30, -10,  30,  40,  40,  30, -10, -30},
    {-30, -10,  20,  30,  30,  20, -10, -30},
    {-30, -30,   0,   0,   0,   0, -30, -30},
    {-50, -30, -30, -30, -30, -30, -30, -50}
};

int AI::scoreMove(const Game& game, const Move& move, const Move& hashMove)
{
    // Prioritize hash move
    if (move.fromX == hashMove.fromX && move.fromY == hashMove.fromY &&
        move.toX == hashMove.toX && move.toY == hashMove.toY) {
        return 1000000;
    }

    // MVV-LVA: Most Valuable Victim - Least Valuable Aggressor
    Piece* target = game.getPiece(move.toX, move.toY);
    int score = 0;

    if (target != nullptr)
    {
        Piece* attacker = game.getPiece(move.fromX, move.fromY);
        if (attacker != nullptr) {
             score = 10000 + 10 * target->getValue() - attacker->getValue();
        }
    }
    return score;
}

int AI::eval(const Game& game, Chess::PieceColor colorToMax)
{
    int totalScore = 0;
    int whiteMaterial = 0;
    int blackMaterial = 0;

    // First pass: count material for endgame detection
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.m_board[x][y].get();
            if(p != nullptr)
            {
                int pieceValue = p->getValue();
                if (p->getColor() == Chess::WHITE) {
                    whiteMaterial += pieceValue;
                } else {
                    blackMaterial += pieceValue;
                }
            }
        }
    }

    // Determine if we're in endgame (less than 1300 material per side including king)
    bool isEndgame = (whiteMaterial < 21300 && blackMaterial < 21300);

    // Second pass: evaluate with PST
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = game.m_board[x][y].get();

            if(p != nullptr)
            {
                int pieceScore = p->getValue();
                
                // PST indexing: for White use y directly, for Black use (7-y) to mirror
                int pstY = (p->getColor() == Chess::WHITE) ? y : (7 - y);

                switch (p->getType())
                {
                case Chess::PAWN: pieceScore += pawnPST[pstY][x]; break;
                case Chess::BISHOP: pieceScore += bishopPST[pstY][x]; break;
                case Chess::KNIGHT: pieceScore += knightPST[pstY][x]; break;
                case Chess::ROOK: pieceScore += rookPST[pstY][x]; break;
                case Chess::QUEEN: pieceScore += queenPST[pstY][x]; break;
                case Chess::KING:
                    if (isEndgame) {
                        pieceScore += king_endgame_pst[pstY][x];
                    } else {
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

int AI::quiescence(Game& state, int alpha, int beta, bool maxing, Chess::PieceColor aiColor)
{
    nodeCount++;

    int standPat = eval(state, aiColor);

    if(maxing)
    {
        if(standPat >= beta) return beta;
        if(alpha < standPat) alpha = standPat;
    }
    else
    {
        if(standPat <= alpha) return alpha;
        if(beta > standPat) beta = standPat;
    }

    std::vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());
    
    // Only search captures
    std::vector<Move> captures;
    for(const auto& move : allMoves)
    {
        if(state.getPiece(move.toX, move.toY) != nullptr)
        {
            captures.push_back(move);
        }
    }

    // Sort captures by MVV-LVA
    std::sort(captures.begin(), captures.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a, Move()) > scoreMove(state, b, Move());
    });

    if(maxing)
    {
        for(const auto& move : captures)
        {
            UndoInfo undo = state.makeMove(move);
            int score = quiescence(state, alpha, beta, false, aiColor);
            state.unmakeMove(move, undo);

            if(score >= beta) return beta;
            if(score > alpha) alpha = score;
        }
        return alpha;
    }
    else
    {
        for(const auto& move : captures)
        {
            UndoInfo undo = state.makeMove(move);
            int score = quiescence(state, alpha, beta, true, aiColor);
            state.unmakeMove(move, undo);

            if(score <= alpha) return alpha;
            if(score < beta) beta = score;
        }
        return beta;
    }
}

int AI::alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor, int ply)
{
    nodeCount++;

    // Check transposition table
    uint64_t currentHash = state.currentHash;
    auto it = transpositionTable.find(currentHash);
    if(it != transpositionTable.end() && it->second.depth >= depth)
    {
        TTEntry& entry = it->second;
        if(entry.flag == TTEntry::EXACT) {
            return entry.score;
        }
        else if(entry.flag == TTEntry::LOWER_BOUND) {
            alpha = std::max(alpha, entry.score);
        }
        else if(entry.flag == TTEntry::UPPER_BOUND) {
            beta = std::min(beta, entry.score);
        }
        if(alpha >= beta) {
            return entry.score;
        }
    }

    // Check game state
    Game::GameState gameState = state.getGameState();
    if (gameState != Game::IN_PROGRESS)
    {
        if (gameState == Game::CHECKMATE) {
            // FIXED: If current turn player is in checkmate, they lose
            // If it's AI's turn (maxing matches aiColor == current turn), AI loses -> very negative
            // If it's opponent's turn, opponent loses -> very positive
            Chess::PieceColor currentTurn = state.getCurrentTurn();
            if (currentTurn == aiColor) {
                return -100000 + ply; // AI is checkmated, prefer later checkmates (more chances for opponent to err)
            } else {
                return 100000 - ply; // Opponent is checkmated, prefer sooner checkmates
            }
        }
        return 0; // Draw
    }

    // At depth 0, enter quiescence search
    if(depth == 0)
    {
        return quiescence(state, alpha, beta, maxing, aiColor);
    }

    std::vector<Move> allMoves = state.generateMoves(state.getCurrentTurn());

    // Get hash move for ordering
    Move hashMove;
    if(it != transpositionTable.end()) {
        hashMove = it->second.bestMove;
    }

    // Move Ordering
    std::sort(allMoves.begin(), allMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(state, a, hashMove) > scoreMove(state, b, hashMove);
    });

    Move bestMove;
    TTEntry::Flag flag = TTEntry::UPPER_BOUND;

    if(maxing)
    {
        int maxEval = -200000;
        for(const auto& move : allMoves)
        {
            UndoInfo undo = state.makeMove(move);
            
            int evalScore = alphabeta(state, depth - 1, alpha, beta, false, aiColor, ply + 1);

            state.unmakeMove(move, undo);

            if(evalScore > maxEval) {
                maxEval = evalScore;
                bestMove = move;
            }
            alpha = std::max(alpha, evalScore);

            if(beta <= alpha) {
                flag = TTEntry::LOWER_BOUND;
                break;
            }
        }
        if(alpha > -200000 && flag != TTEntry::LOWER_BOUND) {
            flag = TTEntry::EXACT;
        }
        
        // Store in transposition table
        if(transpositionTable.size() < TT_SIZE) {
            transpositionTable[currentHash] = {currentHash, depth, maxEval, flag, bestMove};
        }
        
        return maxEval;
    }
    else
    {
        int minEval = 200000;
        for(const auto& move : allMoves)
        {
            UndoInfo undo = state.makeMove(move);

            int evalScore = alphabeta(state, depth - 1, alpha, beta, true, aiColor, ply + 1);

            state.unmakeMove(move, undo);

            if(evalScore < minEval) {
                minEval = evalScore;
                bestMove = move;
            }
            beta = std::min(beta, evalScore);

            if(beta <= alpha) {
                flag = TTEntry::LOWER_BOUND;
                break;
            }
        }
        if(beta < 200000 && flag != TTEntry::LOWER_BOUND) {
            flag = TTEntry::EXACT;
        }
        
        // Store in transposition table
        if(transpositionTable.size() < TT_SIZE) {
            transpositionTable[currentHash] = {currentHash, depth, minEval, flag, bestMove};
        }
        
        return minEval;
    }
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

    // Clear transposition table periodically to avoid stale entries
    if(transpositionTable.size() > TT_SIZE) {
        transpositionTable.clear();
    }

    Game rootGame = game; 
    std::vector<Move> rootMoves = rootGame.generateMoves(aiColor);
    
    if(rootMoves.empty()) return Move();
    
    // Initial sorting (MVV-LVA)
    std::sort(rootMoves.begin(), rootMoves.end(), [&](const Move& a, const Move& b) {
        return scoreMove(rootGame, a, Move()) > scoreMove(rootGame, b, Move());
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

            int score = alphabeta(rootGame, currentDepth - 1, alpha, beta, false, aiColor, 1);
            
            rootGame.unmakeMove(move, undo);

            alpha = std::max(alpha, score);
            scoredMoves[i].score = score;
        }

        // Sort by score for next iteration
        std::sort(scoredMoves.begin(), scoredMoves.end(), [](const MoveScore& a, const MoveScore& b) {
            return a.score > b.score;
        });

        bestMoveSoFar = scoredMoves[0].move;

        qDebug() << "Depth" << currentDepth << "complete: Best Score =" << scoredMoves[0].score;
    }

    qDebug() << "========================================";
    qDebug() << "AI Iterative Depth:" << searchDepth; 
    qDebug() << "Time Elapsed:" << timer.elapsed() << "ms"; 
    qDebug() << "Nodes Visited:" << nodeCount;
    qDebug() << "Best Move Score:" << scoredMoves[0].score;
    qDebug() << "TT Entries:" << transpositionTable.size();
    qDebug() << "========================================";

    return bestMoveSoFar;
}
