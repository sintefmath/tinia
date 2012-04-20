#include "tinia/qtobserver/moc/FileDialogButton.hpp"
#include <QFileDialog>
#include <tinia/policy/File.hpp>

namespace tinia {
namespace qtobserver {
FileDialogButton::FileDialogButton(std::string key,
                                   bool showValue,
                                   std::shared_ptr<policy::Policy> policy,
                                   QWidget *parent) :
    QPushButton(parent), m_key(key), m_policy(policy),
   m_controller(this, m_policy, key, showValue)
{
   connect(this, SIGNAL(clicked()), this, SLOT(readFile()));
}

}

void qtobserver::FileDialogButton::readFile()
{
   QString fileName = QFileDialog::getOpenFileName(this,
                                                   this->text(),
                                                   ".");
   policy::File file;
   file.fullPath(fileName.toStdString());
   m_policy->updateElement(m_key, file);

}
} // of namespace tinia
