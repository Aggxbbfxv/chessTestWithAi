#ifndef GAME_H
#define GAME_H

#include <vector>
#include "piece.h"
#include "move.h"

class Game
{
    // [최적화] AI 클래스가 Game의 private 멤버(m_board)에 직접 접근할 수 있게 허용
    // 이를 통해 eval 함수에서 getPiece() 함수 호출 비용을 제거하여 속도를 높임
    friend class AI;

public:
    Game();
    Game(const Game& other);
    ~Game();

    void resetBoard();
    
    // [최적화] 자주 호출되므로 인라인 처리가 유리할 수 있으나, 
    // AI는 friend로 직접 접근하므로 외부 UI용으로 유지
    Piece* getPiece(int x, int y) const;
    
    Piece* makeMove(const Move& move);
    void unmakeMove(const Move& move, Piece* capturedPiece);
    Game makeMoveCopy(const Move& move) const;

    Piece::PieceColor getCurrentTurn() const { return p_currentTurn; }
    bool isGameOver();

    std::vector<Move> generateMoves(Piece::PieceColor color);

    void setPiece(int x, int y, Piece* piece);

private:
    void switchTurn();
    bool isCheck(Piece::PieceColor kingColor) const;

    Piece* m_board[8][8];
    Piece::PieceColor p_currentTurn;

    // 킹 위치 캐시 (검색 성능 O(1) 보장용)
    int m_wKingX, m_wKingY;
    int m_bKingX, m_bKingY;
};

#endif // GAME_H