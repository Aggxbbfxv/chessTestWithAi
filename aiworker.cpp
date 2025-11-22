#include "aiworker.h"

AiWorker::AiWorker(QObject *parent) : QObject(parent)
{
}

void AiWorker::findBestMove(const Game &game, Chess::PieceColor aiColor)
{
    Move bestMove = AI::findBestMove(game, aiColor);
    emit moveFound(bestMove);
}
