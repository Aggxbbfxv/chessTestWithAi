#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ChessBoardWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void on_startButton_clicked();
    void on_aiWhite_toggled(bool checked);
    void on_aiBlack_toggled(bool checked);

private:
    Ui::MainWindow *ui;
    ChessBoardWidget *m_boardWidget;

    // Which sides are driven by AI
    bool m_aiPlaysWhite = false;
    bool m_aiPlaysBlack = false;
};
#endif // MAINWINDOW_H
