#ifndef GAME_H
#define GAME_H

#include <vector>
#include "piece.h"
#include "move.h"

class Game
{
public:
    Game();
    Game(const Game& other);
    ~Game();

    void resetBoard();
    Piece* getPiece(int x, int y) const;
    Piece* makeMove(const Move& move);
    void unmakeMove(const Move& move, Piece* capturedPiece);
    Game makeMoveCopy(const Move& move) const;

    Piece::PieceColor getCurrentTurn() const { return p_currentTurn; }
    bool isGameOver();

    // [최적화] 반환형은 유지하되 내부 구현 최적화
    std::vector<Move> generateMoves(Piece::PieceColor color);

    void setPiece(int x, int y, Piece* piece);

private:
    void switchTurn();
    bool isCheck(Piece::PieceColor kingColor) const; // 로직 완전히 변경됨

    Piece* m_board[8][8];
    Piece::PieceColor p_currentTurn;

    // [추가] 킹의 위치를 O(1)로 찾기 위한 캐시 변수
    int m_wKingX, m_wKingY;
    int m_bKingX, m_bKingY;
};

#endif // GAME_H
