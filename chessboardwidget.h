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

    // ���� ���� ��ü
    Game m_game;

    // UI ���ͷ��� ����
    QPoint m_selectedPos;          // ���� ���õ� ��ǥ
    std::vector<Move> m_validMoves; // ���õ� �⹰�� �� �� �ִ� ����

    gameMode m_currentMode; // [�߰�] ���� ���� ��� ���� ����
};

#endif
