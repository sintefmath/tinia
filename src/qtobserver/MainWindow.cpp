/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

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

