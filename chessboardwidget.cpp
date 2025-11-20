#include "chessboardwidget.h"
#include <QDebug>
#include <QCoreApplication>
// QPainter, QFont, QColor 등은 chessboardwidget.h에 이미 포함된
// <QPainter>, <QMouseEvent> 등을 통해 간접적으로 포함됩니다.

ChessBoardWidget::ChessBoardWidget(QWidget *parent)
    : QWidget(parent), m_selectedPos(-1, -1), m_currentMode(customMode)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

QSize ChessBoardWidget::sizeHint() const
{
    return QSize(600, 600);
}

void ChessBoardWidget::setGameMode(gameMode mode)
{
    m_currentMode = mode;
    if (mode == aiMode) qDebug() << "Mode Set: AI (Black)";
    else qDebug() << "Mode Set: Custom (PvP)";
}

void ChessBoardWidget::startGame()
{
    m_game.resetBoard();
    m_selectedPos = QPoint(-1, -1);
    m_validMoves.clear();
    update();
    qDebug() << "Game Started!";
}

void ChessBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 안티앨리어싱으로 부드럽게

    drawBoard(painter);
    drawHighlights(painter); // 조각(drawPieces)보다 먼저 그려야 하이라이트가 조각을 덮지 않습니다.
    drawPieces(painter);
}

void ChessBoardWidget::drawBoard(QPainter &painter)
{
    int w = width() / 8;
    int h = height() / 8;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            // 클래식한 나무 느낌의 색상 (기존 코드 유지)
            if ((row + col) % 2 == 0) painter.setBrush(QColor(240, 217, 181)); // 밝은 칸
            else painter.setBrush(QColor(181, 136, 99)); // 어두운 칸

            painter.setPen(Qt::NoPen);
            painter.drawRect(col * w, row * h, w, h);
        }
    }
}

void ChessBoardWidget::drawPieces(QPainter &painter)
{
    int w = width() / 8;
    int h = height() / 8;

    // [수정] 기본 폰트를 가져와서 크기와 굵기만 조절
    QFont font = painter.font();
    font.setPixelSize(qMin(w, h) * 0.75); // 크기는 0.8보다 약간 줄여서 (0.75)
    font.setBold(true); // [추가] 글자를 굵게 표시
    painter.setFont(font);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Piece* p = m_game.getPiece(col, row);
            if (!p) continue;

            QString text;

            // [수정] 기호를 다시 영문자로 변경
            switch (p->getType()) {
            case Piece::PAWN: text = "P"; break;
            case Piece::ROOK: text = "R"; break;
            case Piece::KNIGHT: text = "N"; break;
            case Piece::BISHOP: text = "B"; break;
            case Piece::QUEEN: text = "Q"; break;
            case Piece::KING: text = "K"; break;
            default: text = "?"; break;
            }

            // [유지] 색상은 세련된 흑/백을 그대로 사용
            if (p->getColor() == Piece::WHITE) {
                painter.setPen(QColor(245, 245, 245)); // 밝은 회색 (흰색 기물)
            } else { // Piece::BLACK
                painter.setPen(QColor(30, 30, 30)); // 어두운 회색 (검은색 기물)
            }

            painter.drawText(col * w, row * h, w, h, Qt::AlignCenter, text);
        }
    }
}

void ChessBoardWidget::drawHighlights(QPainter &painter)
{
    int w = width() / 8;
    int h = height() / 8;

    if (m_selectedPos.x() != -1) {
        // [수정] 선택된 기물 (선명한 노란색 테두리)
        painter.setPen(QPen(QColor(255, 215, 0, 220), 4)); // 굵기 4, 골드 옐로우
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_selectedPos.x() * w + 2, m_selectedPos.y() * h + 2, w - 4, h - 4); // 테두리가 잘리지 않게 안쪽으로

        // [수정] 이동 가능 위치 (연한 노란색 원)
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 215, 0, 70)); // 연한 골드 옐로우 (투명도 70)
        for (const Move& m : m_validMoves) {
            // 사각형 대신 중앙에 원을 그려 더 부드럽게 표시
            QPoint center(m.toX * w + w / 2, m.toY * h + h / 2);
            painter.drawEllipse(center, w / 4, h / 4);
        }
    }
}



void ChessBoardWidget::mousePressEvent(QMouseEvent *event)
{
    // [수정] 이 함수는 시각적 요소가 아닌 로직 부분이므로 원본 코드를 그대로 유지합니다.
    // (기존 코드와 동일)

    int w = width() / 8;
    int h = height() / 8;
    if (w == 0 || h == 0) return;

    int col = event->x() / w;
    int row = event->y() / h;

    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    // 1. 기물 이동 시도 (이미 선택된 상태에서 클릭)
    if (m_selectedPos.x() != -1) {
        for (const Move& m : m_validMoves) {
            if (m.toX == col && m.toY == row) {

                // [사람] 이동 실행
                Piece* captured = m_game.makeMove(m);
                if (captured != nullptr) {
                    delete captured; // 잡힌 기물 메모리 해제
                }

                // 선택 초기화 및 화면 갱신
                m_selectedPos = QPoint(-1, -1);
                m_validMoves.clear();
                repaint(); // AI 계산 전에 사람이 둔 것을 먼저 화면에 표시
                QCoreApplication::processEvents(); // UI 반응성 확보

                // [AI] 턴 진행 (AI 모드이고 게임이 안 끝났다면)
                if (m_currentMode == aiMode && !m_game.isGameOver()) {

                    // 작성해주신 AI 코드를 그대로 사용합니다.
                    Move aiMove = AI::findBestMove(m_game);

                    if (!aiMove.isNull()) {
                        m_game.makeMove(aiMove);
                        qDebug() << "AI Moved:" << aiMove.fromX << aiMove.fromY << "->" << aiMove.toX << aiMove.toY;
                    } else {
                        qDebug() << "AI cannot move (Stalemate or Checkmate?)";
                    }
                    update(); // AI 이동 후 화면 갱신
                }
                return;
            }
        }
    }

    // 2. 기물 선택 (이동이 아닌 경우)
    Piece* p = m_game.getPiece(col, row);

    // 내 턴인 기물만 선택 가능 (AI 모드일 때 흑색 기물은 선택 불가)
    if (p && p->getColor() == m_game.getCurrentTurn()) {
        // AI 모드에서 사용자가 AI 턴(흑색)의 기물을 클릭하려 하면 무시
        if (m_currentMode == aiMode && p->getColor() == Piece::BLACK) {
            return;
        }

        m_selectedPos = QPoint(col, row);
        std::vector<Move> allMoves = m_game.generateMoves(m_game.getCurrentTurn());
        m_validMoves.clear();

        // 선택한 기물의 이동 경로만 필터링
        for (const Move& m : allMoves) {
            if (m.fromX == col && m.fromY == row) {
                m_validMoves.push_back(m);
            }
        }
        update();
    } else {
        // 빈 곳 클릭 시 선택 해제
        m_selectedPos = QPoint(-1, -1);
        m_validMoves.clear();
        update();
    }
}
