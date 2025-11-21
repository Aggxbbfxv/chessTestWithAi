#include "piece.h"
#include "game.h"
#include <cmath>
#include <vector>

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
            break;
        }

        newX += dx;
        newY += dy;
    }
}

Piece* Pawn::clone() const { return new Pawn(*this); }

void Pawn::addPossibleMoves(const Game& game, int x, int y, vector<Move>& moves) const
{
    int direction = (p_color == Piece::WHITE) ? -1 : 1;
    int promotionRank = (p_color == Piece::WHITE) ? 0 : 7;

    // 한 칸 전진
    int oneStepY = y + direction;
    if(oneStepY >= 0 && oneStepY < 8 && game.getPiece(x, oneStepY) == nullptr)
    {
        if (oneStepY == promotionRank) { // 프로모션
            moves.emplace_back(x, y, x, oneStepY, (int)Piece::QUEEN);
            moves.emplace_back(x, y, x, oneStepY, (int)Piece::ROOK);
            moves.emplace_back(x, y, x, oneStepY, (int)Piece::BISHOP);
            moves.emplace_back(x, y, x, oneStepY, (int)Piece::KNIGHT);
        } else {
            moves.emplace_back(x, y, x, oneStepY);
        }

        // 두 칸 전진 (첫 수일 때)
        int startRow = (p_color == Piece::WHITE) ? 6 : 1;
        if(y == startRow)
        {
            int twoStepY = y + 2 * direction;
            if (game.getPiece(x, twoStepY) == nullptr)
            {
                moves.emplace_back(x, y, x, twoStepY);
            }
        }
    }

    // 대각선 캡처
    int captureY = y + direction;
    if(captureY >= 0 && captureY < 8)
    {
        // 왼쪽 캡처
        if(x > 0)
        {
            Piece* leftTarget = game.getPiece(x - 1, captureY);
            if(leftTarget && leftTarget->getColor() != p_color)
            {
                if (captureY == promotionRank) { // 프로모션
                    moves.emplace_back(x, y, x - 1, captureY, (int)Piece::QUEEN);
                    moves.emplace_back(x, y, x - 1, captureY, (int)Piece::ROOK);
                    moves.emplace_back(x, y, x - 1, captureY, (int)Piece::BISHOP);
                    moves.emplace_back(x, y, x - 1, captureY, (int)Piece::KNIGHT);
                } else {
                    moves.emplace_back(x, y, x - 1, captureY);
                }
            }
        }
        // 오른쪽 캡처
        if(x < 7)
        {
            Piece* rightTarget = game.getPiece(x + 1, captureY);
            if(rightTarget && rightTarget->getColor() != p_color)
            {
                if (captureY == promotionRank) { // 프로모션
                    moves.emplace_back(x, y, x + 1, captureY, (int)Piece::QUEEN);
                    moves.emplace_back(x, y, x + 1, captureY, (int)Piece::ROOK);
                    moves.emplace_back(x, y, x + 1, captureY, (int)Piece::BISHOP);
                    moves.emplace_back(x, y, x + 1, captureY, (int)Piece::KNIGHT);
                } else {
                    moves.emplace_back(x, y, x + 1, captureY);
                }
            }
        }
    }

    // 앙파상 로직
    int epTarget = game.getEnPassantTargetSquare();
    if (epTarget != -1) {
        int epX = epTarget % 8;
        int epY = epTarget / 8;

        if (y == ((p_color == Piece::WHITE) ? 3 : 4)) {
             if (epX == x + 1 && epY == y + direction) {
                moves.emplace_back(x, y, x + 1, y + direction, Move::EN_PASSANT);
            } else if (epX == x - 1 && epY == y + direction) {
                moves.emplace_back(x, y, x - 1, y + direction, Move::EN_PASSANT);
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

    // 캐슬링 로직 추가
    Piece::PieceColor enemyColor = (p_color == Piece::WHITE) ? Piece::BLACK : Piece::WHITE;
    if (game.isCheck(p_color)) return;

    // 킹사이드 캐슬링
    if (game.canCastle(p_color, true))
    {
        if (game.getPiece(x + 1, y) == nullptr && game.getPiece(x + 2, y) == nullptr)
        {
            if (!game.isSquareAttacked(x, y, enemyColor) &&
                !game.isSquareAttacked(x + 1, y, enemyColor) &&
                !game.isSquareAttacked(x + 2, y, enemyColor))
            {
                moves.emplace_back(x, y, x + 2, y, Move::CASTLE_KS);
            }
        }
    }

    // 퀸사이드 캐슬링
    if (game.canCastle(p_color, false))
    {
        if (game.getPiece(x - 1, y) == nullptr && game.getPiece(x - 2, y) == nullptr && game.getPiece(x - 3, y) == nullptr)
        {
            if (!game.isSquareAttacked(x, y, enemyColor) &&
                !game.isSquareAttacked(x - 1, y, enemyColor) &&
                !game.isSquareAttacked(x - 2, y, enemyColor))
            {
                moves.emplace_back(x, y, x - 2, y, Move::CASTLE_QS);
            }
        }
    }
}