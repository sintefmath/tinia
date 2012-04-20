#ifndef FILEDIALOGBUTTON_HPP
#define FILEDIALOGBUTTON_HPP

#include <QPushButton>
#include <tinia/policylib/PolicyLib.hpp>
#include <memory>
#include "tinia/qtobserver/moc/StringController.hpp"

namespace tinia {
namespace qtobserver {
class FileDialogButton : public QPushButton
{
    Q_OBJECT
public:
    explicit FileDialogButton(std::string key, bool showValue,
                              std::shared_ptr<policylib::PolicyLib> policyLib,
                              QWidget *parent = 0);

signals:

public slots:
   void readFile();
private:
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   StringController m_controller;
};

}
}
#endif // FILEDIALOGBUTTON_HPP
