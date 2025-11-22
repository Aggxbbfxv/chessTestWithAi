#include "piece.h"
#include "game.h"
#include <cmath>
#include <vector>

int Piece::getValue() const
{
    switch(p_type)
    {
    case Chess::PAWN: return 100;
    case Chess::KNIGHT: return 300;
    case Chess::BISHOP: return 310;
    case Chess::ROOK: return 500;
    case Chess::QUEEN: return 900;
    case Chess::KING: return 20000;
    default: return 0;
    }
}

static void addStraightMove(std::vector<Move>& moves, const Game& game, Chess::PieceColor color, int x, int y, int dx, int dy)
{
    int newX = x + dx;
    int newY = y + dy;

    while (newX >= 0 && newX < 8 && newY >= 0 && newY < 8)
    {
        Piece* target = game.getPiece(newX, newY);

        if (target == nullptr)
        {
            moves.emplace_back(x, y, newX, newY);
        }
        else
        {
            if (target->getColor() != color)
            {
                moves.emplace_back(x, y, newX, newY);
            }
            break;
        }

        newX += dx;
        newY += dy;
    }
}

Piece* Pawn::clone() const { return new Pawn(*this); }

void Pawn::addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const
{
    int direction = (p_color == Chess::WHITE) ? -1 : 1;
    int promotionRank = (p_color == Chess::WHITE) ? 0 : 7;

    // Forward pushes
    int oneStepY = y + direction;
    if(oneStepY >= 0 && oneStepY < 8 && game.getPiece(x, oneStepY) == nullptr)
    {
        if (oneStepY == promotionRank) {
            moves.emplace_back(x, y, x, oneStepY, Chess::QUEEN);
            moves.emplace_back(x, y, x, oneStepY, Chess::ROOK);
            moves.emplace_back(x, y, x, oneStepY, Chess::BISHOP);
            moves.emplace_back(x, y, x, oneStepY, Chess::KNIGHT);
        } else {
            moves.emplace_back(x, y, x, oneStepY);
        }

        int startRow = (p_color == Chess::WHITE) ? 6 : 1;
        if(y == startRow)
        {
            int twoStepY = y + 2 * direction;
            if (game.getPiece(x, twoStepY) == nullptr)
            {
                moves.emplace_back(x, y, x, twoStepY);
            }
        }
    }

    // Diagonal captures
    int captureY = y + direction;
    if(captureY >= 0 && captureY < 8)
    {
        if(x > 0)
        {
            Piece* leftTarget = game.getPiece(x - 1, captureY);
            if(leftTarget && leftTarget->getColor() != p_color)
            {
                if (captureY == promotionRank) {
                    moves.emplace_back(x, y, x - 1, captureY, Chess::QUEEN);
                    moves.emplace_back(x, y, x - 1, captureY, Chess::ROOK);
                    moves.emplace_back(x, y, x - 1, captureY, Chess::BISHOP);
                    moves.emplace_back(x, y, x - 1, captureY, Chess::KNIGHT);
                } else {
                    moves.emplace_back(x, y, x - 1, captureY);
                }
            }
        }
        if(x < 7)
        {
            Piece* rightTarget = game.getPiece(x + 1, captureY);
            if(rightTarget && rightTarget->getColor() != p_color)
            {
                if (captureY == promotionRank) {
                    moves.emplace_back(x, y, x + 1, captureY, Chess::QUEEN);
                    moves.emplace_back(x, y, x + 1, captureY, Chess::ROOK);
                    moves.emplace_back(x, y, x + 1, captureY, Chess::BISHOP);
                    moves.emplace_back(x, y, x + 1, captureY, Chess::KNIGHT);
                } else {
                    moves.emplace_back(x, y, x + 1, captureY);
                }
            }
        }
    }

    // En-passant
    int epTarget = game.getEnPassantTargetSquare();
    if (epTarget != -1) {
        int epX = epTarget % 8;
        int epY = epTarget / 8;

        if (y == ((p_color == Chess::WHITE) ? 3 : 4)) {
             if (epX == x + 1 && epY == y + direction) {
                moves.emplace_back(x, y, x + 1, y + direction, Move::EN_PASSANT);
            } else if (epX == x - 1 && epY == y + direction) {
                moves.emplace_back(x, y, x - 1, y + direction, Move::EN_PASSANT);
            }
        }
    }
}

Piece* Knight::clone() const { return new Knight(*this); }

void Knight::addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const
{
    static const int dx[] = {1,1,2,2,-1,-1,-2,-2};
    static const int dy[] = {2,-2,1,-1,2,-2,1,-1};

    for(int i = 0; i < 8; ++i)
    {
        int newX = x + dx[i];
        int newY = y + dy[i];

        if(newX >= 0 && newX < 8 && newY >= 0 && newY < 8)
        {
            Piece* target = game.getPiece(newX, newY);
            if(target == nullptr || target->getColor() != p_color)
            {
                moves.emplace_back(x, y, newX, newY);
            }
        }
    }
}

Piece* Bishop::clone() const { return new Bishop(*this); }

void Bishop::addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const
{
    addStraightMove(moves, game, p_color, x, y, 1, 1);
    addStraightMove(moves, game, p_color, x, y, 1, -1);
    addStraightMove(moves, game, p_color, x, y, -1, 1);
    addStraightMove(moves, game, p_color, x, y, -1, -1);
}

Piece* Rook::clone() const { return new Rook(*this); }

void Rook::addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const
{
    addStraightMove(moves, game, p_color, x, y, 0, 1);
    addStraightMove(moves, game, p_color, x, y, 0, -1);
    addStraightMove(moves, game, p_color, x, y, 1, 0);
    addStraightMove(moves, game, p_color, x, y, -1, 0);
}

Piece* Queen::clone() const { return new Queen(*this); }

void Queen::addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const
{
    addStraightMove(moves, game, p_color, x, y, 1, 1);
    addStraightMove(moves, game, p_color, x, y, 1, -1);
    addStraightMove(moves, game, p_color, x, y, -1, 1);
    addStraightMove(moves, game, p_color, x, y, -1, -1);
    addStraightMove(moves, game, p_color, x, y, 0, 1);
    addStraightMove(moves, game, p_color, x, y, 0, -1);
    addStraightMove(moves, game, p_color, x, y, 1, 0);
    addStraightMove(moves, game, p_color, x, y, -1, 0);
}

Piece* King::clone() const { return new King(*this); }

void King::addPossibleMoves(const Game& game, int x, int y, std::vector<Move>& moves) const
{
    for(int dx = -1; dx <= 1; ++dx)
    {
        for(int dy = -1; dy <= 1; ++dy)
        {
            if(dx == 0 && dy == 0) continue;
            int newX = x + dx;
            int newY = y + dy;

            if(newX >= 0 && newX < 8 && newY >= 0 && newY < 8)
            {
                Piece* target = game.getPiece(newX, newY);
                if(target == nullptr || target->getColor() != p_color)
                {
                    moves.emplace_back(x, y, newX, newY);
                }
            }
        }
    }

    // Castling
    Chess::PieceColor enemyColor = (p_color == Chess::WHITE) ? Chess::BLACK : Chess::WHITE;
    if (game.isCheck(p_color)) return;

    // King-side castling
    if (game.canCastle(p_color, true)) {
        Piece* rook = game.getPiece(7, y);
        if (rook && rook->getType() == Chess::ROOK && rook->getColor() == p_color) {
            if (game.getPiece(x + 1, y) == nullptr && game.getPiece(x + 2, y) == nullptr) {
                if (!game.isSquareAttacked(x, y, enemyColor) &&
                    !game.isSquareAttacked(x + 1, y, enemyColor) &&
                    !game.isSquareAttacked(x + 2, y, enemyColor)) {
                    moves.emplace_back(x, y, x + 2, y, Move::CASTLE_KS);
                }
            }
        }
    }

    // Queen-side castling
    if (game.canCastle(p_color, false)) {
        Piece* rook = game.getPiece(0, y);
        if (rook && rook->getType() == Chess::ROOK && rook->getColor() == p_color) {
            if (game.getPiece(x - 1, y) == nullptr && game.getPiece(x - 2, y) == nullptr && game.getPiece(x - 3, y) == nullptr) {
                if (!game.isSquareAttacked(x, y, enemyColor) &&
                    !game.isSquareAttacked(x - 1, y, enemyColor) &&
                    !game.isSquareAttacked(x - 2, y, enemyColor)) {
                    moves.emplace_back(x, y, x - 2, y, Move::CASTLE_QS);
                }
            }
        }
    }
}
