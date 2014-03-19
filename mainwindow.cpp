#include <QMainWindow>
#include <QAction>
#include <QMenuBar>
#include <QLabel>
#include <QFrame>
#include <dirent.h>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle("Demo Putting Glasses");
    setWindowFlags(Qt::WindowCloseButtonHint);
    initErrorMessages();
    debug = QCoreApplication::arguments().contains("debug", Qt::CaseInsensitive);
    detector = new Detector(debug);
    imageNameLabel = new QLabel();
    spectacleLabel = new QLabel();
    pathToModelsLabel = new QLabel();

    findImageButton = new QPushButton(tr("Choose image"));
    connect(findImageButton, SIGNAL(clicked()), this, SLOT(findImage()));

    findSpectacleButton = new QPushButton(tr("Choose spectacles"));
    connect(findSpectacleButton, SIGNAL(clicked()), this, SLOT(findSpectacles()));

    findModelsButton = new QPushButton(tr("Set path to models"));
    connect(findModelsButton, SIGNAL(clicked()), this, SLOT(findModels()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(pathToModelsLabel,0,0);
    mainLayout->addWidget(findModelsButton, 0,1);
    mainLayout->addWidget(imageNameLabel,1,0);
    mainLayout->addWidget(findImageButton, 1,1);
    mainLayout->addWidget(spectacleLabel, 2, 0);
    mainLayout->addWidget(findSpectacleButton, 2, 1);
    setLayout(mainLayout);

    resize(400, 200);
}

void MainWindow::initErrorMessages() {
    errorMessages[Detector::StatusError] = tr("Error!");
    errorMessages[Detector::StatusNoImage] = tr("No image was chosen!");
    errorMessages[Detector::StatusNoSpectacles] = tr("No spectacles chosen!");
    errorMessages[Detector::StatusNoModel] = tr("Recognition model(s) not loaded!");
    errorMessages[Detector::StatusNotLoaded] = tr("Could not load the given file!");
}

MainWindow::~MainWindow()
{

}


void MainWindow::doEvent()
{
    Detector::Status status = detector->detectAndDraw();
    if (status!=Detector::StatusOK) showErrorMessage(status);
}

void MainWindow::findImage()
{
    QString fileName = getImageForLabel(tr("Choose Image"));
    if (fileName.isEmpty()) return;
    imageNameLabel->setPixmap(QPixmap(fileName).scaledToWidth(200));
    Detector::Status status = detector->loadImage(fileName.toStdString());
    if (status!=Detector::StatusOK) showErrorMessage(status);
    if (detector->isReady()) doEvent();
}

void MainWindow::findSpectacles()
{
    QString fileName = getImageForLabel(tr("Choose Spectacles"));
    if (fileName.isEmpty()) return;
    spectacleLabel->setPixmap(QPixmap(fileName).scaledToWidth(200));
    Detector::Status status = detector->loadSpectacles(fileName.toStdString());
    if (status!=Detector::StatusOK) showErrorMessage(status);
    if (detector->isReady()) doEvent();
}

QString MainWindow::getImageForLabel(QString prompt)
{
    return QFileDialog::getOpenFileName(this,
        prompt, getcwd(NULL,0), tr("Image Files (*.png *.jpg *.bmp)"));
}

void MainWindow::findModels() {
    QString fileName = QFileDialog::getExistingDirectory(this,
        tr("Locate Models"), getcwd(NULL,0));
    if (fileName.isEmpty()) return;
    //just to make it easier for the detector...
    fileName+=QDir::separator();
    pathToModelsLabel->setText(fileName);
    Detector::Status status = detector->loadModels(fileName.toStdString());
    if (status!=Detector::StatusOK) showErrorMessage(status);
    if (detector->isReady()) doEvent();
}

void MainWindow::showErrorMessage(Detector::Status status) {
    QMessageBox::information(NULL, QObject::tr("ERROR!"), errorMessages[status], "OK");
}
