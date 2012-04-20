#ifndef QTOBSERVER_RADIOBUTTON_HPP
#define QTOBSERVER_RADIOBUTTON_HPP

#include <QRadioButton>
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/StateListener.hpp"
#include <memory>

namespace qtobserver {

class RadioButton : public QRadioButton, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit RadioButton(std::string value, std::string key,
                         std::shared_ptr<policylib::PolicyLib> policyLib,
                         QWidget *parent = 0);

   ~RadioButton();

   void stateElementModified(policylib::StateElement *stateElement);
signals:
   void setCheckedFromPolicyLib(bool);
public slots:
   void setCheckedFromQt(bool);
private:
   std::string m_value;
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;

};

} // namespace qtobserver

#endif // QTOBSERVER_RADIOBUTTON_HPP
