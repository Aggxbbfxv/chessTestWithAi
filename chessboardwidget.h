#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include <QWidget>
#include <QPoint>
#include <QThread>
#include "game.h"
#include "move.h"
#include "aiworker.h"
#include <vector>
#include <QPainter>
#include <QMouseEvent>


class ChessBoardWidget : public QWidget
{
    Q_OBJECT

public:
    enum gameMode
    {
        customMode,
        aiMode
    };

    explicit ChessBoardWidget(QWidget *parent = nullptr);
    ~ChessBoardWidget();
    QSize sizeHint() const override;

    void setGameMode(gameMode mode);
    void setAiPlaysWhite(bool enable);
    void setAiPlaysBlack(bool enable);
    void setAiPlayers(bool white, bool black);

public slots:
    void startGame();

private slots:
    void onAiMoveFound(Move move);

signals:
    void requestAiMove(const Game& game);


private:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void drawBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawHighlights(QPainter &painter);
    void triggerAiIfNeeded();
    bool isAiTurn() const;

    // Game state data
    Game m_game;

    // UI interaction state
    QPoint m_selectedPos;          // Currently selected square
    std::vector<Move> m_validMoves; // Moves available for the selected piece

    // Last move highlight
    QPoint m_lastFrom;
    QPoint m_lastTo;

    gameMode m_currentMode; // Current game mode
    bool m_aiPlaysWhite = false;
    bool m_aiPlaysBlack = false;
};

#endif
