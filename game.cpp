#include "game.h"
#include "piece.h"
#include "ZobristHasher.h"
#include <vector>
#include <algorithm>
#include <memory>

Game::Game() : p_currentTurn(Chess::WHITE)
{
    resetBoard();
}

Game::~Game() = default;

Game::Game(const Game& other) : p_currentTurn(other.p_currentTurn)
{
    // [최적화] 킹 위치 복사
    m_wKingX = other.m_wKingX; m_wKingY = other.m_wKingY;
    m_bKingX = other.m_bKingX; m_bKingY = other.m_bKingY;

    // 캐슬링 권한 복사
    w_castle_ks = other.w_castle_ks;
    w_castle_qs = other.w_castle_qs;
    b_castle_ks = other.b_castle_ks;
    b_castle_qs = other.b_castle_qs;

    // 앙파상 정보 복사
    enPassantTargetSquare = other.enPassantTargetSquare;

    // 50수 및 해시 정보 복사
    fiftyMoveCounter = other.fiftyMoveCounter;
    currentHash = other.currentHash;
    history = other.history;

    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            if(other.m_board[x][y])
                m_board[x][y] = std::unique_ptr<Piece>(other.m_board[x][y]->clone());
            else
                m_board[x][y] = nullptr;
        }
    }
}

void Game::resetBoard()
{
    for(int x=0; x<8; ++x)
        for(int y=0; y<8; ++y)
            m_board[x][y].reset();

    // 흑(Black) 배치
    m_board[0][0] = std::make_unique<Rook>(Chess::BLACK);   m_board[7][0] = std::make_unique<Rook>(Chess::BLACK);
    m_board[1][0] = std::make_unique<Knight>(Chess::BLACK); m_board[6][0] = std::make_unique<Knight>(Chess::BLACK);
    m_board[2][0] = std::make_unique<Bishop>(Chess::BLACK); m_board[5][0] = std::make_unique<Bishop>(Chess::BLACK);
    m_board[3][0] = std::make_unique<Queen>(Chess::BLACK);  m_board[4][0] = std::make_unique<King>(Chess::BLACK);
    for(int x=0; x<8; ++x) m_board[x][1] = std::make_unique<Pawn>(Chess::BLACK);

    // 백(White) 배치
    m_board[0][7] = std::make_unique<Rook>(Chess::WHITE);   m_board[7][7] = std::make_unique<Rook>(Chess::WHITE);
    m_board[1][7] = std::make_unique<Knight>(Chess::WHITE); m_board[6][7] = std::make_unique<Knight>(Chess::WHITE);
    m_board[2][7] = std::make_unique<Bishop>(Chess::WHITE); m_board[5][7] = std::make_unique<Bishop>(Chess::WHITE);
    m_board[3][7] = std::make_unique<Queen>(Chess::WHITE);  m_board[4][7] = std::make_unique<King>(Chess::WHITE);
    for(int x=0; x<8; ++x) m_board[x][6] = std::make_unique<Pawn>(Chess::WHITE);

    // [최적화] 킹 위치 초기화
    m_bKingX = 4; m_bKingY = 0;
    m_wKingX = 4; m_wKingY = 7;

    // 캐슬링 권한 초기화
    w_castle_ks = true;
    w_castle_qs = true;
    b_castle_ks = true;
    b_castle_qs = true;

    // 앙파상 초기화
    enPassantTargetSquare = -1;

    // 50수 카운터 초기화
    fiftyMoveCounter = 0;

    p_currentTurn = Chess::WHITE;

    // 해시 초기화
    history.clear();
    currentHash = ZobristHasher::getInstance().calculateHash(*this);
    history.push_back(currentHash);
}

Piece* Game::getPiece(int x, int y) const
{
    if(x < 0 || x >= 8 || y < 0 || y >= 8) return nullptr;
    return m_board[x][y].get();
}

void Game::setPiece(int x, int y, std::unique_ptr<Piece> piece)
{
    if(x < 0 || x >= 8 || y < 0 || y >= 8) return;
    m_board[x][y] = std::move(piece);
}

void Game::switchTurn()
{
    p_currentTurn = (p_currentTurn == Chess::WHITE) ? Chess::BLACK : Chess::WHITE;
}

UndoInfo Game::makeMove(const Move& move)
{
    UndoInfo undo;
    undo.w_castle_ks = w_castle_ks;
    undo.w_castle_qs = w_castle_qs;
    undo.b_castle_ks = b_castle_ks;
    undo.b_castle_qs = b_castle_qs;
    undo.enPassantTargetSquare = enPassantTargetSquare;
    undo.fiftyMoveCounter = fiftyMoveCounter;
    undo.currentHash = currentHash;

    const auto& hasher = ZobristHasher::getInstance();
    uint64_t newHash = currentHash;

    Piece* pieceToMoveRaw = m_board[move.fromX][move.fromY].get();
    Chess::PieceColor movingColor = pieceToMoveRaw->getColor();

    // 1. 턴 변경 해시
    newHash ^= hasher.getBlackTurnKey();

    // 2. 이전 앙파상 타겟이 있었다면 해시에서 제거
    if (enPassantTargetSquare != -1) {
        newHash ^= hasher.getEnPassantKey(enPassantTargetSquare % 8);
    }

    // 3. 50수 카운터 및 캡처 처리
    fiftyMoveCounter++;
    std::unique_ptr<Piece> capturedPiece = nullptr;
    undo.capturedPieceType = Chess::EMPTY;

    if (move.moveType == Move::EN_PASSANT) {
        int capturedPawnY = (movingColor == Chess::WHITE) ? move.toY + 1 : move.toY - 1;
        capturedPiece = std::move(m_board[move.toX][capturedPawnY]);
        if (capturedPiece) {
            undo.capturedPieceType = capturedPiece->getType();
            undo.capturedPieceColor = capturedPiece->getColor();
            newHash ^= hasher.getPieceKey(capturedPiece->getType(), capturedPiece->getColor(), move.toX, capturedPawnY);
        }
        fiftyMoveCounter = 0;
    } else {
        capturedPiece = std::move(m_board[move.toX][move.toY]);
        if (capturedPiece) {
            undo.capturedPieceType = capturedPiece->getType();
            undo.capturedPieceColor = capturedPiece->getColor();
            newHash ^= hasher.getPieceKey(capturedPiece->getType(), capturedPiece->getColor(), move.toX, move.toY);
            fiftyMoveCounter = 0;
        }
    }
    if (pieceToMoveRaw->getType() == Chess::PAWN) {
        fiftyMoveCounter = 0;
    }

    // 4. 기물 이동 (출발지에서 제거)
    auto pieceToMove = std::move(m_board[move.fromX][move.fromY]);
    newHash ^= hasher.getPieceKey(pieceToMove->getType(), movingColor, move.fromX, move.fromY);

    // 5. 킹 위치 및 캐슬링 권한 업데이트
    if (pieceToMove->getType() == Chess::KING) {
        if (movingColor == Chess::WHITE) {
            if(w_castle_ks) newHash ^= hasher.getCastleKey(Chess::WHITE, true);
            if(w_castle_qs) newHash ^= hasher.getCastleKey(Chess::WHITE, false);
            w_castle_ks = false; w_castle_qs = false;
            m_wKingX = move.toX; m_wKingY = move.toY;
        } else {
            if(b_castle_ks) newHash ^= hasher.getCastleKey(Chess::BLACK, true);
            if(b_castle_qs) newHash ^= hasher.getCastleKey(Chess::BLACK, false);
            b_castle_ks = false; b_castle_qs = false;
            m_bKingX = move.toX; m_bKingY = move.toY;
        }
    }
    if (move.fromX == 0 && move.fromY == 7 && w_castle_qs) { newHash ^= hasher.getCastleKey(Chess::WHITE, false); w_castle_qs = false; }
    if (move.fromX == 7 && move.fromY == 7 && w_castle_ks) { newHash ^= hasher.getCastleKey(Chess::WHITE, true); w_castle_ks = false; }
    if (move.fromX == 0 && move.fromY == 0 && b_castle_qs) { newHash ^= hasher.getCastleKey(Chess::BLACK, false); b_castle_qs = false; }
    if (move.fromX == 7 && move.fromY == 0 && b_castle_ks) { newHash ^= hasher.getCastleKey(Chess::BLACK, true); b_castle_ks = false; }

    // 6. 캐슬링 시 룩 이동
    if (move.moveType == Move::CASTLE_KS) {
        auto rook = std::move(m_board[7][move.fromY]);
        newHash ^= hasher.getPieceKey(Chess::ROOK, movingColor, 7, move.fromY);
        newHash ^= hasher.getPieceKey(Chess::ROOK, movingColor, 5, move.fromY);
        m_board[5][move.fromY] = std::move(rook);
    } else if (move.moveType == Move::CASTLE_QS) {
        auto rook = std::move(m_board[0][move.fromY]);
        newHash ^= hasher.getPieceKey(Chess::ROOK, movingColor, 0, move.fromY);
        newHash ^= hasher.getPieceKey(Chess::ROOK, movingColor, 3, move.fromY);
        m_board[3][move.fromY] = std::move(rook);
    }

    // 7. 프로모션 처리
    if (move.promotionType != 0) { // Chess::EMPTY 대신 0 사용
        switch((Chess::PieceType)move.promotionType) {
            case Chess::QUEEN: pieceToMove = std::make_unique<Queen>(movingColor); break;
            case Chess::ROOK: pieceToMove = std::make_unique<Rook>(movingColor); break;
            case Chess::BISHOP: pieceToMove = std::make_unique<Bishop>(movingColor); break;
            case Chess::KNIGHT: pieceToMove = std::make_unique<Knight>(movingColor); break;
            default: break; // Should not happen
        }
    }
    newHash ^= hasher.getPieceKey(pieceToMove->getType(), movingColor, move.toX, move.toY);
    m_board[move.toX][move.toY] = std::move(pieceToMove);

    // 8. 새로운 앙파상 타겟 설정
    if (m_board[move.toX][move.toY]->getType() == Chess::PAWN && abs(move.fromY - move.toY) == 2) {
        enPassantTargetSquare = move.fromX + (move.fromY + move.toY) / 2 * 8;
        newHash ^= hasher.getEnPassantKey(enPassantTargetSquare % 8);
    } else {
        enPassantTargetSquare = -1;
    }

    switchTurn();
    currentHash = newHash;
    history.push_back(currentHash);
    return undo;
}

void Game::unmakeMove(const Move& move, const UndoInfo& undo)
{
    history.pop_back();
    currentHash = undo.currentHash;
    fiftyMoveCounter = undo.fiftyMoveCounter;
    enPassantTargetSquare = undo.enPassantTargetSquare;
    w_castle_ks = undo.w_castle_ks;
    w_castle_qs = undo.w_castle_qs;
    b_castle_ks = undo.b_castle_ks;
    b_castle_qs = undo.b_castle_qs;

    switchTurn();

    auto movedPiece = std::move(m_board[move.toX][move.toY]);
    Chess::PieceColor movingColor = movedPiece->getColor();

    if (move.promotionType != 0) { // Chess::EMPTY 대신 0 사용
        movedPiece = std::make_unique<Pawn>(movingColor);
    }

    if (movedPiece->getType() == Chess::KING) {
        if (movingColor == Chess::WHITE) { m_wKingX = move.fromX; m_wKingY = move.fromY; }
        else { m_bKingX = move.fromX; m_bKingY = move.fromY; }
    }

    m_board[move.fromX][move.fromY] = std::move(movedPiece);

    if (move.moveType == Move::EN_PASSANT) {
        m_board[move.toX][move.toY].reset();
        int capturedPawnY = (movingColor == Chess::WHITE) ? move.toY + 1 : move.toY - 1;
        
        // 캡처된 폰 재생성
        m_board[move.toX][capturedPawnY] = std::make_unique<Pawn>(undo.capturedPieceColor);

    } else {
        // 캡처된 기물 재생성
        if (undo.capturedPieceType != Chess::EMPTY) {
            switch(undo.capturedPieceType) {
                case Chess::PAWN:   m_board[move.toX][move.toY] = std::make_unique<Pawn>(undo.capturedPieceColor); break;
                case Chess::KNIGHT: m_board[move.toX][move.toY] = std::make_unique<Knight>(undo.capturedPieceColor); break;
                case Chess::BISHOP: m_board[move.toX][move.toY] = std::make_unique<Bishop>(undo.capturedPieceColor); break;
                case Chess::ROOK:   m_board[move.toX][move.toY] = std::make_unique<Rook>(undo.capturedPieceColor); break;
                case Chess::QUEEN:  m_board[move.toX][move.toY] = std::make_unique<Queen>(undo.capturedPieceColor); break;
                case Chess::KING:   m_board[move.toX][move.toY] = std::make_unique<King>(undo.capturedPieceColor); break; // Should not happen
                default: m_board[move.toX][move.toY].reset(); break;
            }
        } else {
            m_board[move.toX][move.toY].reset();
        }
    }

    if (move.moveType == Move::CASTLE_KS) {
        auto rook = std::move(m_board[5][move.fromY]);
        m_board[7][move.fromY] = std::move(rook);
    } else if (move.moveType == Move::CASTLE_QS) {
        auto rook = std::move(m_board[3][move.fromY]);
        m_board[0][move.fromY] = std::move(rook);
    }
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
bool Game::isCheck(Chess::PieceColor kingColor) const
{
    int kx, ky;
    // 캐싱된 좌표 사용
    if (kingColor == Chess::WHITE) { kx = m_wKingX; ky = m_wKingY; }
    else                           { kx = m_bKingX; ky = m_bKingY; }

    Chess::PieceColor enemyColor = (kingColor == Chess::WHITE) ? Chess::BLACK : Chess::WHITE;

    // 1. 나이트 공격 확인
    static const int knDx[] = {1, 1, 2, 2, -1, -1, -2, -2};
    static const int knDy[] = {2, -2, 1, -1, 2, -2, 1, -1};
    for(int i=0; i<8; ++i) {
        int nx = kx + knDx[i];
        int ny = ky + knDy[i];
        if(nx>=0 && nx<8 && ny>=0 && ny<8) {
            Piece* p = m_board[nx][ny].get();
            if(p && p->getColor() == enemyColor && p->getType() == Chess::KNIGHT) return true;
        }
    }

    // 2. 폰 공격 확인
    // 내 킹을 공격할 수 있는 '적 폰의 위치'를 확인합니다.
    // 적 폰이 이동해오는 방향의 반대쪽 대각선에 적 폰이 있어야 함
    int pawnDir = (kingColor == Chess::WHITE) ? -1 : 1; 

    int py = ky + pawnDir; 
    if(py >= 0 && py < 8) {
        if(kx > 0) {
            Piece* p = m_board[kx-1][py].get();
            if(p && p->getColor() == enemyColor && p->getType() == Chess::PAWN) return true;
        }
        if(kx < 7) {
            Piece* p = m_board[kx+1][py].get();
            if(p && p->getColor() == enemyColor && p->getType() == Chess::PAWN) return true;
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
            Piece* p = m_board[cx][cy].get();
            if(p) {
                if(p->getColor() == enemyColor && (p->getType() == Chess::ROOK || p->getType() == Chess::QUEEN)) return true;
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
            Piece* p = m_board[cx][cy].get();
            if(p) {
                if(p->getColor() == enemyColor && (p->getType() == Chess::BISHOP || p->getType() == Chess::QUEEN)) return true;
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
                Piece* p = m_board[nx][ny].get();
                if(p && p->getColor() == enemyColor && p->getType() == Chess::KING) return true;
            }
        }
    }

    return false;
}

bool Game::isSquareAttacked(int x, int y, Chess::PieceColor attackerColor) const
{
    // 1. 나이트 공격 확인
    static const int knDx[] = {1, 1, 2, 2, -1, -1, -2, -2};
    static const int knDy[] = {2, -2, 1, -1, 2, -2, 1, -1};
    for(int i=0; i<8; ++i) {
        int nx = x + knDx[i];
        int ny = y + knDy[i];
        if(nx>=0 && nx<8 && ny>=0 && ny<8) {
            Piece* p = m_board[nx][ny].get();
            if(p && p->getColor() == attackerColor && p->getType() == Chess::KNIGHT) return true;
        }
    }

    // 2. 폰 공격 확인
    int pawnDir = (attackerColor == Chess::WHITE) ? -1 : 1; // 백은 y 감소, 흑은 y 증가 방향으로 공격
    int py = y + pawnDir;
    if(py >= 0 && py < 8) {
        if(x > 0) {
            Piece* p = m_board[x-1][py].get();
            if(p && p->getColor() == attackerColor && p->getType() == Chess::PAWN) return true;
        }
        if(x < 7) {
            Piece* p = m_board[x+1][py].get();
            if(p && p->getColor() == attackerColor && p->getType() == Chess::PAWN) return true;
        }
    }

    // 3. 직선 공격 (룩, 퀸)
    static const int rDx[] = {0, 0, 1, -1};
    static const int rDy[] = {1, -1, 0, 0};
    for(int i=0; i<4; ++i) {
        int cx = x, cy = y;
        while(true) {
            cx += rDx[i]; cy += rDy[i];
            if(cx<0||cx>=8||cy<0||cy>=8) break;
            Piece* p = m_board[cx][cy].get();
            if(p) {
                if(p->getColor() == attackerColor && (p->getType() == Chess::ROOK || p->getType() == Chess::QUEEN)) return true;
                break;
            }
        }
    }

    // 4. 대각선 공격 (비숍, 퀸)
    static const int bDx[] = {1, 1, -1, -1};
    static const int bDy[] = {1, -1, 1, -1};
    for(int i=0; i<4; ++i) {
        int cx = x, cy = y;
        while(true) {
            cx += bDx[i]; cy += bDy[i];
            if(cx<0||cx>=8||cy<0||cy>=8) break;
            Piece* p = m_board[cx][cy].get();
            if(p) {
                if(p->getColor() == attackerColor && (p->getType() == Chess::BISHOP || p->getType() == Chess::QUEEN)) return true;
                break;
            }
        }
    }

    // 5. 적 킹 인접 확인
    for(int dx=-1; dx<=1; ++dx){
        for(int dy=-1; dy<=1; ++dy){
            if(dx==0 && dy==0) continue;
            int nx = x+dx, ny = y+dy;
            if(nx>=0 && nx<8 && ny>=0 && ny<8) {
                Piece* p = m_board[nx][ny].get();
                if(p && p->getColor() == attackerColor && p->getType() == Chess::KING) return true;
            }
        }
    }

    return false;
}

bool Game::isInsufficientMaterial() const
{
    int white_knights = 0;
    int white_bishops = 0;
    int black_knights = 0;
    int black_bishops = 0;
    int num_pieces = 0;

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            Piece* p = m_board[x][y].get();
            if (p == nullptr || p->getType() == Chess::KING) continue;

            // 퀸, 룩, 폰이 하나라도 있으면 기물 부족 아님
            if (p->getType() == Chess::QUEEN || p->getType() == Chess::ROOK || p->getType() == Chess::PAWN) {
                return false;
            }

            num_pieces++;

            if (p->getColor() == Chess::WHITE) {
                if (p->getType() == Chess::KNIGHT) white_knights++;
                else if (p->getType() == Chess::BISHOP) white_bishops++;
            } else {
                if (p->getType() == Chess::KNIGHT) black_knights++;
                else if (p->getType() == Chess::BISHOP) black_bishops++;
            }
        }
    }

    // 킹 vs 킹
    if (num_pieces == 0) return true;

    // 킹 vs 킹 + 마이너 피스
    if (white_knights + white_bishops + black_knights + black_bishops == 1) {
        return true;
    }

    // 킹 + 비숍 vs 킹 + 비숍 (같은 색 칸)
    if (white_bishops == 1 && black_bishops == 1 && num_pieces == 2) {
        int b1_color = -1, b2_color = -1;
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                Piece* p = m_board[x][y].get();
                if (p != nullptr && p->getType() == Chess::BISHOP) {
                    if (b1_color == -1) b1_color = (x + y) % 2;
                    else b2_color = (x + y) % 2;
                }
            }
        }
        if (b1_color == b2_color) return true;
    }

    return false;
}

std::vector<Move> Game::generateMoves(int x, int y)
{
    std::vector<Move> legalMoves;
    Piece* p = getPiece(x, y);
    
    if (!p || p->getColor() != p_currentTurn) return legalMoves;

    std::vector<Move> possibleMoves;
    possibleMoves.reserve(28); // 기물 하나당 최대 이동 수 (퀸 등)
    
    p->addPossibleMoves(*this, x, y, possibleMoves);

    for(const auto& move : possibleMoves)
    {
        UndoInfo undo = this->makeMove(move);

        // [수정] makeMove 후에는 턴이 바뀌므로 p_currentTurn은 상대방입니다.
        // 이동한 기물의 색상(p->getColor())이 체크 상태인지 확인해야 합니다.
        if(!this->isCheck(p->getColor()))
        {
            legalMoves.push_back(move);
        }

        this->unmakeMove(move, undo);
    }

    return legalMoves;
}

std::vector<Move> Game::generateMoves(Chess::PieceColor color)
{
    std::vector<Move> possibleMoves;
    possibleMoves.reserve(64); // 메모리 재할당 최소화

    // 1. 모든 기물의 기본 이동 수집 (Pseudo-legal)
    for(int x = 0; x < 8; ++x)
    {
        for(int y = 0; y < 8; ++y)
        {
            Piece* p = m_board[x][y].get();
            if(p && p->getColor() == color)
            {
                // [최적화] 벡터 참조 전달
                p->addPossibleMoves(*this, x, y, possibleMoves);
            }
        }
    }

    std::vector<Move> legalMoves;
    legalMoves.reserve(possibleMoves.size());

    // 2. 체크 상태 검증 (isCheck 최적화로 인해 매우 빨라짐)
    for(const auto& move : possibleMoves)
    {
        UndoInfo undo = this->makeMove(move);

        if(!this->isCheck(color))
        {
            legalMoves.push_back(move);
        }

        this->unmakeMove(move, undo);
    }

    return legalMoves;
}

Game::GameState Game::getGameState()
{
    // 1. 50수 규칙 검사
    if (fiftyMoveCounter >= 100) {
        return DRAW;
    }

    // 2. 3회 동형 반복 검사
    if (history.size() > 4) {
        if (std::count(history.begin(), history.end() - 1, currentHash) >= 2) {
            return DRAW;
        }
    }
    
    // 3. 기물 부족 검사
    if (isInsufficientMaterial()) {
        return DRAW;
    }

    // 4. 체크메이트 / 스테일메이트 검사
    std::vector<Move> moves = generateMoves(p_currentTurn);
    if (moves.empty()) {
        if (isCheck(p_currentTurn)) {
            return CHECKMATE;
        } else {
            return STALEMATE;
        }
    }

    return IN_PROGRESS;
}

bool Game::isGameOver()
{
    // getGameState는 non-const이므로 const_cast를 사용하거나,
    // getGameState를 const로 만들거나, isGameOver에서 로직을 중복 구현해야 합니다.
    // generateMoves가 non-const인 것이 문제의 근원입니다.
    // generateMoves를 const로 변경하는 것이 가장 이상적이지만,
    // 내부적으로 makeMove/unmakeMove를 사용하므로 수정 범위가 커집니다.
    // 여기서는 const 한정자를 제거하여 문제를 해결합니다.
    return getGameState() != IN_PROGRESS;
}

bool Game::canCastle(Chess::PieceColor color, bool isKingSide) const
{
    if (color == Chess::WHITE) {
        return isKingSide ? w_castle_ks : w_castle_qs;
    } else {
        return isKingSide ? b_castle_ks : b_castle_qs;
    }
}
