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

    m_boardWidget->startGame();
}

