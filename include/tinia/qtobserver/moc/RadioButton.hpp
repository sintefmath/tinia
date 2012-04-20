#ifndef QTOBSERVER_RADIOBUTTON_HPP
#define QTOBSERVER_RADIOBUTTON_HPP

#include <QRadioButton>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include <memory>

namespace tinia {
namespace qtobserver {

class RadioButton : public QRadioButton, public policy::StateListener
{
    Q_OBJECT
public:
    explicit RadioButton(std::string value, std::string key,
                         std::shared_ptr<policy::Policy> policy,
                         QWidget *parent = 0);

   ~RadioButton();

   void stateElementModified(policy::StateElement *stateElement);
signals:
   void setCheckedFromPolicy(bool);
public slots:
   void setCheckedFromQt(bool);
private:
   std::string m_value;
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;

};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_RADIOBUTTON_HPP
