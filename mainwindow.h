#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtGui>
#include <QMainWindow>
#include <QMessageBox>
#include "detector.h"

namespace Ui {
    class MainWindow;
}
class MyThread;


class MainWindow : public QDialog
{
    Q_OBJECT
    QAction* action;
    Detector* detector;
    QLabel *imageNameLabel;
    QLabel *spectacleLabel;
    QLabel *pathToModelsLabel;
    QPushButton *findModelsButton;
    QPushButton *findImageButton;
    QPushButton *findSpectacleButton;
    bool debug;

    QHash<Detector::Status, QString> errorMessages;

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void doEvent();
    void findImage();
    void findSpectacles();
    void findModels();

private:
    void showErrorMessage(Detector::Status status);
    void initErrorMessages();
    QString getImageForLabel(QString prompt);
};


#endif // MAINWINDOW_H
