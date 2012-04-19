#include "qtobserver/moc/FileDialogButton.hpp"
#include <QFileDialog>
#include <policylib/File.hpp>
namespace qtobserver {
FileDialogButton::FileDialogButton(std::string key,
                                   bool showValue,
                                   std::shared_ptr<policylib::PolicyLib> policyLib,
                                   QWidget *parent) :
    QPushButton(parent), m_key(key), m_policyLib(policyLib),
   m_controller(this, m_policyLib, key, showValue)
{
   connect(this, SIGNAL(clicked()), this, SLOT(readFile()));
}

}

void qtobserver::FileDialogButton::readFile()
{
   QString fileName = QFileDialog::getOpenFileName(this,
                                                   this->text(),
                                                   ".");
   policylib::File file;
   file.fullPath(fileName.toStdString());
   m_policyLib->updateElement(m_key, file);

}
