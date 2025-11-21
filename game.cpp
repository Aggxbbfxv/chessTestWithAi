#include "game.h"
#include "piece.h"
#include "ZobristHasher.h"
#include <vector>
#include <algorithm>

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

    // 캐슬링 권한 초기화
    w_castle_ks = true;
    w_castle_qs = true;
    b_castle_ks = true;
    b_castle_qs = true;

    // 앙파상 초기화
    enPassantTargetSquare = -1;

    // 50수 카운터 초기화
    fiftyMoveCounter = 0;

    p_currentTurn = Piece::WHITE;

    // 해시 초기화
    history.clear();
    currentHash = ZobristHasher::getInstance().calculateHash(*this);
    history.push_back(currentHash);
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

    Piece* pieceToMove = m_board[move.fromX][move.fromY];
    Piece::PieceColor movingColor = pieceToMove->getColor();

    // 1. 턴 변경 해시
    newHash ^= hasher.getBlackTurnKey();

    // 2. 이전 앙파상 타겟이 있었다면 해시에서 제거
    if (enPassantTargetSquare != -1) {
        newHash ^= hasher.getEnPassantKey(enPassantTargetSquare % 8);
    }

    // 3. 50수 카운터 및 캡처 처리
    fiftyMoveCounter++;
    Piece* capturedPiece = nullptr;
    if (move.moveType == Move::EN_PASSANT) {
        int capturedPawnY = (movingColor == Piece::WHITE) ? move.toY + 1 : move.toY - 1;
        capturedPiece = m_board[move.toX][capturedPawnY];
        newHash ^= hasher.getPieceKey(capturedPiece->getType(), capturedPiece->getColor(), move.toX, capturedPawnY);
        m_board[move.toX][capturedPawnY] = nullptr;
        fiftyMoveCounter = 0;
    } else {
        capturedPiece = m_board[move.toX][move.toY];
        if (capturedPiece != nullptr) {
            newHash ^= hasher.getPieceKey(capturedPiece->getType(), capturedPiece->getColor(), move.toX, move.toY);
            fiftyMoveCounter = 0;
        }
    }
    if (pieceToMove->getType() == Piece::PAWN) {
        fiftyMoveCounter = 0;
    }
    undo.capturedPiece = capturedPiece;

    // 4. 기물 이동 (출발지에서 제거)
    m_board[move.fromX][move.fromY] = nullptr;
    newHash ^= hasher.getPieceKey(pieceToMove->getType(), movingColor, move.fromX, move.fromY);

    // 5. 킹 위치 및 캐슬링 권한 업데이트
    if (pieceToMove->getType() == Piece::KING) {
        if (movingColor == Piece::WHITE) {
            if(w_castle_ks) newHash ^= hasher.getCastleKey(Piece::WHITE, true);
            if(w_castle_qs) newHash ^= hasher.getCastleKey(Piece::WHITE, false);
            w_castle_ks = false; w_castle_qs = false;
            m_wKingX = move.toX; m_wKingY = move.toY;
        } else {
            if(b_castle_ks) newHash ^= hasher.getCastleKey(Piece::BLACK, true);
            if(b_castle_qs) newHash ^= hasher.getCastleKey(Piece::BLACK, false);
            b_castle_ks = false; b_castle_qs = false;
            m_bKingX = move.toX; m_bKingY = move.toY;
        }
    }
    if (move.fromX == 0 && move.fromY == 7 && w_castle_qs) { newHash ^= hasher.getCastleKey(Piece::WHITE, false); w_castle_qs = false; }
    if (move.fromX == 7 && move.fromY == 7 && w_castle_ks) { newHash ^= hasher.getCastleKey(Piece::WHITE, true); w_castle_ks = false; }
    if (move.fromX == 0 && move.fromY == 0 && b_castle_qs) { newHash ^= hasher.getCastleKey(Piece::BLACK, false); b_castle_qs = false; }
    if (move.fromX == 7 && move.fromY == 0 && b_castle_ks) { newHash ^= hasher.getCastleKey(Piece::BLACK, true); b_castle_ks = false; }

    // 6. 캐슬링 시 룩 이동
    if (move.moveType == Move::CASTLE_KS) {
        Piece* rook = m_board[7][move.fromY];
        m_board[5][move.fromY] = rook;
        m_board[7][move.fromY] = nullptr;
        newHash ^= hasher.getPieceKey(Piece::ROOK, movingColor, 7, move.fromY);
        newHash ^= hasher.getPieceKey(Piece::ROOK, movingColor, 5, move.fromY);
    } else if (move.moveType == Move::CASTLE_QS) {
        Piece* rook = m_board[0][move.fromY];
        m_board[3][move.fromY] = rook;
        m_board[0][move.fromY] = nullptr;
        newHash ^= hasher.getPieceKey(Piece::ROOK, movingColor, 0, move.fromY);
        newHash ^= hasher.getPieceKey(Piece::ROOK, movingColor, 3, move.fromY);
    }

    // 7. 프로모션 처리
    if (move.promotionType != 0) { // Piece::EMPTY 대신 0 사용
        delete pieceToMove; // 폰 삭제
        switch((Piece::PieceType)move.promotionType) {
            case Piece::QUEEN: pieceToMove = new Queen(movingColor); break;
            case Piece::ROOK: pieceToMove = new Rook(movingColor); break;
            case Piece::BISHOP: pieceToMove = new Bishop(movingColor); break;
            case Piece::KNIGHT: pieceToMove = new Knight(movingColor); break;
            default: break; // Should not happen
        }
    }
    m_board[move.toX][move.toY] = pieceToMove;
    newHash ^= hasher.getPieceKey(pieceToMove->getType(), movingColor, move.toX, move.toY);

    // 8. 새로운 앙파상 타겟 설정
    if (pieceToMove->getType() == Piece::PAWN && abs(move.fromY - move.toY) == 2) {
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

    Piece* movedPiece = m_board[move.toX][move.toY];
    Piece::PieceColor movingColor = movedPiece->getColor();

    if (move.promotionType != 0) { // Piece::EMPTY 대신 0 사용
        delete movedPiece;
        movedPiece = new Pawn(movingColor);
    }

    m_board[move.fromX][move.fromY] = movedPiece;

    if (movedPiece->getType() == Piece::KING) {
        if (movingColor == Piece::WHITE) { m_wKingX = move.fromX; m_wKingY = move.fromY; }
        else { m_bKingX = move.fromX; m_bKingY = move.fromY; }
    }

    if (move.moveType == Move::EN_PASSANT) {
        m_board[move.toX][move.toY] = nullptr;
        int capturedPawnY = (movingColor == Piece::WHITE) ? move.toY + 1 : move.toY - 1;
        m_board[move.toX][capturedPawnY] = undo.capturedPiece;
    } else {
        m_board[move.toX][move.toY] = undo.capturedPiece;
    }

    if (move.moveType == Move::CASTLE_KS) {
        Piece* rook = m_board[5][move.fromY];
        m_board[7][move.fromY] = rook;
        m_board[5][move.fromY] = nullptr;
    } else if (move.moveType == Move::CASTLE_QS) {
        Piece* rook = m_board[3][move.fromY];
        m_board[0][move.fromY] = rook;
        m_board[3][move.fromY] = nullptr;
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

bool Game::isSquareAttacked(int x, int y, Piece::PieceColor attackerColor) const
{
    // 1. 나이트 공격 확인
    static const int knDx[] = {1, 1, 2, 2, -1, -1, -2, -2};
    static const int knDy[] = {2, -2, 1, -1, 2, -2, 1, -1};
    for(int i=0; i<8; ++i) {
        int nx = x + knDx[i];
        int ny = y + knDy[i];
        if(nx>=0 && nx<8 && ny>=0 && ny<8) {
            Piece* p = m_board[nx][ny];
            if(p && p->getColor() == attackerColor && p->getType() == Piece::KNIGHT) return true;
        }
    }

    // 2. 폰 공격 확인
    int pawnDir = (attackerColor == Piece::WHITE) ? 1 : -1;
    int py = y + pawnDir;
    if(py >= 0 && py < 8) {
        if(x > 0) {
            Piece* p = m_board[x-1][py];
            if(p && p->getColor() == attackerColor && p->getType() == Piece::PAWN) return true;
        }
        if(x < 7) {
            Piece* p = m_board[x+1][py];
            if(p && p->getColor() == attackerColor && p->getType() == Piece::PAWN) return true;
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
            Piece* p = m_board[cx][cy];
            if(p) {
                if(p->getColor() == attackerColor && (p->getType() == Piece::ROOK || p->getType() == Piece::QUEEN)) return true;
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
            Piece* p = m_board[cx][cy];
            if(p) {
                if(p->getColor() == attackerColor && (p->getType() == Piece::BISHOP || p->getType() == Piece::QUEEN)) return true;
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
                Piece* p = m_board[nx][ny];
                if(p && p->getColor() == attackerColor && p->getType() == Piece::KING) return true;
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
            Piece* p = m_board[x][y];
            if (p == nullptr || p->getType() == Piece::KING) continue;

            // 퀸, 룩, 폰이 하나라도 있으면 기물 부족 아님
            if (p->getType() == Piece::QUEEN || p->getType() == Piece::ROOK || p->getType() == Piece::PAWN) {
                return false;
            }

            num_pieces++;

            if (p->getColor() == Piece::WHITE) {
                if (p->getType() == Piece::KNIGHT) white_knights++;
                else if (p->getType() == Piece::BISHOP) white_bishops++;
            } else {
                if (p->getType() == Piece::KNIGHT) black_knights++;
                else if (p->getType() == Piece::BISHOP) black_bishops++;
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
                Piece* p = m_board[x][y];
                if (p != nullptr && p->getType() == Piece::BISHOP) {
                    if (b1_color == -1) b1_color = (x + y) % 2;
                    else b2_color = (x + y) % 2;
                }
            }
        }
        if (b1_color == b2_color) return true;
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
    vector<Move> moves = generateMoves(p_currentTurn);
    if (moves.empty()) {
        if (isCheck(p_currentTurn)) {
            return CHECKMATE;
        } else {
            return STALEMATE;
        }
    }

    return IN_PROGRESS;
}

bool Game::isGameOver() const
{
    // getGameState는 non-const이므로 const_cast를 사용하거나,
    // getGameState를 const로 만들거나, isGameOver에서 로직을 중복 구현해야 합니다.
    // generateMoves가 non-const인 것이 문제의 근원입니다.
    // generateMoves를 const로 변경하는 것이 가장 이상적이지만,
    // 내부적으로 makeMove/unmakeMove를 사용하므로 수정 범위가 커집니다.
    // 여기서는 임시로 const_cast를 사용하여 문제를 해결합니다.
    return const_cast<Game*>(this)->getGameState() != IN_PROGRESS;
}

bool Game::canCastle(Piece::PieceColor color, bool isKingSide) const
{
    if (color == Piece::WHITE) {
        return isKingSide ? w_castle_ks : w_castle_qs;
    } else {
        return isKingSide ? b_castle_ks : b_castle_qs;
    }
}