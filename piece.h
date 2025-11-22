#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include "move.h"



#include "types.h"

class Game;

class Piece
{
public:

    Piece(Piece::PieceColor color, Piece::PieceType type) : p_color(color), p_type(type) {}
    virtual ~Piece() {}

    // [����] ���͸� ��ȯ���� �ʰ� ������ �޾� �߰��� (�޸� �Ҵ� ����)
    virtual void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const = 0;

    virtual Piece* clone() const = 0;

    Piece::PieceColor getColor() const { return p_color; }
    Piece::PieceType getType() const { return p_type; }
    int getValue() const;

protected:
    Piece::PieceColor p_color;
    Piece::PieceType p_type;
};

class Pawn : public Piece
{
public:
    Pawn(Piece::PieceColor color) : Piece(color, Piece::PAWN) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Knight : public Piece
{
public:
    Knight(Piece::PieceColor color) : Piece(color, Piece::KNIGHT) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Bishop : public Piece
{
public:
    Bishop(Piece::PieceColor color) : Piece(color, Piece::BISHOP) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Rook : public Piece
{
public:
    Rook(Piece::PieceColor color) : Piece(color, Piece::ROOK) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Queen : public Piece
{
public:
    Queen(Piece::PieceColor color) : Piece(color, Piece::QUEEN) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class King : public Piece
{
public:
    King(Piece::PieceColor color) : Piece(color, Piece::KING) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

#endif // PIECE_H
