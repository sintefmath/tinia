#ifndef QTOBSERVER_MAINWINDOW_HPP
#define QTOBSERVER_MAINWINDOW_HPP

#include <QMainWindow>
namespace tinia {
namespace qtobserver {


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
   void clearDrawWidget(QWidget* newWidget);
   void clearDrawWidget();
private:
   QWidget *m_drawWidget;
};


} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_MAINWINDOW_HPP
