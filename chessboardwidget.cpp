#include "chessboardwidget.h"
#include <QDebug>
#include <QCoreApplication>
#include <QPainter>
#include <QMouseEvent>

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
    painter.setRenderHint(QPainter::Antialiasing);

    drawBoard(painter);
    drawHighlights(painter);
    drawPieces(painter);
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
            case Piece::PAWN: text = "P"; break;
            case Piece::ROOK: text = "R"; break;
            case Piece::KNIGHT: text = "N"; break;
            case Piece::BISHOP: text = "B"; break;
            case Piece::QUEEN: text = "Q"; break;
            case Piece::KING: text = "K"; break;
            default: text = "?"; break;
            }

            if (p->getColor() == Piece::WHITE) {
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

    if (m_selectedPos.x() != -1) {
        for (const Move& m : m_validMoves) {
            if (m.toX == col && m.toY == row) {
                UndoInfo undo = m_game.makeMove(m);
                if (undo.capturedPiece != nullptr) {
                    delete undo.capturedPiece;
                }

                m_selectedPos = QPoint(-1, -1);
                m_validMoves.clear();
                repaint();
                QCoreApplication::processEvents();

                if (m_currentMode == aiMode && !m_game.isGameOver()) {
                    Move aiMove = AI::findBestMove(m_game);
                    if (!aiMove.isNull()) {
                        UndoInfo ai_undo = m_game.makeMove(aiMove);
                        if (ai_undo.capturedPiece != nullptr) {
                            delete ai_undo.capturedPiece;
                        }
                        qDebug() << "AI Moved:" << aiMove.fromX << aiMove.fromY << "->" << aiMove.toX << aiMove.toY;
                    } else {
                        qDebug() << "AI cannot move (Stalemate or Checkmate?)";
                    }
                    update();
                }
                return;
            }
        }
    }

    Piece* p = m_game.getPiece(col, row);

    if (p && p->getColor() == m_game.getCurrentTurn()) {
        if (m_currentMode == aiMode && p->getColor() == Piece::BLACK) {
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