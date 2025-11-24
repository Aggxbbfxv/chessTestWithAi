#include "chessboardwidget.h"
#include "piece.h"
#include "ai.h"
#include <QDebug>
#include <QCoreApplication>
#include <QPainter>
#include <QMouseEvent>

ChessBoardWidget::ChessBoardWidget(QWidget *parent)
    : QWidget(parent),
      m_selectedPos(-1, -1),
      m_lastFrom(-1, -1),
      m_lastTo(-1, -1),
      m_currentMode(customMode)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

ChessBoardWidget::~ChessBoardWidget()
{
    // No heap-owned resources to release yet. Added to satisfy the Q_OBJECT vtable.
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

void ChessBoardWidget::setAiPlaysWhite(bool enable)
{
    m_aiPlaysWhite = enable;
}

void ChessBoardWidget::setAiPlaysBlack(bool enable)
{
    m_aiPlaysBlack = enable;
}

void ChessBoardWidget::setAiPlayers(bool white, bool black)
{
    m_aiPlaysWhite = white;
    m_aiPlaysBlack = black;
}

void ChessBoardWidget::startGame()
{
    m_game.resetBoard();
    m_selectedPos = QPoint(-1, -1);
    m_validMoves.clear();
    m_lastFrom = QPoint(-1, -1);
    m_lastTo = QPoint(-1, -1);
    update();
    qDebug() << "Game Started!";
    triggerAiIfNeeded();
}

void ChessBoardWidget::onAiMoveFound(Move move)
{
    if (move.isNull()) {
        qDebug() << "AI cannot move (Stalemate or Checkmate?)";
        return;
    }

    m_game.makeMove(move);
    m_selectedPos = QPoint(-1, -1);
    m_validMoves.clear();
    m_lastFrom = QPoint(move.fromX, move.fromY);
    m_lastTo = QPoint(move.toX, move.toY);

    qDebug() << "AI Moved:" << move.fromX << move.fromY << "->" << move.toX << move.toY;
    update();
}

void ChessBoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawBoard(painter);
    drawPieces(painter);
    drawHighlights(painter);
}

void ChessBoardWidget::drawBoard(QPainter &painter)
{
    int w = width() / 8;
    int h = height() / 8;

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            if ((row + col) % 2 == 0) painter.setBrush(QColor(240, 217, 181));
            else painter.setBrush(QColor(181, 136, 99));

            painter.setPen(Qt::NoPen);
            painter.drawRect(col * w, row * h, w, h);
        }
    }
}

void ChessBoardWidget::drawPieces(QPainter &painter)
{
    int w = width() / 8;
    int h = height() / 8;

    QFont font = painter.font();
    font.setPixelSize(qMin(w, h) * 0.75);
    font.setBold(true);
    painter.setFont(font);

    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            Piece* p = m_game.getPiece(col, row);
            if (!p) continue;

            QString text;
            switch (p->getType()) {
            case Chess::PAWN: text = "P"; break;
            case Chess::ROOK: text = "R"; break;
            case Chess::KNIGHT: text = "N"; break;
            case Chess::BISHOP: text = "B"; break;
            case Chess::QUEEN: text = "Q"; break;
            case Chess::KING: text = "K"; break;
            default: text = "?"; break;
            }

            if (p->getColor() == Chess::WHITE) {
                painter.setPen(QColor(245, 245, 245));
            } else {
                painter.setPen(QColor(30, 30, 30));
            }

            painter.drawText(col * w, row * h, w, h, Qt::AlignCenter, text);
        }
    }
}

void ChessBoardWidget::drawHighlights(QPainter &painter)
{
    int w = width() / 8;
    int h = height() / 8;
    if (w == 0 || h == 0) return;

    // Highlight the last move squares with a translucent red overlay
    const QColor lastMoveColor(255, 0, 0, 100);
    painter.setPen(Qt::NoPen);
    if (m_lastFrom.x() >= 0 && m_lastFrom.y() >= 0) {
        painter.fillRect(m_lastFrom.x() * w, m_lastFrom.y() * h, w, h, lastMoveColor);
    }
    if (m_lastTo.x() >= 0 && m_lastTo.y() >= 0) {
        painter.fillRect(m_lastTo.x() * w, m_lastTo.y() * h, w, h, lastMoveColor);
    }

    if (m_selectedPos.x() != -1) {
        painter.setPen(QPen(QColor(255, 215, 0, 220), 4));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(m_selectedPos.x() * w + 2, m_selectedPos.y() * h + 2, w - 4, h - 4);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 215, 0, 70));
        for (const Move& m : m_validMoves) {
            QPoint center(m.toX * w + w / 2, m.toY * h + h / 2);
            painter.drawEllipse(center, w / 4, h / 4);
        }
    }
}

void ChessBoardWidget::mousePressEvent(QMouseEvent *event)
{
    int w = width() / 8;
    int h = height() / 8;
    if (w == 0 || h == 0) return;

    int col = event->x() / w;
    int row = event->y() / h;

    if (col < 0 || col >= 8 || row < 0 || row >= 8) return;

    if (isAiTurn()) {
        // AI controls the current turn; ignore human input and let AI act if pending
        triggerAiIfNeeded();
        return;
    }

    if (m_selectedPos.x() != -1) {
        for (const Move& m : m_validMoves) {
            if (m.toX == col && m.toY == row) {
                UndoInfo undo = m_game.makeMove(m);

                m_lastFrom = QPoint(m.fromX, m.fromY);
                m_lastTo = QPoint(m.toX, m.toY);
                m_selectedPos = QPoint(-1, -1);
                m_validMoves.clear();
                repaint();
                QCoreApplication::processEvents();

                triggerAiIfNeeded();
                return;
            }
        }
    }

    Piece* p = m_game.getPiece(col, row);

    if (p && p->getColor() == m_game.getCurrentTurn()) {
        if ((p->getColor() == Chess::WHITE && m_aiPlaysWhite) ||
            (p->getColor() == Chess::BLACK && m_aiPlaysBlack)) {
            return;
        }

        m_selectedPos = QPoint(col, row);
        m_validMoves = m_game.generateMoves(col, row);

        update();
    } else {
        m_selectedPos = QPoint(-1, -1);
        m_validMoves.clear();
        update();
    }
}

bool ChessBoardWidget::isAiTurn() const
{
    Chess::PieceColor turn = m_game.getCurrentTurn();
    return (turn == Chess::WHITE && m_aiPlaysWhite) || (turn == Chess::BLACK && m_aiPlaysBlack);
}

void ChessBoardWidget::triggerAiIfNeeded()
{
    if (m_game.isGameOver()) return;

    while (isAiTurn()) {
        Move aiMove = AI::findBestMove(m_game, m_game.getCurrentTurn());
        if (aiMove.isNull()) {
            qDebug() << "AI cannot move (no legal moves)";
            break;
        }

        onAiMoveFound(aiMove);

        if (m_game.isGameOver()) break;

        // Let the event loop process UI updates between consecutive AI moves (AI vs AI)
        QCoreApplication::processEvents();
    }
}
