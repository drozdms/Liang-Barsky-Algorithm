#include "mainwindow.h"
#include <QHBoxLayout>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)

{
    setMouseTracking(true);
    setLayout(new QHBoxLayout(this));
    setWindowIcon(QIcon("clipping.png"));

    setupUi(this);
    connect(this->pushButton, SIGNAL(clicked()), paintWidget, SLOT(generateRandomSegments()));
//    connect(this->parentWidget(), SIGNAL(keyReleaseEvent(QKeyEvent*e)), paintWidget, SLOT(reset(QKeyEvent*e)));
}

MainWindow::~MainWindow()
{
   // delete ui;
}




