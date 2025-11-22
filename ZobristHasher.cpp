#include "ZobristHasher.h"
#include "game.h"
#include "piece.h"
#include <random>
#include <chrono>

ZobristHasher& ZobristHasher::getInstance() {
    static ZobristHasher instance;
    return instance;
}

ZobristHasher::ZobristHasher() {
    initialize();
}

void ZobristHasher::initialize() {
    std::mt19937_64 engine(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<uint64_t> dist;

    for (int i = 0; i < 13; ++i) {
        for (int j = 0; j < 8; ++j) {
            for (int k = 0; k < 8; ++k) {
                pieceKeys[i][j][k] = dist(engine);
            }
        }
    }

    blackTurnKey = dist(engine);

    for (int i = 0; i < 4; ++i) {
        castlingKeys[i] = dist(engine);
    }

    for (int i = 0; i < 8; ++i) {
        enPassantKeys[i] = dist(engine);
    }
}

uint64_t ZobristHasher::getPieceKey(Chess::PieceType type, Chess::PieceColor color, int x, int y) const {
    // PieceType enum starts from EMPTY=0, PAWN=1, etc.
    // We can map this to an index from 0 to 11 for the 12 piece types (6 types * 2 colors)
    // Index = (type - 1) * 2 + (color - 1)
    if (type == Chess::EMPTY || color == Chess::NONE) return 0;
    return pieceKeys[(type - 1) * 2 + (color - 1)][y][x];
}

uint64_t ZobristHasher::getBlackTurnKey() const {
    return blackTurnKey;
}

uint64_t ZobristHasher::getCastleKey(Chess::PieceColor color, bool isKingSide) const {
    if (color == Chess::WHITE) {
        return isKingSide ? castlingKeys[0] : castlingKeys[1];
    } else {
        return isKingSide ? castlingKeys[2] : castlingKeys[3];
    }
}

uint64_t ZobristHasher::getEnPassantKey(int x) const {
    if (x < 0 || x >= 8) return 0;
    return enPassantKeys[x];
}


uint64_t ZobristHasher::calculateHash(const Game& game) const {
    uint64_t hash = 0;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Piece* p = game.getPiece(x, y);
            if (p != nullptr) {
                hash ^= getPieceKey(p->getType(), p->getColor(), x, y);
            }
        }
    }

    if (game.getCurrentTurn() == Chess::BLACK) {
        hash ^= getBlackTurnKey();
    }

    if (game.canCastle(Chess::WHITE, true)) hash ^= getCastleKey(Chess::WHITE, true);
    if (game.canCastle(Chess::WHITE, false)) hash ^= getCastleKey(Chess::WHITE, false);
    if (game.canCastle(Chess::BLACK, true)) hash ^= getCastleKey(Chess::BLACK, true);
    if (game.canCastle(Chess::BLACK, false)) hash ^= getCastleKey(Chess::BLACK, false);

    int epSquare = game.getEnPassantTargetSquare();
    if (epSquare != -1) {
        hash ^= getEnPassantKey(epSquare % 8);
    }

    return hash;
}
