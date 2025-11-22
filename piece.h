#ifndef PIECE_H
#define PIECE_H

#include <vector>
#include "move.h"
#include "types.h"

class Game;

class Piece
{
public:

    Piece(Chess::PieceColor color, Chess::PieceType type) : p_color(color), p_type(type) {}
    virtual ~Piece() {}

    // [����] ���͸� ��ȯ���� �ʰ� ������ �޾� �߰��� (�޸� �Ҵ� ����)
    virtual void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const = 0;

    virtual Piece* clone() const = 0;

    Chess::PieceColor getColor() const { return p_color; }
    Chess::PieceType getType() const { return p_type; }
    int getValue() const;

protected:
    Chess::PieceColor p_color;
    Chess::PieceType p_type;
};

class Pawn : public Piece
{
public:
    Pawn(Chess::PieceColor color) : Piece(color, Chess::PAWN) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Knight : public Piece
{
public:
    Knight(Chess::PieceColor color) : Piece(color, Chess::KNIGHT) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Bishop : public Piece
{
public:
    Bishop(Chess::PieceColor color) : Piece(color, Chess::BISHOP) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Rook : public Piece
{
public:
    Rook(Chess::PieceColor color) : Piece(color, Chess::ROOK) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class Queen : public Piece
{
public:
    Queen(Chess::PieceColor color) : Piece(color, Chess::QUEEN) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

class King : public Piece
{
public:
    King(Chess::PieceColor color) : Piece(color, Chess::KING) {}
    void addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const override;
    virtual Piece* clone() const override;
};

#endif // PIECE_H
