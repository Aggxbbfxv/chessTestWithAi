#ifndef MOVE_H
#define MOVE_H

#include "types.h" // For Chess::PieceType

struct Move
{
    enum MoveType { NORMAL, CASTLE_KS, CASTLE_QS, EN_PASSANT };

    int fromX, fromY, toX, toY;
    Chess::PieceType promotionType; // Use enum for type safety
    MoveType moveType;

    Move() : fromX(0), fromY(0), toX(0), toY(0), promotionType(Chess::EMPTY), moveType(NORMAL) {}
    Move(int fx, int fy, int tx, int ty) : fromX(fx), fromY(fy), toX(tx), toY(ty), promotionType(Chess::EMPTY), moveType(NORMAL) {}
    // Promotion constructor
    Move(int fx, int fy, int tx, int ty, Chess::PieceType pType) : fromX(fx), fromY(fy), toX(tx), toY(ty), promotionType(pType), moveType(NORMAL) {}
    // Special move constructor
    Move(int fx, int fy, int tx, int ty, MoveType mType) : fromX(fx), fromY(fy), toX(tx), toY(ty), promotionType(Chess::EMPTY), moveType(mType) {}


    bool isNull() const {return fromX == 0 && fromY ==0 && toX == 0 && toY == 0; }
};

#endif // MOVE_H
