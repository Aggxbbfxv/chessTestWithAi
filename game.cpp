#include "game.h"

#include <algorithm>

Game::Game() : p_currentTurn(Piece::WHITE), m_wKingX(4), m_wKingY(7), m_bKingX(4), m_bKingY(0)
{
    for (auto& column : m_board)
    {
        for (auto& cell : column)
        {
            cell = nullptr;
        }
    }
    resetBoard();
}

Game::Game(const Game& other) : p_currentTurn(other.p_currentTurn),
    m_wKingX(other.m_wKingX), m_wKingY(other.m_wKingY),
    m_bKingX(other.m_bKingX), m_bKingY(other.m_bKingY)
{
    for (auto& column : m_board)
    {
        for (auto& cell : column)
        {
            cell = nullptr;
        }
    }

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            if (other.m_board[x][y])
            {
                m_board[x][y] = other.m_board[x][y]->clone();
            }
        }
    }
}

Game::~Game()
{
    for (auto& column : m_board)
    {
        for (auto& cell : column)
        {
            delete cell;
            cell = nullptr;
        }
    }
}

void Game::resetBoard()
{
    for (auto& column : m_board)
    {
        for (auto& cell : column)
        {
            delete cell;
            cell = nullptr;
        }
    }

    // Black pieces
    m_board[0][0] = new Rook(Piece::BLACK);
    m_board[1][0] = new Knight(Piece::BLACK);
    m_board[2][0] = new Bishop(Piece::BLACK);
    m_board[3][0] = new Queen(Piece::BLACK);
    m_board[4][0] = new King(Piece::BLACK);
    m_board[5][0] = new Bishop(Piece::BLACK);
    m_board[6][0] = new Knight(Piece::BLACK);
    m_board[7][0] = new Rook(Piece::BLACK);
    for (int i = 0; i < 8; ++i)
    {
        m_board[i][1] = new Pawn(Piece::BLACK);
    }

    // White pieces
    m_board[0][7] = new Rook(Piece::WHITE);
    m_board[1][7] = new Knight(Piece::WHITE);
    m_board[2][7] = new Bishop(Piece::WHITE);
    m_board[3][7] = new Queen(Piece::WHITE);
    m_board[4][7] = new King(Piece::WHITE);
    m_board[5][7] = new Bishop(Piece::WHITE);
    m_board[6][7] = new Knight(Piece::WHITE);
    m_board[7][7] = new Rook(Piece::WHITE);
    for (int i = 0; i < 8; ++i)
    {
        m_board[i][6] = new Pawn(Piece::WHITE);
    }

    m_wKingX = 4; m_wKingY = 7;
    m_bKingX = 4; m_bKingY = 0;
    p_currentTurn = Piece::WHITE;
}

Piece* Game::getPiece(int x, int y) const
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return nullptr;
    return m_board[x][y];
}

void Game::setPiece(int x, int y, Piece* piece)
{
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return;
    m_board[x][y] = piece;

    if (piece && piece->getType() == Piece::KING)
    {
        if (piece->getColor() == Piece::WHITE)
        {
            m_wKingX = x; m_wKingY = y;
        }
        else
        {
            m_bKingX = x; m_bKingY = y;
        }
    }
}

Piece* Game::makeMove(const Move& move)
{
    Piece* moving = getPiece(move.fromX, move.fromY);
    if (!moving) return nullptr;

    Piece* captured = m_board[move.toX][move.toY];
    m_board[move.toX][move.toY] = moving;
    m_board[move.fromX][move.fromY] = nullptr;

    if (moving->getType() == Piece::KING)
    {
        if (moving->getColor() == Piece::WHITE)
        {
            m_wKingX = move.toX; m_wKingY = move.toY;
        }
        else
        {
            m_bKingX = move.toX; m_bKingY = move.toY;
        }
    }

    switchTurn();
    return captured;
}

void Game::unmakeMove(const Move& move, Piece* capturedPiece)
{
    Piece* moving = m_board[move.toX][move.toY];
    m_board[move.fromX][move.fromY] = moving;
    m_board[move.toX][move.toY] = capturedPiece;

    if (moving && moving->getType() == Piece::KING)
    {
        if (moving->getColor() == Piece::WHITE)
        {
            m_wKingX = move.fromX; m_wKingY = move.fromY;
        }
        else
        {
            m_bKingX = move.fromX; m_bKingY = move.fromY;
        }
    }

    switchTurn();
}

Game Game::makeMoveCopy(const Move& move) const
{
    Game copy(*this);
    Piece* captured = copy.makeMove(move);
    if (captured)
    {
        delete captured;
    }
    return copy;
}

bool Game::isGameOver()
{
    return generateMoves(p_currentTurn).empty();
}

std::vector<Move> Game::generateMoves(Piece::PieceColor color)
{
    std::vector<Move> pseudoMoves;
    std::vector<Move> legalMoves;

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            Piece* p = m_board[x][y];
            if (!p || p->getColor() != color) continue;
            p->addPossibleMoves(*this, x, y, pseudoMoves);
        }
    }

    for (const auto& move : pseudoMoves)
    {
        Piece* captured = makeMove(move);
        bool inCheck = isCheck(color);
        unmakeMove(move, captured);

        if (!inCheck)
        {
            legalMoves.push_back(move);
        }
    }
    return legalMoves;
}

void Game::switchTurn()
{
    p_currentTurn = (p_currentTurn == Piece::WHITE) ? Piece::BLACK : Piece::WHITE;
}

bool Game::isCheck(Piece::PieceColor kingColor) const
{
    int kingX = (kingColor == Piece::WHITE) ? m_wKingX : m_bKingX;
    int kingY = (kingColor == Piece::WHITE) ? m_wKingY : m_bKingY;

    Piece::PieceColor opponent = (kingColor == Piece::WHITE) ? Piece::BLACK : Piece::WHITE;

    for (int x = 0; x < 8; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            Piece* p = m_board[x][y];
            if (!p || p->getColor() != opponent) continue;

            std::vector<Move> moves;
            p->addPossibleMoves(*this, x, y, moves);
            for (const auto& mv : moves)
            {
                if (mv.toX == kingX && mv.toY == kingY)
                {
                    return true;
                }
            }
        }
    }

    return false;
}
