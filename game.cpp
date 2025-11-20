#include "game.h"
#include "piece.h"
#include <vector>

Game::Game() : p_currentTurn(Piece::WHITE)
{
    for(int x = 0; x < 8; ++x)
        for(int y = 0; y < 8; ++y)
            m_board[x][y] = nullptr;

    resetBoard();
}

Game::~Game()
{
    for(int x = 0; x < 8; ++x)
        for(int y = 0; y < 8; ++y)
            if(m_board[x][y]) delete m_board[x][y];
}

Game::Game(const Game& other) : p_currentTurn(other.p_currentTurn)
{
    // [최적화] 킹 위치 복사
    m_wKingX = other.m_wKingX; m_wKingY = other.m_wKingY;
    m_bKingX = other.m_bKingX; m_bKingY = other.m_bKingY;

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            if(other.m_board[x][y])
                m_board[x][y] = other.m_board[x][y]->clone();
            else
                m_board[x][y] = nullptr;
        }
    }
}

void Game::resetBoard()
{
    for(int x=0; x<8; ++x)
        for(int y=0; y<8; ++y)
            if(m_board[x][y]) { delete m_board[x][y]; m_board[x][y] = nullptr; }

    // 흑(Black) 배치
    m_board[0][0] = new Rook(Piece::BLACK);   m_board[7][0] = new Rook(Piece::BLACK);
    m_board[1][0] = new Knight(Piece::BLACK); m_board[6][0] = new Knight(Piece::BLACK);
    m_board[2][0] = new Bishop(Piece::BLACK); m_board[5][0] = new Bishop(Piece::BLACK);
    m_board[3][0] = new Queen(Piece::BLACK);  m_board[4][0] = new King(Piece::BLACK);
    for(int x=0; x<8; ++x) m_board[x][1] = new Pawn(Piece::BLACK);

    // 백(White) 배치
    m_board[0][7] = new Rook(Piece::WHITE);   m_board[7][7] = new Rook(Piece::WHITE);
    m_board[1][7] = new Knight(Piece::WHITE); m_board[6][7] = new Knight(Piece::WHITE);
    m_board[2][7] = new Bishop(Piece::WHITE); m_board[5][7] = new Bishop(Piece::WHITE);
    m_board[3][7] = new Queen(Piece::WHITE);  m_board[4][7] = new King(Piece::WHITE);
    for(int x=0; x<8; ++x) m_board[x][6] = new Pawn(Piece::WHITE);

    // [최적화] 킹 위치 초기화
    m_bKingX = 4; m_bKingY = 0;
    m_wKingX = 4; m_wKingY = 7;

    p_currentTurn = Piece::WHITE;
}

Piece* Game::getPiece(int x, int y) const
{
    if(x < 0 || x >= 8 || y < 0 || y >= 8) return nullptr;
    return m_board[x][y];
}

void Game::setPiece(int x, int y, Piece* piece)
{
    if(x < 0 || x >= 8 || y < 0 || y >= 8) return;
    m_board[x][y] = piece;
}

void Game::switchTurn()
{
    p_currentTurn = (p_currentTurn == Piece::WHITE) ? Piece::BLACK : Piece::WHITE;
}

Piece* Game::makeMove(const Move& move)
{
    Piece* capturedPiece = m_board[move.toX][move.toY];
    Piece* pieceToMove = m_board[move.fromX][move.fromY];

    // [최적화] 킹이 이동하는 경우 위치 캐시 업데이트
    if (pieceToMove->getType() == Piece::KING) {
        if (pieceToMove->getColor() == Piece::WHITE) {
            m_wKingX = move.toX; m_wKingY = move.toY;
        } else {
            m_bKingX = move.toX; m_bKingY = move.toY;
        }
    }

    m_board[move.toX][move.toY] = pieceToMove;
    m_board[move.fromX][move.fromY] = nullptr;

    switchTurn();
    return capturedPiece;
}

void Game::unmakeMove(const Move& move, Piece* capturedPiece)
{
    switchTurn();

    Piece* movedPiece = m_board[move.toX][move.toY];

    // [최적화] 킹 위치 원복
    if (movedPiece->getType() == Piece::KING) {
        if (movedPiece->getColor() == Piece::WHITE) {
            m_wKingX = move.fromX; m_wKingY = move.fromY;
        } else {
            m_bKingX = move.fromX; m_bKingY = move.fromY;
        }
    }

    m_board[move.fromX][move.fromY] = movedPiece;
    m_board[move.toX][move.toY] = capturedPiece;
}

Game Game::makeMoveCopy(const Move& move) const
{
    Game newGame = *this;
    newGame.makeMove(move);
    return newGame;
}

// [대폭 최적화] Reverse Check (킹 기준 역탐색)
// 모든 적 기물의 공격 범위를 계산하는 대신, 킹에게 도달할 수 있는 경로만 검사합니다.
// 시간 복잡도가 O(N)에서 거의 O(1)로 줄어듭니다.
bool Game::isCheck(Piece::PieceColor kingColor) const
{
    int kx, ky;
    // 캐싱된 좌표 사용
    if (kingColor == Piece::WHITE) { kx = m_wKingX; ky = m_wKingY; }
    else                           { kx = m_bKingX; ky = m_bKingY; }

    Piece::PieceColor enemyColor = (kingColor == Piece::WHITE) ? Piece::BLACK : Piece::WHITE;

    // 1. 나이트 공격 확인
    static const int knDx[] = {1, 1, 2, 2, -1, -1, -2, -2};
    static const int knDy[] = {2, -2, 1, -1, 2, -2, 1, -1};
    for(int i=0; i<8; ++i) {
        int nx = kx + knDx[i];
        int ny = ky + knDy[i];
        if(nx>=0 && nx<8 && ny>=0 && ny<8) {
            Piece* p = m_board[nx][ny];
            if(p && p->getColor() == enemyColor && p->getType() == Piece::KNIGHT) return true;
        }
    }

    // 2. 폰 공격 확인
    // 내 킹을 공격할 수 있는 '적 폰의 위치'를 확인합니다.
    // 적 폰이 이동해오는 방향의 반대쪽 대각선에 적 폰이 있어야 함
    int pawnDir = (kingColor == Piece::WHITE) ? -1 : 1; 

    int py = ky + pawnDir; 
    if(py >= 0 && py < 8) {
        if(kx > 0) {
            Piece* p = m_board[kx-1][py];
            if(p && p->getColor() == enemyColor && p->getType() == Piece::PAWN) return true;
        }
        if(kx < 7) {
            Piece* p = m_board[kx+1][py];
            if(p && p->getColor() == enemyColor && p->getType() == Piece::PAWN) return true;
        }
    }

    // 3. 직선 공격 (룩, 퀸)
    static const int rDx[] = {0, 0, 1, -1};
    static const int rDy[] = {1, -1, 0, 0};
    for(int i=0; i<4; ++i) {
        int cx = kx, cy = ky;
        while(true) {
            cx += rDx[i]; cy += rDy[i];
            if(cx<0||cx>=8||cy<0||cy>=8) break;
            Piece* p = m_board[cx][cy];
            if(p) {
                if(p->getColor() == enemyColor && (p->getType() == Piece::ROOK || p->getType() == Piece::QUEEN)) return true;
                break; 
            }
        }
    }

    // 4. 대각선 공격 (비숍, 퀸)
    static const int bDx[] = {1, 1, -1, -1};
    static const int bDy[] = {1, -1, 1, -1};
    for(int i=0; i<4; ++i) {
        int cx = kx, cy = ky;
        while(true) {
            cx += bDx[i]; cy += bDy[i];
            if(cx<0||cx>=8||cy<0||cy>=8) break;
            Piece* p = m_board[cx][cy];
            if(p) {
                if(p->getColor() == enemyColor && (p->getType() == Piece::BISHOP || p->getType() == Piece::QUEEN)) return true;
                break;
            }
        }
    }

    // 5. 적 킹 인접 확인 (규칙상 불가하지만 체크)
    for(int dx=-1; dx<=1; ++dx){
        for(int dy=-1; dy<=1; ++dy){
            if(dx==0 && dy==0) continue;
            int nx = kx+dx, ny = ky+dy;
            if(nx>=0 && nx<8 && ny>=0 && ny<8) {
                Piece* p = m_board[nx][ny];
                if(p && p->getColor() == enemyColor && p->getType() == Piece::KING) return true;
            }
        }
    }

    return false;
}

vector<Move> Game::generateMoves(Piece::PieceColor color)
{
    vector<Move> possibleMoves;
    possibleMoves.reserve(64); // 메모리 재할당 최소화

    // 1. 모든 기물의 기본 이동 수집 (Pseudo-legal)
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = m_board[x][y];
            if(p && p->getColor() == color)
            {
                // [최적화] 벡터 참조 전달
                p->addPossibleMoves(*this, x, y, possibleMoves);
            }
        }
    }

    vector<Move> legalMoves;
    legalMoves.reserve(possibleMoves.size());

    // 2. 체크 상태 검증 (isCheck 최적화로 인해 매우 빨라짐)
    for(const auto& move : possibleMoves)
    {
        Piece* captured = this->makeMove(move);

        if(!this->isCheck(color))
        {
            legalMoves.push_back(move);
        }

        this->unmakeMove(move, captured);
    }

    return legalMoves;
}

bool Game::isGameOver()
{
    vector<Move> moves = generateMoves(p_currentTurn);
    return moves.empty();
}