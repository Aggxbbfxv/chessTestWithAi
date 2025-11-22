#ifndef GAME_H
#define GAME_H

#include <vector>
#include <memory>
#include "types.h"
#include "move.h"
#include "ZobristHasher.h"

class Piece; // Forward declaration

// 이동 실행 취소에 필요한 정보를 담는 구조체
struct UndoInfo
{
    Piece::PieceType capturedPieceType;
    Piece::PieceColor capturedPieceColor;
    bool w_castle_ks;
    bool w_castle_qs;
    bool b_castle_ks;
    bool b_castle_qs;
    int enPassantTargetSquare;
    int fiftyMoveCounter;
    uint64_t currentHash;
};

class Game
{
    friend class AI;
public:
    enum GameState { IN_PROGRESS, CHECKMATE, STALEMATE, DRAW };

    Game();
    Game(const Game& other);
    Game& operator=(const Game& other) = delete; // 복사 할당 연산자 비활성화
    ~Game();

    void resetBoard();
    
    Piece* getPiece(int x, int y) const;
    
    UndoInfo makeMove(const Move& move);
    void unmakeMove(const Move& move, const UndoInfo& undo);
    Game makeMoveCopy(const Move& move) const;

    Piece::PieceColor getCurrentTurn() const { return p_currentTurn; }
    bool isGameOver() const;
    GameState getGameState();

    // [최적화] 특정 위치의 기물에 대한 이동만 생성
    std::vector<Move> generateMoves(int x, int y);
    std::vector<Move> generateMoves(Piece::PieceColor color);

    void setPiece(int x, int y, std::unique_ptr<Piece> piece);

    // Public utility functions
    void switchTurn();
    bool isCheck(Piece::PieceColor kingColor) const;
    bool isSquareAttacked(int x, int y, Piece::PieceColor attackerColor) const;
    bool canCastle(Piece::PieceColor color, bool isKingSide) const;
    int getEnPassantTargetSquare() const { return enPassantTargetSquare; }

private:
    bool isInsufficientMaterial() const;

    std::unique_ptr<Piece> m_board[8][8];
    Piece::PieceColor p_currentTurn;

    // 킹 위치 캐시 (검색 성능 O(1) 보장용)
    int m_wKingX, m_wKingY;
    int m_bKingX, m_bKingY;

    // 캐슬링 권한
    bool w_castle_ks; // White King-side
    bool w_castle_qs; // White Queen-side
    bool b_castle_ks; // Black King-side
    bool b_castle_qs; // Black Queen-side

    // 앙파상 타겟 (-1이면 없음)
    int enPassantTargetSquare;

    // 50수 규칙 카운터
    int fiftyMoveCounter;

    // Zobrist 해시
    uint64_t currentHash;
    std::vector<uint64_t> history;
};

#endif // GAME_H