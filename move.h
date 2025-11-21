#ifndef MOVE_H
#define MOVE_H

struct Move
{
    enum MoveType { NORMAL, CASTLE_KS, CASTLE_QS, EN_PASSANT };

    int fromX, fromY, toX, toY;
    int promotionType; // 폰 프로모션 타입 (Piece::PieceType 대신 int 사용)
    MoveType moveType;              // 캐슬링 등 특수 이동 타입

    Move() : fromX(0), fromY(0), toX(0), toY(0), promotionType(0), moveType(NORMAL) {}
    Move(int fx, int fy, int tx, int ty) : fromX(fx), fromY(fy), toX(tx), toY(ty), promotionType(0), moveType(NORMAL) {}
    // 프로모션용 생성자
    Move(int fx, int fy, int tx, int ty, int pType) : fromX(fx), fromY(fy), toX(tx), toY(ty), promotionType(pType), moveType(NORMAL) {}
    // 특수 이동용 생성자
    Move(int fx, int fy, int tx, int ty, MoveType mType) : fromX(fx), fromY(fy), toX(tx), toY(ty), promotionType(0), moveType(mType) {}


    bool isNull() const {return fromX == 0 && fromY ==0 && toX == 0 && toY == 0; }
};

#endif // MOVE_H
