#ifndef AIWORKER_H
#define AIWORKER_H

#include <QObject>
#include "game.h"
#include "move.h"
#include "ai.h"

class AiWorker : public QObject
{
    Q_OBJECT
public:
    explicit AiWorker(QObject *parent = nullptr);

public slots:
    void findBestMove(const Game& game, Piece::PieceColor aiColor);

signals:
    void moveFound(Move move);
};

#endif // AIWORKER_H
