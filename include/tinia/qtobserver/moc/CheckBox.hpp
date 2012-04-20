#ifndef QTOBSERVER_CHECKBOX_HPP
#define QTOBSERVER_CHECKBOX_HPP

#include <QCheckBox>
#include <tinia/policy/Policy.hpp>
#include <tinia/policy/StateListener.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {

class CheckBox : public QCheckBox, public policy::StateListener
{
    Q_OBJECT
public:
    explicit CheckBox(std::string key, std::shared_ptr<policy::Policy> policy,
                      QWidget *parent = 0);
   ~CheckBox();

   void stateElementModified(policy::StateElement *stateElement);

signals:
   void setCheckFromPolicy(bool checked);

public slots:
   void setCheckedFromQt(bool checked);


private:
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;

};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_CHECKBOX_HPP
