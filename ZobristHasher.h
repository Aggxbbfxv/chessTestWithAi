#ifndef ZOBRISTHASHER_H
#define ZOBRISTHASHER_H

#include <cstdint>
#include "types.h"

class Game;

class ZobristHasher {
public:
    static ZobristHasher& getInstance();

    uint64_t calculateHash(const Game& game) const;
    void initialize();

    // 증분 업데이트를 위한 키 접근
    uint64_t getPieceKey(Piece::PieceType type, Piece::PieceColor color, int x, int y) const;
    uint64_t getBlackTurnKey() const;
    uint64_t getCastleKey(Piece::PieceColor color, bool isKingSide) const;
    uint64_t getEnPassantKey(int x) const;


private:
    ZobristHasher();
    ZobristHasher(const ZobristHasher&) = delete;
    ZobristHasher& operator=(const ZobristHasher&) = delete;

    uint64_t pieceKeys[13][8][8]; // [PieceType * 2 + Color][x][y]
    uint64_t blackTurnKey;
    uint64_t castlingKeys[4]; // w_ks, w_qs, b_ks, b_qs
    uint64_t enPassantKeys[8]; // file x
};

#endif // ZOBRISTHASHER_H
