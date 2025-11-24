#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "chessboardwidget.h"
#include <QVBoxLayout>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_boardWidget = new ChessBoardWidget(ui->widget);
    QVBoxLayout *layout = new QVBoxLayout(ui->widget);
    layout->addWidget(m_boardWidget);
    layout->setContentsMargins(0,0,0,0);
    ui->widget->setLayout(layout);
    m_boardWidget->setEnabled(false);

    // Initialize AI color selection from UI defaults
    m_aiPlaysWhite = ui->aiWhite->isChecked();
    m_aiPlaysBlack = ui->aiBlack->isChecked();
    m_boardWidget->setAiPlayers(m_aiPlaysWhite, m_aiPlaysBlack);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_startButton_clicked()
{
    m_boardWidget->setEnabled(true);

    ui->groupBox->setEnabled(false);
    ui->startButton->setEnabled(false);

    if(ui->aiButton->isChecked()){
        m_boardWidget->setGameMode(ChessBoardWidget::aiMode);
    }
    else if(ui->customButton->isChecked())
    {
        m_boardWidget->setGameMode(ChessBoardWidget::customMode);
    }

    // Sync AI sides with current checkbox state for the new game
    m_boardWidget->setAiPlayers(m_aiPlaysWhite, m_aiPlaysBlack);

    m_boardWidget->startGame();
}

void MainWindow::on_aiWhite_toggled(bool checked)
{
    m_aiPlaysWhite = checked;
    m_boardWidget->setAiPlayers(m_aiPlaysWhite, m_aiPlaysBlack);
}

void MainWindow::on_aiBlack_toggled(bool checked)
{
    m_aiPlaysBlack = checked;
    m_boardWidget->setAiPlayers(m_aiPlaysWhite, m_aiPlaysBlack);
}
