#ifndef AI_H
#define AI_H

#include "game.h"
#include "move.h"
#include <vector>

// Transposition Table Entry
struct TTEntry {
    uint64_t key;
    int score;
    int depth;
    int flag; // 0: Exact, 1: Lowerbound (Alpha), 2: Upperbound (Beta)
    Move bestMove;

    TTEntry() : key(0), score(0), depth(-1), flag(0) {}
};

class TranspositionTable {
public:
    TranspositionTable(int sizeMB);
    ~TranspositionTable();

    void clear();
    bool probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove);
    void store(uint64_t key, int depth, int score, int flag, Move bestMove);

private:
    std::vector<TTEntry> table;
    size_t size;
};

class AI
{
public:
    static const int searchDepth = 6; // Target depth
    static Move findBestMove(const Game& game, Chess::PieceColor aiColor);

    static int nodeCount;
    static int qNodeCount; // Quiescence node count

private:
    // Search
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maxing, Chess::PieceColor aiColor);
    static int quiescence(Game& state, int alpha, int beta, Chess::PieceColor aiColor);

    // Evaluation
    static int eval(const Game& board, Chess::PieceColor colorToMax);
    static int scoreMove(const Game& game, const Move& move, const Move& ttMove);

    // Tables
    static TranspositionTable tt;
    
    // PeSTO Piece-Square Tables (Midgame & Endgame)
    static const int mg_pawn_table[64];
    static const int eg_pawn_table[64];
    static const int mg_knight_table[64];
    static const int eg_knight_table[64];
    static const int mg_bishop_table[64];
    static const int eg_bishop_table[64];
    static const int mg_rook_table[64];
    static const int eg_rook_table[64];
    static const int mg_queen_table[64];
    static const int eg_queen_table[64];
    static const int mg_king_table[64];
    static const int eg_king_table[64];

    // Piece Values
    static const int mg_value[6];
    static const int eg_value[6];
};

#endif // AI_H
