#include "game.h"
#include "piece.h"
#include <vector>

Game::Game() : p_currentTurn(Piece::WHITE)
{
    // 초기화 시 nullptr 설정
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
    // 킹 위치 복사
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
    // 기존 기물 정리
    for(int x=0; x<8; ++x)
        for(int y=0; y<8; ++y)
            if(m_board[x][y]) { delete m_board[x][y]; m_board[x][y] = nullptr; }

    // 흑(Black) 배치 (y=0,1)
    m_board[0][0] = new Rook(Piece::BLACK);   m_board[7][0] = new Rook(Piece::BLACK);
    m_board[1][0] = new Knight(Piece::BLACK); m_board[6][0] = new Knight(Piece::BLACK);
    m_board[2][0] = new Bishop(Piece::BLACK); m_board[5][0] = new Bishop(Piece::BLACK);
    m_board[3][0] = new Queen(Piece::BLACK);  m_board[4][0] = new King(Piece::BLACK);
    for(int x=0; x<8; ++x) m_board[x][1] = new Pawn(Piece::BLACK);

    // 백(White) 배치 (y=6,7)
    m_board[0][7] = new Rook(Piece::WHITE);   m_board[7][7] = new Rook(Piece::WHITE);
    m_board[1][7] = new Knight(Piece::WHITE); m_board[6][7] = new Knight(Piece::WHITE);
    m_board[2][7] = new Bishop(Piece::WHITE); m_board[5][7] = new Bishop(Piece::WHITE);
    m_board[3][7] = new Queen(Piece::WHITE);  m_board[4][7] = new King(Piece::WHITE);
    for(int x=0; x<8; ++x) m_board[x][6] = new Pawn(Piece::WHITE);

    // 킹 위치 초기화
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

    // 킹 위치 업데이트
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

    // 킹 위치 원복
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

// [대폭 최적화] 킹의 위치에서 역으로 공격자를 탐색 (Reverse Check)
// O(1) - 보드 전체를 돌지 않고 위협이 가능한 경로만 검사
bool Game::isCheck(Piece::PieceColor kingColor) const
{
    int kx, ky;
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

    // 2. 폰 공격 확인 (적 폰이 나를 공격할 수 있는 위치에 있는지 확인)
    // 적 폰의 전진 방향이 내 킹 쪽이어야 함. 즉 내 킹 입장에선 "적 폰이 오는 방향의 반대"를 봐야 함.
    // 예: White King(y=6), Black Pawn은 y가 작은 곳에서 옴. -> 검사는 y-1에서 해야 함.
    int pawnDir = (kingColor == Piece::WHITE) ? -1 : 1; // 적 폰이 있는 y축 방향 (내 기준 위쪽/아래쪽)

    // 주의: pawnDir은 "내 킹이 어디서 공격받나"가 아니라, "적 폰이 이동하는 방향"의 역이어야 함.
    // Black Pawn moves +1 (down). White King checks up (-1).
    // White Pawn moves -1 (up). Black King checks down (+1).

    int py = ky + pawnDir;
    if(py >= 0 && py < 8) {
        // 왼쪽 대각선
        if(kx > 0) {
            Piece* p = m_board[kx-1][py];
            if(p && p->getColor() == enemyColor && p->getType() == Piece::PAWN) return true;
        }
        // 오른쪽 대각선
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
                break; // 내 기물이든 적 기물이든 막히면 더 이상 못 감
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

    // 5. 적 킹 (인접해 있는지 - 사실 정상 게임에선 불가능하지만 룰상 체크)
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

// [최적화] 메모리 할당 최소화 및 generateMoves 구조 개선
vector<Move> Game::generateMoves(Piece::PieceColor color)
{
    vector<Move> possibleMoves;
    possibleMoves.reserve(50); // 평균적인 체스 이동 수만큼 예약

    // 1. 모든 가능한 이동 수집 (Pseudo-legal moves)
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = m_board[x][y];
            if(p && p->getColor() == color)
            {
                // 참조로 넘겨서 벡터 복사 방지
                p->addPossibleMoves(*this, x, y, possibleMoves);
            }
        }
    }

    vector<Move> legalMoves;
    legalMoves.reserve(possibleMoves.size()); // 유효 이동만 담을 벡터

    // 2. 체크 여부 검증 (실제 이동 -> isCheck -> 복구)
    // isCheck가 O(1)로 빨라져서 이 루프가 매우 빨라짐
    for(const auto& move : possibleMoves)
    {
        Piece* captured = this->makeMove(move);

        // 내가 둔 수로 인해 내 왕이 체크 상태가 아니어야 함
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
