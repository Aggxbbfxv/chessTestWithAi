#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include "move.h"

using namespace std;

class Game;

class Piece
{
public:
    enum PieceColor {NONE, WHITE, BLACK};
    enum PieceType {EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

    Piece(PieceColor color, PieceType type) : p_color(color), p_type(type) {}
    virtual ~Piece() {}

    // [변경] 벡터를 반환하지 않고 참조로 받아 추가함 (메모리 할당 방지)
    virtual void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const = 0;

    virtual Piece* clone() const = 0;

    PieceColor getColor() const { return p_color; }
    PieceType getType() const { return p_type; }
    int getValue() const;

protected:
    PieceColor p_color;
    PieceType p_type;
};

class Pawn : public Piece
{
public:
    Pawn(PieceColor color) : Piece(color, PAWN) {}
    void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Knight : public Piece
{
public:
    Knight(PieceColor color) : Piece(color, KNIGHT) {}
    void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Bishop : public Piece
{
public:
    Bishop(PieceColor color) : Piece(color, BISHOP) {}
    void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Rook : public Piece
{
public:
    Rook(PieceColor color) : Piece(color, ROOK) {}
    void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Queen : public Piece
{
public:
    Queen(PieceColor color) : Piece(color, QUEEN) {}
    void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class King : public Piece
{
public:
    King(PieceColor color) : Piece(color, KING) {}
    void addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

#endif // PIECE_H
