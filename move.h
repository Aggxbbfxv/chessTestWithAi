#ifndef MOVE_H
#define MOVE_H

struct Move
{
    int fromX, fromY, toX, toY;

    Move() : fromX(0), fromY(0), toX(0), toY(0) {}
    Move(int fx, int fy, int tx, int ty) : fromX(fx), fromY(fy), toX(tx), toY(ty) {}

    bool isNull() const {return fromX == 0 && fromY ==0 && toX == 0 && toY == 0; }
};

#endif // MOVE_H
