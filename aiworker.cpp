#include "aiworker.h"

AiWorker::AiWorker(QObject *parent) : QObject(parent)
{
}

void AiWorker::findBestMove(const Game &game, Piece::PieceColor aiColor)
{
    Move bestMove = AI::findBestMove(game, aiColor);
    emit moveFound(bestMove);
}
