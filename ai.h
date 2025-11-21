#ifndef AI_H
#define AI_H

#include <cstdint>
#include <unordered_map>
#include <vector>
#include "game.h"
#include "move.h"

class AI
{
public:
    static const int searchDepth = 7;
    static Move findBestMove(const Game& game);

    static int nodeCount;

private:
    enum class EntryType { EXACT, LOWER, UPPER };

    struct TTEntry
    {
        int depth;
        int value;
        EntryType type;
        Move bestMove;
    };

    static int eval(const Game& board);
    static int alphabeta(Game& state, int depth, int alpha, int beta, bool maximizingPlayer);
    static int quiescence(Game& state, int alpha, int beta);
    static int scoreMove(const Game& state, const Move& move, Piece::PieceColor moverColor);

    static uint64_t computeHash(const Game& state);
    static void initZobrist();
    static void storeKiller(int depth, const Move& move);

    static const Piece::PieceColor aiColor;
    static std::unordered_map<uint64_t, TTEntry> transpositionTable;
    static bool zobristInitialized;
    static uint64_t zobristPiece[2][6][8][8];
    static uint64_t zobristTurn;

    static std::vector<std::vector<Move>> killerMoves; // two killer moves per depth
    static int historyHeuristic[8][8][8][8];
};

#endif // AI_H