#include "piece.h"
#include "game.h"

using namespace std;

int Piece::getValue() const
{
    switch(p_type)
    {
    case PAWN: return 100;
    case KNIGHT: return 300;
    case BISHOP: return 310;
    case ROOK: return 500;
    case QUEEN: return 900;
    case KING: return 20000;
    default: return 0;
    }
}

// [최적화] 헬퍼 함수도 벡터 참조를 받음
static void addStraightMove(vector<Move>& moves, const Game& game, Piece::PieceColor color, int x, int y, int dx, int dy)
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
            break; // 막히면 중단
        }

        newX += dx;
        newY += dy;
    }
}

Piece* Pawn::clone() const { return new Pawn(*this); }

void Pawn::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
{
    int direction = (p_color == Piece::WHITE) ? -1 : 1;
    int startRow = (p_color == Piece::WHITE) ? 6 : 1;

    int oneStepY = y + direction;
    if(oneStepY >= 0 && oneStepY < 8 && game.getPiece(x, oneStepY) == nullptr)
    {
        moves.emplace_back(x, y, x, oneStepY);

        if(y == startRow)
        {
            int twoStepY = y + 2 * direction;
            if (game.getPiece(x, twoStepY) == nullptr)
            {
                moves.emplace_back(x, y, x, twoStepY);
            }
        }
    }

    int CaptureY = y + direction;
    if(CaptureY >= 0 && CaptureY < 8)
    {
        if(x > 0)
        {
            Piece* leftTarget = game.getPiece(x - 1, CaptureY);
            if(leftTarget && leftTarget->getColor() != p_color)
            {
                moves.emplace_back(x, y, x - 1, CaptureY);
            }
        }
        if(x < 7)
        {
            Piece* rightTarget = game.getPiece(x + 1, CaptureY);
            if(rightTarget && rightTarget->getColor() != p_color)
            {
                moves.emplace_back(x, y, x + 1, CaptureY);
            }
        }
    }
}

Piece* Knight::clone() const { return new Knight(*this); }

void Knight::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
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

void Bishop::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
{
    addStraightMove(moves, game, p_color, x, y, 1, 1);
    addStraightMove(moves, game, p_color, x, y, 1, -1);
    addStraightMove(moves, game, p_color, x, y, -1, 1);
    addStraightMove(moves, game, p_color, x, y, -1, -1);
}

Piece* Rook::clone() const { return new Rook(*this); }

void Rook::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
{
    addStraightMove(moves, game, p_color, x, y, 0, 1);
    addStraightMove(moves, game, p_color, x, y, 0, -1);
    addStraightMove(moves, game, p_color, x, y, 1, 0);
    addStraightMove(moves, game, p_color, x, y, -1, 0);
}

Piece* Queen::clone() const { return new Queen(*this); }

void Queen::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
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

void King::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
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
}
