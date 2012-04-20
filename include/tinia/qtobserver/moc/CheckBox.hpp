#ifndef QTOBSERVER_CHECKBOX_HPP
#define QTOBSERVER_CHECKBOX_HPP

#include <QCheckBox>
#include <tinia/policylib/PolicyLib.hpp>
#include <tinia/policylib/StateListener.hpp>
#include <memory>

namespace qtobserver {

class CheckBox : public QCheckBox, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit CheckBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                      QWidget *parent = 0);
   ~CheckBox();

   void stateElementModified(policylib::StateElement *stateElement);

signals:
   void setCheckFromPolicy(bool checked);

public slots:
   void setCheckedFromQt(bool checked);


private:
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;

};

} // namespace qtobserver

#endif // QTOBSERVER_CHECKBOX_HPP
