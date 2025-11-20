#ifndef CHESSBOARDWIDGET_H
#define CHESSBOARDWIDGET_H

#include <QWidget>
#include <QPoint>
#include "game.h"
#include "move.h"
#include <vector>
#include <QPainter>
#include <QMouseEvent>
#include <ai.h>


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
    QSize sizeHint() const override;

    void setGameMode(gameMode mode);

public slots:
    void startGame();

private:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

    void drawBoard(QPainter &painter);
    void drawPieces(QPainter &painter);
    void drawHighlights(QPainter &painter);

    // 게임 로직 객체
    Game m_game;

    // UI 인터랙션 변수
    QPoint m_selectedPos;          // 현재 선택된 좌표
    std::vector<Move> m_validMoves; // 선택된 기물이 갈 수 있는 곳들

    gameMode m_currentMode; // [추가] 현재 게임 모드 저장 변수
};

#endif
