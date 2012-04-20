#include "tinia/qtobserver/moc/MainWindow.hpp"
#include <QLayout>
#include <QLabel>
#include <QHBoxLayout>
namespace tinia {
namespace qtobserver {

MainWindow::MainWindow(QWidget *parent) :
   QMainWindow(parent)
{
   setCentralWidget(new QWidget());
   m_drawWidget = new QWidget(this);
   centralWidget()->setLayout(new QHBoxLayout(this));
   centralWidget()->layout()->addWidget(m_drawWidget);
   m_drawWidget->setLayout(new QHBoxLayout(this));
   m_drawWidget->layout()->addWidget(new QLabel("Building GUI"));
}

MainWindow::~MainWindow()
{
}

}

void qtobserver::MainWindow::clearDrawWidget(QWidget* newWidget)
{
   delete m_drawWidget;
   centralWidget()->layout()->addWidget(newWidget);
   newWidget->show();
}
void qtobserver::MainWindow::clearDrawWidget(){
   setCentralWidget(new QWidget());
}

} // of namespace tinia

